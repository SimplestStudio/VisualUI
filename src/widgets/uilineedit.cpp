#include "uilineedit.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uiapplication.h"
#include "uiutils.h"
#include <vector>
#ifdef _WIN32
# include <windowsx.h>
#else
# include "cmath"
# include "widgets/linux/gtkcaret.h"
#endif

#define DEFAULT_CARET_WIDTH 1

using namespace UIUnicode;

UILineEdit::UILineEdit(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_sourceText(text),
#ifdef _WIN32
    m_caretCreated(false),
#else
    m_caret(nullptr),
#endif
    m_editable(true),
    m_pos(0),
    m_caretPosX(0),
    m_caretPosY(0),
    m_textOffsetX(0),
    m_caretPositions()
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
    metrics()->setMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UILineEdit::~UILineEdit()
{

}

void UILineEdit::setText(const tstring &text) noexcept
{
    UIAbstractButton::setText(text);
    m_sourceText = text;
    m_pos = 0;
    m_textOffsetX = 0;

    updateViewportAndCaret();
    updateCaretPosition();
}

void UILineEdit::setPlaceholderText(const tstring &text) noexcept
{
    m_placeholderText = text;
    if (m_text.empty()) {
        updateViewportAndCaret();
        updateCaretPosition();
    }
}

void UILineEdit::setEditable(bool editable)
{
    m_editable = editable;
}

#ifdef _WIN32
bool UILineEdit::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST:
        return false;

    case WM_SETCURSOR:
        if (!m_disabled && m_editable && LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(nullptr, IDC_IBEAM));
            *result = TRUE;
            return true;
        }
        break;

    case WM_DPICHANGED_NOTIFY: {
        UIAbstractButton::event(msg, wParam, lParam, result);
        
        // Reset scroll and position when DPI changes
        m_textOffsetX = 0;
        m_pos = 0;
        
        int w = 0;
        int h = 0;
        size(&w, &h);

        bool rtl = m_rtl;
        const Metrics *mtr = metrics();
        int mrg_top = mtr->value(Metrics::TextMarginTop) * m_dpi_ratio;
        int mrg_bottom = mtr->value(Metrics::TextMarginBottom) * m_dpi_ratio;
        int mrg_left = (rtl ? mtr->value(Metrics::TextMarginRight) : mtr->value(Metrics::TextMarginLeft)) * m_dpi_ratio;
        int mrg_right = (rtl ? mtr->value(Metrics::TextMarginLeft) : mtr->value(Metrics::TextMarginRight)) * m_dpi_ratio;
        m_viewportRc = Rect(mrg_left, mrg_top, w - mrg_right - mrg_left, h - mrg_bottom - mrg_top);
        updateViewportAndCaret();
        updateCaretPosition();
        return true;
    }

    case WM_SIZE: {
        SetFocus(m_root_hWnd);

        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        bool rtl = m_rtl;
        const Metrics *mtr = metrics();
        int mrg_top = mtr->value(Metrics::TextMarginTop) * m_dpi_ratio;
        int mrg_bottom = mtr->value(Metrics::TextMarginBottom) * m_dpi_ratio;
        int mrg_left = (rtl ? mtr->value(Metrics::TextMarginRight) : mtr->value(Metrics::TextMarginLeft)) * m_dpi_ratio;
        int mrg_right = (rtl ? mtr->value(Metrics::TextMarginLeft) : mtr->value(Metrics::TextMarginRight)) * m_dpi_ratio;
        m_viewportRc = Rect(mrg_left, mrg_top, w - mrg_right - mrg_left, h - mrg_bottom - mrg_top);
        updateViewportAndCaret();
        updateCaretPosition();
        break;
    }

    case WM_SETFOCUS: {
        updateViewportAndCaret();
        if (!m_caretCreated && m_editable) {
            Rect defCaretRc;
            defaultCaretRect(defCaretRc);
            CreateCaret(m_hWindow, NULL, defCaretRc.width, defCaretRc.height);
            m_caretCreated = true;
            SetCaretPos(m_caretPosX, m_caretPosY);
            ShowCaret(m_hWindow);
        }
        break;
    }

    case WM_KILLFOCUS: {
        if (m_sourceText != m_text)
            m_sourceText = m_text;
        if (!m_text.empty())
            m_pos = 0;
        updateViewportAndCaret();
        if (m_caretCreated) {
            m_caretCreated = false;
            DestroyCaret();
        }
        break;
    }

    case WM_KEYDOWN:
        if (!m_editable)
            break;
        switch (wParam) {
        case VK_HOME:
            if (m_pos > 0 && m_text.length() > 0) {
                m_pos = 0;
                updateViewportAndCaret();
            }
            break;

        case VK_END:
            if (m_pos < m_text.length() && m_text.length() > 0) {
                m_pos = m_text.length();
                updateViewportAndCaret();
            }
            break;

        case VK_LEFT:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                updateViewportAndCaret();
            }
            break;

        case VK_RIGHT:
            if (m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_pos += len;
                updateViewportAndCaret();
            }
            break;

        case VK_DELETE:
            if (m_text.length() > 0 && m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_text.erase(m_pos, len);
                updateViewportAndCaret();
            }
            break;
        }

        updateCaretPosition();
        break;

    case WM_CHAR:
        if (!m_editable)
            break;
        switch (wParam) {
        case 0x08: // Backspace
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                SendMessage(m_hWindow, WM_KEYDOWN, VK_DELETE, 1L);
            }
            break;

        case 0x09: // Tab
        case 0x0A: // Linefeed
            break;

        case 0x1B: // Escape
            m_text = m_sourceText;
        case 0x0D: // Return        
            SetFocus(m_root_hWnd);
            break;

        default:
            wchar_t ch = (wchar_t)wParam;
            m_text.insert(m_pos, 1, ch);
            m_pos++;
            updateViewportAndCaret();
            break;
        }

        updateCaretPosition();
        break;

    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            updateCaretOnMouseClick(x, y);
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UILineEdit::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_ENTER_NOTIFY: {
        if (!m_disabled && m_editable) {
            GdkDisplay *display = gtk_widget_get_display(m_hWindow);
            GdkCursor *cursor = gdk_cursor_new_from_name(display, "text");
            GdkWindow *gdk_wnd = gtk_widget_get_window(m_hWindow);
            if (gdk_wnd && cursor) {
                gdk_window_set_cursor(gdk_wnd, cursor);
                g_object_unref(cursor);
            }
        }
        break;
    }

    case GDK_HOOKED_SIZE_ALLOC: {
        GtkAllocation *alc = (GtkAllocation*)param;
        const Metrics *mtr = metrics();
        int mrg_top = mtr->value(Metrics::TextMarginTop) * m_dpi_ratio;
        int mrg_left = (m_rtl ? mtr->value(Metrics::TextMarginRight) : mtr->value(Metrics::TextMarginLeft)) * m_dpi_ratio;
        int mrg_right = (m_rtl ? mtr->value(Metrics::TextMarginLeft) : mtr->value(Metrics::TextMarginRight)) * m_dpi_ratio;
        int mrg_bottom = mtr->value(Metrics::TextMarginBottom) * m_dpi_ratio;
        m_viewportRc = Rect(mrg_left, mrg_top, alc->width - mrg_right - mrg_left, alc->height - mrg_bottom - mrg_top);
        updateViewportAndCaret();
        updateCaretPosition();
        break;
    }

    case GDK_FOCUS_CHANGE: {
        GdkEventFocus *fev = (GdkEventFocus*)param;
        if (fev->in == 1) {
            updateViewportAndCaret();
            if (!m_caret && m_editable) {
                Rect defCaretRc;
                defaultCaretRect(defCaretRc);
                m_caret = gtk_caret_create(m_hWindow, defCaretRc.width, defCaretRc.height);
                gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
                gtk_caret_show(GTK_CARET(m_caret));
            }
        } else {
            if (m_sourceText != m_text)
                m_sourceText = m_text;
            if (!m_text.empty())
                m_pos = 0;
            updateViewportAndCaret();
            if (m_caret) {
                gtk_caret_destroy(GTK_CARET(m_caret));
                m_caret = nullptr;
            }
        }
        break;
    }

    case GDK_KEY_PRESS: {
        if (!m_editable)
            break;
        GdkEventKey *kev = (GdkEventKey*)param;
        switch (kev->keyval) {
        case GDK_KEY_Home:
            if (m_pos > 0 && m_text.length() > 0) {
                m_pos = 0;
                updateViewportAndCaret();
            }
            break;

        case GDK_KEY_End:
            if (m_pos < m_text.length() && m_text.length() > 0) {
                m_pos = m_text.length();
                updateViewportAndCaret();
            }
            break;

        case GDK_KEY_Left:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -= len;
                updateViewportAndCaret();
            }
            break;

        case GDK_KEY_Right:
            if (m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_pos += len;
                updateViewportAndCaret();
            }
            break;

        case GDK_KEY_Delete:
            if (m_text.length() > 0 && m_pos < m_text.length()) {
                size_t len = charLenAt(m_text, m_pos);
                m_text.erase(m_pos, len);
                updateViewportAndCaret();
            }
            break;

        case GDK_KEY_BackSpace:
            if (m_pos > 0) {
                size_t len = charLenBefore(m_text, m_pos);
                m_pos -=len;
                if (m_pos < m_text.length()) {
                    size_t len = charLenAt(m_text, m_pos);
                    m_text.erase(m_pos, len);
                    updateViewportAndCaret();
                }
            }
            break;

        case GDK_KEY_Tab:
        case GDK_KEY_Linefeed:
            break;

        case GDK_KEY_Escape:
            m_text = m_sourceText;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            if (GtkWidget *parent = gtk_widget_get_parent(m_hWindow))
                gtk_widget_grab_focus(parent);
            break;

        default:
            gunichar uni = gdk_keyval_to_unicode(kev->keyval);
            if (uni != 0) {
                char utf8[5] = {0};
                int len = g_unichar_to_utf8(uni, utf8);
                m_text.insert(m_pos, utf8, len);
                m_pos += len;
                updateViewportAndCaret();
            }
            break;
        }

        updateCaretPosition();
        return true;
    }

    case GDK_BUTTON_PRESS: {
        if (!m_disabled) {
            GdkEventButton *bev = (GdkEventButton*)param;
            updateCaretOnMouseClick(bev->x, bev->y);
        }
        break;
    }

    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UILineEdit::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
    if (m_hEmf)
        de->DrawEmfIcon(m_hEmf, iconAngle());
#else
    if (m_hBmp)
        de->DrawIcon(m_hBmp);
    if (m_hSvg)
        de->DrawSvgIcon(m_hSvg, iconAngle());
#endif
    if (!displayedText().empty()) {
        de->DrawStringWithLayout(rc, displayedText(), m_hFont, false, m_textOffsetX);
    }
}

const tstring& UILineEdit::displayedText() const
{
    return (m_text.empty() && !m_placeholderText.empty()) ? m_placeholderText : m_text;
}

void UILineEdit::updateCaretPosition()
{
#ifdef _WIN32
    if (m_caretCreated)
        SetCaretPos(m_caretPosX, m_caretPosY);
#else
    if (m_caret)
        gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
#endif
}

void UILineEdit::calculateCaretPositions()
{
    m_caretPositions.clear();
    const tstring &textToShow = displayedText();
    if (textToShow.empty()) {
        Rect defCaretRc;
        defaultCaretRect(defCaretRc);
        m_caretPositions.push_back({defCaretRc.x, defCaretRc.y});
        return;
    }

#ifdef _WIN32
    HDC hdc = GetDC(m_hWindow);
    HGDIOBJ oldFont = SelectObject(hdc, m_hFont);

    TEXTMETRICW tm = {0};
    GetTextMetricsW(hdc, &tm);
    int x = m_viewportRc.x;
    int y = m_viewportRc.y;
    int width = 0;
    int height = tm.tmHeight;
    int len = textToShow.length();

    std::vector<int> dx(len);
    GCP_RESULTS gcp = {0};
    gcp.lStructSize = sizeof(gcp);
    gcp.lpDx = dx.data();
    gcp.nGlyphs = (UINT)dx.size();
    GetCharacterPlacementW(hdc, textToShow.c_str(), len, 0, &gcp, GCP_LIGATE | GCP_DIACRITIC);

    SelectObject(hdc, oldFont);
    ReleaseDC(m_hWindow, hdc);    
    
    int algn = metrics()->value(Metrics::TextAlignment);
    // if (algn & Metrics::AlignHCenter) {
    //     if (!multiline)
    //         x += round((m_viewportRc.width - width) / 2.0);
    // } else
    // if (algn & Metrics::AlignHRight) {
    //     if (!multiline)
    //         x += m_viewportRc.width - width;
    // }

    if (algn & Metrics::AlignVCenter) {
        y += round((m_viewportRc.height - height) / 2.0);
    } else
    if (algn & Metrics::AlignVBottom) {
        y += m_viewportRc.height - height;
    }

    m_caretPositions.push_back({x, y});
    
    int posX = x;
    for (size_t i = 0; i < len; ++i) {
        posX += dx[i];
        m_caretPositions.push_back({posX, y});
    }
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(m_hWindow, textToShow.c_str());
    pango_layout_set_font_description(lut, m_hFont->desc);

    int x = m_viewportRc.x;
    int y = m_viewportRc.y;
    int width, height;
    pango_layout_get_size(lut, &width, &height);
    width /= PANGO_SCALE;
    height /= PANGO_SCALE;

    int algn = metrics()->value(Metrics::TextAlignment);
    // if (algn & Metrics::AlignHCenter) {
    //     if (!multiline)
    //         x += round((m_viewportRc.width - width) / 2.0);
    // } else
    // if (algn & Metrics::AlignHRight) {
    //     if (!multiline)
    //         x += m_viewportRc.width - width;
    // }

    if (algn & Metrics::AlignVCenter)
        y += round((m_viewportRc.height - height) / 2.0);
    else
    if (algn & Metrics::AlignVBottom)
        y += m_viewportRc.height - height;

    m_caretPositions.push_back({x, y});

    size_t bytePos = 0;
    while (bytePos < textToShow.length()) {
        PangoRectangle rect;
        pango_layout_index_to_pos(lut, bytePos, &rect);
        m_caretPositions.push_back({x + (rect.x + rect.width) / PANGO_SCALE, y + rect.y / PANGO_SCALE});
        bytePos = charNextPos(textToShow, bytePos);
    }

    g_object_unref(lut);
#endif
}

void UILineEdit::updateViewportAndCaret()
{
    if (m_pos > m_text.length())
        m_pos = m_text.length();

    // Calculate caret positions BEFORE adjusting scroll
    calculateCaretPositions();

    // Adjust scroll offset based on fresh positions
    if (!m_caretPositions.empty()) {
#ifdef _WIN32
        size_t pos = (m_pos < m_caretPositions.size()) ? m_pos : m_caretPositions.size() - 1;
#else
        size_t charIndex = 0;
        size_t bytePos = 0;
        while (bytePos < m_pos && bytePos < m_text.length()) {
            bytePos = charNextPos(m_text, bytePos);
            charIndex++;
        }
        size_t pos = (charIndex < m_caretPositions.size()) ? charIndex : m_caretPositions.size() - 1;
#endif
        int caretXClean = m_caretPositions[pos].x;
        
        // Calculate where caret would be with current scroll
        int caretX = caretXClean + m_textOffsetX;
        
        // Adjust scroll if caret outside viewport
        int viewportRight = m_viewportRc.x + m_viewportRc.width;
        int viewportLeft = m_viewportRc.x;
        
        if (caretX > viewportRight) {
            m_textOffsetX -= (caretX - viewportRight);
        } else
        if (caretX < viewportLeft) {
            m_textOffsetX += (viewportLeft - caretX);
        }
        
        // Don't scroll past start
        if (m_textOffsetX > 0)
            m_textOffsetX = 0;
        
        // Update caret position
        m_caretPosX = caretXClean + m_textOffsetX;
        m_caretPosY = m_caretPositions[pos].y;
    }

    update();
}

void UILineEdit::updateCaretOnMouseClick(int x, int y)
{
    if (!m_viewportRc.contains(Point(x, y)))
    {
        return;
    }

    if (m_caretPositions.empty()) {
        m_pos = 0;
        updateViewportAndCaret();
        updateCaretPosition();
        return;
    }

    // Find the closest caret position
    // m_caretPositions are WITHOUT scroll, so add scrollOffsetX for comparison
    size_t bestCharIndex = 0;
    int bestDist = INT_MAX;

    for (size_t i = 0; i < m_caretPositions.size(); i++) {
        int caretXWithScroll = m_caretPositions[i].x + m_textOffsetX;
        int dist = std::abs(x - caretXWithScroll);
        if (dist < bestDist) {
            bestDist = dist;
            bestCharIndex = i;
        }
    }

#ifdef _WIN32
    m_pos = bestCharIndex;
#else
    size_t bytePos = 0;
    for (size_t i = 0; i < bestCharIndex && bytePos < m_text.length(); i++) {
        bytePos = charNextPos(m_text, bytePos);
    }
    m_pos = bytePos;
#endif
    
    // Update caret position directly (don't change scroll)
    if (bestCharIndex < m_caretPositions.size()) {
        m_caretPosX = m_caretPositions[bestCharIndex].x + m_textOffsetX;
        m_caretPosY = m_caretPositions[bestCharIndex].y;
        updateCaretPosition();
    }
    
    update();
}

void UILineEdit::defaultCaretRect(Rect &rc)
{
#ifdef _WIN32
    HDC hdc = GetDC(m_hWindow);
    HGDIOBJ oldFont = SelectObject(hdc, m_hFont);

    TEXTMETRICW tm = {0};
    GetTextMetricsW(hdc, &tm);
    int x = m_viewportRc.x;
    int y = m_viewportRc.y;
    int width = roundl(DEFAULT_CARET_WIDTH * m_dpi_ratio);
    int height = tm.tmHeight;
    SelectObject(hdc, oldFont);
    ReleaseDC(m_hWindow, hdc);
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(m_hWindow, "A");
    pango_layout_set_font_description(lut, m_hFont->desc);
    pango_layout_set_wrap(lut, PANGO_WRAP_WORD);
    int x = m_viewportRc.x;
    int y = m_viewportRc.y;
    int width, height;
    pango_layout_get_size(lut, &width, &height);
    width = roundl(DEFAULT_CARET_WIDTH * m_dpi_ratio);
    height /= PANGO_SCALE;
    g_object_unref(lut);
#endif
    int algn = metrics()->value(Metrics::TextAlignment);
    // if (algn & Metrics::AlignHCenter) {
    //     if (!multiline)
    //         x += round((m_viewportRc.width - width) / 2.0);
    // } else
    // if (algn & Metrics::AlignHRight) {
    //     if (!multiline)
    //         x += m_viewportRc.width - width;
    // }

    if (algn & Metrics::AlignVCenter) {
        y += round((m_viewportRc.height - height) / 2.0);
    } else
    if (algn & Metrics::AlignVBottom) {
        y += m_viewportRc.height - height;
    }

    rc.x = x;
    rc.y = y;
    rc.width = width;
    rc.height = height;
}
