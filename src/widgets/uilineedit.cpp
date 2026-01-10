#include "uilineedit.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uiutils.h"
#ifdef _WIN32
# include <windowsx.h>
#else
# include "cmath"
#endif

using namespace UIUnicode;

UILineEdit::UILineEdit(UIWidget *parent, const tstring &text) :
    UIAbstractEditControl(parent, text)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
    metrics()->setMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
}

UILineEdit::~UILineEdit()
{

}

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

    // First position at start
    PangoRectangle strong_pos;
    pango_layout_get_cursor_pos(lut, 0, &strong_pos, NULL);
    m_caretPositions.push_back({x + strong_pos.x / PANGO_SCALE, y + strong_pos.y / PANGO_SCALE});

    size_t bytePos = 0;
    while (bytePos < textToShow.length()) {
        bytePos = charNextPos(textToShow, bytePos);
        pango_layout_get_cursor_pos(lut, bytePos, &strong_pos, NULL);
        m_caretPositions.push_back({x + strong_pos.x / PANGO_SCALE, y + strong_pos.y / PANGO_SCALE});
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

    if (m_text.empty() || m_caretPositions.empty()) {
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
