#include "uiabstracteditcontrol.h"
#include "uimetrics.h"
#include "uifontmetrics.h"
#include "uiapplication.h"
#include "uibutton.h"
#include "uimenu.h"
#include "uicursor.h"
#include "uiutils.h"
#include <vector>
#ifdef _WIN32
# include <windowsx.h>
#else
# include "cmath"
# include "widgets/linux/gtkcaret.h"
#endif

#define DEFAULT_CARET_WIDTH 1
#define DEFUALT_MENU_ITEM_COUNT  3
#define DEFAULT_MENU_ITEM_HEIGHT 26

using namespace UIUnicode;

static tstring GetClipboardText(PlatformWindow hWindow)
{
    tstring clipboardText;
#ifdef _WIN32
    if (OpenClipboard(hWindow)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText) {
                clipboardText.assign(pszText);
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }
#else
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gchar *text = gtk_clipboard_wait_for_text(clipboard);
    if (text) {
        clipboardText.assign(text);
        g_free(text);
    }
#endif
    return clipboardText;
}

UIAbstractEditControl::UIAbstractEditControl(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this),
    m_editable(true),
    m_pos(0),
    m_caretPosX(0),
    m_caretPosY(0),
    m_textOffsetX(0),
    m_textOffsetY(0),
    m_caretPositions(),
#ifdef _WIN32
    m_caretCreated(false),
#else
    m_caret(nullptr),
#endif
    m_menu(nullptr),
    m_sourceText(text)
{

}

UIAbstractEditControl::~UIAbstractEditControl()
{
    if (m_menu) {
        delete m_menu; m_menu = nullptr;
    }
}

void UIAbstractEditControl::setText(const tstring &text)
{
    UIAbstractButton::setText(text);
    m_sourceText = text;
    m_pos = 0;
    m_textOffsetX = 0;
    m_textOffsetY = 0;

    updateViewportAndCaret();
    updateCaretPosition();
}

void UIAbstractEditControl::setPlaceholderText(const tstring &text)
{
    m_placeholderText = text;
    if (m_text.empty()) {
        updateViewportAndCaret();
        updateCaretPosition();
    }
}

void UIAbstractEditControl::setEditable(bool editable)
{
    m_editable = editable;
}

#ifdef _WIN32
bool UIAbstractEditControl::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
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
        if (m_caretCreated) {
            m_caretCreated = false;
            DestroyCaret();

            if (!m_caretCreated && m_editable) {
                Rect defCaretRc;
                defaultCaretRect(defCaretRc);
                CreateCaret(m_hWindow, NULL, defCaretRc.width, defCaretRc.height);
                m_caretCreated = true;
                SetCaretPos(m_caretPosX, m_caretPosY);
                ShowCaret(m_hWindow);
            }
        }
        return true;
    }

    case WM_SIZE: {
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
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (ch == 0x16) { // V
                    tstring clipboardText = GetClipboardText(m_hWindow);
                    if (!clipboardText.empty()) {
                        m_text.insert(m_pos, clipboardText);
                        m_pos += clipboardText.length();
                    }
                }
            } else {
                m_text.insert(m_pos, 1, ch);
                m_pos++;
            }
            updateViewportAndCaret();
            break;
        }

        updateCaretPosition();
        break;

    case WM_RBUTTONUP: {
        if (!m_disabled) {
            showContextMenu();
        }
        break;
    }

    case WM_RBUTTONDOWN:
        if (!m_disabled)
            SetFocus(m_hWindow);
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
bool UIAbstractEditControl::event(uint ev_type, void *param)
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
                if (kev->state & GDK_CONTROL_MASK) {
                    if (kev->keyval == GDK_KEY_v) {
                        tstring clipboardText = GetClipboardText(m_hWindow);
                        if (!clipboardText.empty()) {
                            m_text.insert(m_pos, clipboardText);
                            m_pos += clipboardText.length();
                        }
                    }
                } else {
                    char utf8[5] = {0};
                    int len = g_unichar_to_utf8(uni, utf8);
                    m_text.insert(m_pos, utf8, len);
                    m_pos += len;
                }
                updateViewportAndCaret();
            }
            break;
        }

        updateCaretPosition();
        return true;
    }

    case GDK_BUTTON_RELEASE: {
        GdkEventButton *bev = (GdkEventButton*)param;
        if (!m_disabled && bev->button == GDK_BUTTON_SECONDARY) {
            showContextMenu();
        }
        break;
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

const tstring& UIAbstractEditControl::displayedText() const
{
    return (m_text.empty() && !m_placeholderText.empty()) ? m_placeholderText : m_text;
}

void UIAbstractEditControl::updateCaretPosition()
{
#ifdef _WIN32
    if (m_caretCreated)
        SetCaretPos(m_caretPosX, m_caretPosY);
#else
    if (m_caret)
        gtk_caret_set_position(GTK_CARET(m_caret), m_caretPosX, m_caretPosY);
#endif
}

void UIAbstractEditControl::defaultCaretRect(Rect &rc)
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

void UIAbstractEditControl::showContextMenu()
{
    if (m_menu) {
        delete m_menu; m_menu = nullptr;
    }

    int x = 0, y = 0;
    UICursor::globalPos(x, y);
    int width = 100 * m_dpi_ratio, height = calculateMenuHeight();
    m_menu = new UIMenu(topLevelWidget(), Rect(x + 6 * m_dpi_ratio, y + 6 * m_dpi_ratio, width, height));
    m_menu->setObjectGroupId(_T("ToolTip"));
    m_menu->aboutToDestroySignal.connect([this]() {
        m_menu = nullptr;
    });

    populateMenu();
    m_menu->showAll();
}

void UIAbstractEditControl::populateMenu()
{
    UIButton *cutBtn = m_menu->addSection(_T("Cut"));
    cutBtn->setObjectGroupId(_T("MenuButton"));
    cutBtn->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    cutBtn->setDisabled(true);

    UIButton *copyBtn = m_menu->addSection(_T("Copy"));
    copyBtn->setObjectGroupId(_T("MenuButton"));
    copyBtn->metrics()->setMetrics(Metrics::TextMarginLeft, 12);
    copyBtn->setDisabled(true);

    UIButton *pasteBtn = m_menu->addSection(_T("Paste"));
    pasteBtn->setObjectGroupId(_T("MenuButton"));
    pasteBtn->metrics()->setMetrics(Metrics::TextMarginLeft, 12);

    tstring clipboardText = GetClipboardText(platformWindow());
    if (clipboardText.empty()) {
        pasteBtn->setDisabled(true);
    } else {
        pasteBtn->clickSignal.connect([this, clipboardText]() {
            m_menu->close();

            m_text.insert(m_pos, clipboardText);
            m_pos += clipboardText.length();
            updateViewportAndCaret();
            updateCaretPosition();
        });
    }
}

int UIAbstractEditControl::calculateMenuHeight() const
{
    int itemsHeight = DEFUALT_MENU_ITEM_COUNT * (DEFAULT_MENU_ITEM_HEIGHT + 1) * m_dpi_ratio;
    int padding = 8 * m_dpi_ratio;
    return itemsHeight + padding;
}
