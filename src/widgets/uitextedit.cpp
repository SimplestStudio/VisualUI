#include "uitextedit.h"
#include "uiabstracteditcontrol.h"
#include "uiscrollbar.h"
#include "uidrawningengine.h"
#include "uimetrics.h"
#include "uiapplication.h"
#include "uiutils.h"
#ifdef _WIN32
# include <windowsx.h>
#else
# include "cmath"
#endif


using namespace UIUnicode;

class UITextEditContent : public UIAbstractEditControl
{
public:
    UITextEditContent(UITextEdit *parent, const tstring &text) :
        UIAbstractEditControl(parent, text),
        m_textEdit(parent)
    {
        metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
    }
    
protected:
#ifdef _WIN32
    virtual bool event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result) override
    {
        switch (msg) {
        case WM_KEYDOWN:
            switch (wParam) {
            case VK_UP: {
                size_t newPos = moveUpOneLine(m_pos);
                if (newPos != m_pos) {
                    m_pos = newPos;
                    updateViewportAndCaret();
                }
                break;
            }

            case VK_DOWN: {
                size_t newPos = moveDownOneLine(m_pos);
                if (newPos != m_pos) {
                    m_pos = newPos;
                    updateViewportAndCaret();
                }
                break;
            }

            default:
                break;
            }

            break;

        case WM_CHAR:
            if (!m_editable)
                break;
            switch (wParam) {
            case 0x0D: // Return
                m_text.insert(m_pos, 1, '\n');
                m_pos++;
                updateViewportAndCaret();
                updateCaretPosition();
                return true;

            default:
                break;
            }

            break;

        default:
            break;
        }
        return UIAbstractEditControl::event(msg, wParam, lParam, result);
    }
#else
    virtual bool event(uint ev_type, void *param) override
    {
        switch (ev_type) {
        case GDK_KEY_PRESS: {
            GdkEventKey *kev = (GdkEventKey*)param;
            switch (kev->keyval) {
            case GDK_KEY_Up: {
                size_t newPos = moveUpOneLine(m_pos);
                if (newPos != m_pos) {
                    m_pos = newPos;
                    updateViewportAndCaret();
                }
                break;
            }

            case GDK_KEY_Down: {
                size_t newPos = moveDownOneLine(m_pos);
                if (newPos != m_pos) {
                    m_pos = newPos;
                    updateViewportAndCaret();
                }
                break;
            }

            case GDK_KEY_Return:
            case GDK_KEY_KP_Enter: {
                m_text.insert(m_pos, 1, '\n');
                m_pos += 1;
                updateViewportAndCaret();
                updateCaretPosition();
                return true;
            }

            default:
                break;
            }

            break;
        }

        default:
            break;
        }
        return UIAbstractEditControl::event(ev_type, param);
    }
#endif

    virtual void onPaint(const RECT &rc) override
    {
        UIDrawingEngine *de = engine();
        if (de && !m_text.empty()) {
            de->DrawStringWithLayout(rc, m_text, m_hFont, true, 0, m_textOffsetY);
        }
    }

    virtual void calculateCaretPositions() override
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
        int baseX = m_viewportRc.x;
        int baseY = m_viewportRc.y;
        int lineHeight = tm.tmHeight + tm.tmExternalLeading;
        int len = (int)textToShow.length();

        std::vector<int> dx(len);
        GCP_RESULTS gcp = {0};
        gcp.lStructSize = sizeof(gcp);
        gcp.lpDx = dx.data();
        gcp.nGlyphs = (UINT)dx.size();
        GetCharacterPlacementW(hdc, textToShow.c_str(), len, 0, &gcp, GCP_LIGATE | GCP_DIACRITIC);

        SelectObject(hdc, oldFont);
        ReleaseDC(m_hWindow, hdc);

        // First position at start
        m_caretPositions.push_back({baseX, baseY});

        int currentX = baseX;
        int currentY = baseY;

        for (size_t i = 0; i < (size_t)len; ++i) {
            wchar_t ch = textToShow[i];
            int charWidth = dx[i];

            if (ch == L'\n') {
                // Move to next line
                currentX = baseX;
                currentY += lineHeight;
                m_caretPositions.push_back({currentX, currentY});
                continue;
            } else if (ch == L'\r') {
                // Skip carriage return, position stays same
                m_caretPositions.push_back({currentX, currentY});
                continue;
            }

            // Word wrap - if adding this char exceeds width, wrap to next line
            if (currentX > baseX && currentX - baseX + charWidth > m_viewportRc.width) {
                currentX = baseX;
                currentY += lineHeight;
            }

            currentX += charWidth;
            m_caretPositions.push_back({currentX, currentY});
        }
#else
        PangoLayout *lut = gtk_widget_create_pango_layout(m_hWindow, textToShow.c_str());
        pango_layout_set_font_description(lut, m_hFont->desc);
        pango_layout_set_wrap(lut, PANGO_WRAP_WORD_CHAR);
        pango_layout_set_width(lut, m_viewportRc.width * PANGO_SCALE);

        int baseX = m_viewportRc.x;
        int baseY = m_viewportRc.y;

        // First position at start
        m_caretPositions.push_back({baseX, baseY});

        size_t bytePos = 0;
        while (bytePos < textToShow.length()) {
            PangoRectangle rect;
            pango_layout_index_to_pos(lut, bytePos, &rect);

            int caretX = baseX + (rect.x + rect.width) / PANGO_SCALE;
            int caretY = baseY + rect.y / PANGO_SCALE;

            m_caretPositions.push_back({caretX, caretY});
            bytePos = charNextPos(textToShow, bytePos);
        }

        g_object_unref(lut);
#endif
    }

    virtual void updateViewportAndCaret() override
    {
        if (m_pos > m_text.length())
            m_pos = m_text.length();

        if (!m_textEdit)
            return;

        // Recalculate content height (for scrollbar)
        m_textEdit->calculateContentHeight();

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
            int caretYClean = m_caretPositions[pos].y;

            // Horizontal scrolling (for long lines without wrap - disabled for multiline)
            m_textOffsetX = 0;

            // Vertical scrolling - ensure caret is visible
            int lineHeight = m_textEdit->getLineHeight();
            int caretY = caretYClean + m_textOffsetY;
            int viewportTop = m_viewportRc.y;
            int viewportBottom = m_viewportRc.y + m_viewportRc.height - lineHeight;

            if (caretY < viewportTop) {
                // Caret is above viewport - scroll up
                m_textEdit->m_scrollOffsetY = -(caretYClean - m_viewportRc.y);
                m_textOffsetY = m_textEdit->m_scrollOffsetY;
            } else if (caretY > viewportBottom) {
                // Caret is below viewport - scroll down
                m_textEdit->m_scrollOffsetY = -(caretYClean - viewportBottom);
                m_textOffsetY = m_textEdit->m_scrollOffsetY;
            }

            // Clamp scroll offset
            int maxOffset = m_textEdit->maxScrollOffsetY();
            if (m_textEdit->m_scrollOffsetY < -maxOffset) {
                m_textEdit->m_scrollOffsetY = -maxOffset;
                m_textOffsetY = m_textEdit->m_scrollOffsetY;
            }
            if (m_textEdit->m_scrollOffsetY > 0) {
                m_textEdit->m_scrollOffsetY = 0;
                m_textOffsetY = 0;
            }

            // Update caret position with scroll offset
            m_caretPosX = caretXClean + m_textOffsetX;
            m_caretPosY = caretYClean + m_textOffsetY;

            // Update scrollbar position
            if (m_textEdit->m_verticalScrollBar) {
                m_textEdit->m_verticalScrollBar->setValue(-m_textEdit->m_scrollOffsetY);
            }
        }

        update();
    }

    virtual void updateCaretOnMouseClick(int x, int y) override
    {
        if (!m_viewportRc.contains(Point(x, y)))
        {
            return;
        }

        // Don't react to clicks on placeholder text - keep caret at start
        if (m_text.empty() || m_caretPositions.empty()) {
            m_pos = 0;
            updateViewportAndCaret();
            updateCaretPosition();
            return;
        }

        // Find the closest caret position considering both X and Y with scroll offsets
        size_t bestCharIndex = 0;
        int bestDist = INT_MAX;

        for (size_t i = 0; i < m_caretPositions.size(); i++) {
            int caretXWithScroll = m_caretPositions[i].x + m_textOffsetX;
            int caretYWithScroll = m_caretPositions[i].y + m_textOffsetY;

            // Calculate 2D distance
            int distX = std::abs(x - caretXWithScroll);
            int distY = std::abs(y - caretYWithScroll);
            int dist = distX + distY;  // Manhattan distance

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
            m_caretPosY = m_caretPositions[bestCharIndex].y + m_textOffsetY;
            updateCaretPosition();
        }

        update();
    }
    
private:
    size_t moveUpOneLine(size_t pos) const
    {
        if (!m_textEdit || m_caretPositions.empty() || m_textEdit->m_lineHeight <= 0)
            return pos;
        
#ifdef _WIN32
        size_t charIndex = (pos < m_caretPositions.size()) ? pos : m_caretPositions.size() - 1;
#else
        size_t charIndex = 0;
        size_t bytePos = 0;
        while (bytePos < pos && bytePos < m_text.length()) {
            bytePos = charNextPos(m_text, bytePos);
            charIndex++;
        }
        charIndex = (charIndex < m_caretPositions.size()) ? charIndex : m_caretPositions.size() - 1;
#endif
        
        int currentX = m_caretPositions[charIndex].x;
        int currentY = m_caretPositions[charIndex].y;
        int targetY = currentY - m_textEdit->m_lineHeight;
        
        if (targetY < m_viewportRc.y)
            return pos;  // Already at top line
        
        // Find the closest position on the previous line
        size_t bestIndex = charIndex;
        int bestDist = INT_MAX;
        
        for (size_t i = 0; i < m_caretPositions.size(); i++) {
            if (m_caretPositions[i].y == targetY) {
                int dist = std::abs(m_caretPositions[i].x - currentX);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIndex = i;
                }
            }
        }
        
        if (bestIndex == charIndex)
            return pos;  // No change
        
#ifdef _WIN32
        return bestIndex;
#else
        // Convert char index back to byte position
        bytePos = 0;
        for (size_t i = 0; i < bestIndex && bytePos < m_text.length(); i++) {
            bytePos = charNextPos(m_text, bytePos);
        }
        return bytePos;
#endif
    }

    size_t moveDownOneLine(size_t pos) const
    {
        if (!m_textEdit || m_caretPositions.empty() || m_textEdit->m_lineHeight <= 0)
            return pos;
        
#ifdef _WIN32
        size_t charIndex = (pos < m_caretPositions.size()) ? pos : m_caretPositions.size() - 1;
#else
        size_t charIndex = 0;
        size_t bytePos = 0;
        while (bytePos < pos && bytePos < m_text.length()) {
            bytePos = charNextPos(m_text, bytePos);
            charIndex++;
        }
        charIndex = (charIndex < m_caretPositions.size()) ? charIndex : m_caretPositions.size() - 1;
#endif
        
        int currentX = m_caretPositions[charIndex].x;
        int currentY = m_caretPositions[charIndex].y;
        int targetY = currentY + m_textEdit->m_lineHeight;
        
        // Find the closest position on the next line
        size_t bestIndex = charIndex;
        int bestDist = INT_MAX;
        
        for (size_t i = 0; i < m_caretPositions.size(); i++) {
            if (m_caretPositions[i].y == targetY) {
                int dist = std::abs(m_caretPositions[i].x - currentX);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIndex = i;
                }
            }
        }
        
        if (bestIndex == charIndex)
            return pos;  // No change (already at last line)
        
#ifdef _WIN32
        return bestIndex;
#else
        // Convert char index back to byte position
        bytePos = 0;
        for (size_t i = 0; i < bestIndex && bytePos < m_text.length(); i++) {
            bytePos = charNextPos(m_text, bytePos);
        }
        return bytePos;
#endif
    }

private:
    friend class UITextEdit;
    UITextEdit *m_textEdit = nullptr;
};


UITextEdit::UITextEdit(UIWidget *parent, const tstring &text) :
    UIAbstractScrollArea(parent),
    m_lineHeight(0),
    m_contentHeight(0)
{
    metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);    
    setHorizontalScrollBarPolicy(ScrollBarAlwaysOff);

    UITextEditContent *content = new UITextEditContent(this, text);
    content->setObjectGroupId(_T("TextEditViewport"));
    setContentWidget(content);
}

UITextEdit::~UITextEdit()
{

}

void UITextEdit::setText(const tstring &text) noexcept
{
    if (UITextEditContent *content = dynamic_cast<UITextEditContent*>(m_contentWidget)) {
        content->setText(text);
    }
}

void UITextEdit::setPlaceholderText(const tstring &text) noexcept
{
    if (UITextEditContent *content = dynamic_cast<UITextEditContent*>(m_contentWidget)) {
        content->setPlaceholderText(text);
    }
}

tstring UITextEdit::text() const noexcept
{
    if (UITextEditContent *content = dynamic_cast<UITextEditContent*>(m_contentWidget)) {
        return content->text();
    }
    return {};
}

void UITextEdit::onScrollOffsetChanged()
{
    UITextEditContent *content = dynamic_cast<UITextEditContent*>(m_contentWidget);
    if (!content)
        return;
    
    // Sync text offset with scroll offset from UIAbstractScrollArea
    // For text editor we don't move the widget, we use offset for text rendering
    content->m_textOffsetY = m_scrollOffsetY;
    
    // Update caret Y position with new scroll offset
    if (!content->m_caretPositions.empty()) {
#ifdef _WIN32
        size_t pos = (content->m_pos < content->m_caretPositions.size()) ? content->m_pos : content->m_caretPositions.size() - 1;
#else
        size_t charIndex = 0;
        size_t bytePos = 0;
        while (bytePos < content->m_pos && bytePos < content->m_text.length()) {
            bytePos = charNextPos(content->m_text, bytePos);
            charIndex++;
        }
        size_t pos = (charIndex < content->m_caretPositions.size()) ? charIndex : content->m_caretPositions.size() - 1;
#endif
        content->m_caretPosY = content->m_caretPositions[pos].y + content->m_textOffsetY;
        content->updateCaretPosition();
    }
    
    content->update();
}

void UITextEdit::calculateContentHeight()
{
    UITextEditContent *content = dynamic_cast<UITextEditContent*>(m_contentWidget);
    if (!content)
        return;
    
    Size contentSize = content->size();
    if (contentSize.width <= 0 || contentSize.height <= 0)
        return;
    
    int viewportWidth = contentSize.width;

    const tstring &textToShow = content->displayedText();
    
#ifdef _WIN32
    HDC hdc = GetDC(content->m_hWindow);
    HGDIOBJ oldFont = SelectObject(hdc, content->m_hFont);

    TEXTMETRICW tm = {0};
    GetTextMetricsW(hdc, &tm);
    m_lineHeight = tm.tmHeight + tm.tmExternalLeading;
    
    if (textToShow.empty()) {
        m_contentHeight = m_lineHeight;
    } else {
        int len = (int)textToShow.length();
        std::vector<int> dx(len);
        GCP_RESULTS gcp = {0};
        gcp.lStructSize = sizeof(gcp);
        gcp.lpDx = dx.data();
        gcp.nGlyphs = (UINT)dx.size();
        GetCharacterPlacementW(hdc, textToShow.c_str(), len, 0, &gcp, GCP_LIGATE | GCP_DIACRITIC);
        
        int lineCount = 1;
        int currentLineWidth = 0;
        
        for (size_t i = 0; i < (size_t)len; ++i) {
            wchar_t ch = textToShow[i];
            
            if (ch == L'\n') {
                lineCount++;
                currentLineWidth = 0;
                continue;
            } else if (ch == L'\r') {
                continue;
            }
            
            // Word wrap
            if (currentLineWidth > 0 && currentLineWidth + dx[i] > viewportWidth) {
                lineCount++;
                currentLineWidth = dx[i];
            } else {
                currentLineWidth += dx[i];
            }
        }
        
        m_contentHeight = lineCount * m_lineHeight;
    }
    
    SelectObject(hdc, oldFont);
    ReleaseDC(content->m_hWindow, hdc);
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(content->m_hWindow, textToShow.empty() ? "A" : textToShow.c_str());
    pango_layout_set_font_description(lut, content->m_hFont->desc);
    pango_layout_set_wrap(lut, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(lut, viewportWidth * PANGO_SCALE);
    
    int width, height;
    pango_layout_get_size(lut, &width, &height);
    m_contentHeight = height / PANGO_SCALE;
    
    // Get line height
    PangoLayoutIter *iter = pango_layout_get_iter(lut);
    if (iter) {
        PangoRectangle logical_rect;
        pango_layout_iter_get_line_extents(iter, nullptr, &logical_rect);
        m_lineHeight = logical_rect.height / PANGO_SCALE;
        pango_layout_iter_free(iter);
    }
    
    g_object_unref(lut);
#endif
    
    // Set content height for scrolling
    setContentHeight(m_contentHeight);
    setWheelScrollStep(m_lineHeight > 0 ? m_lineHeight : 20);
}

int UITextEdit::getLineHeight() const
{
    return m_lineHeight > 0 ? m_lineHeight : 20;
}
