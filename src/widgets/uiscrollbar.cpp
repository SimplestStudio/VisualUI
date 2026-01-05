#include "uiscrollbar.h"
#include "uidraghandler.h"

#define THUMB_MIN_SIZE 20

UIScrollBar::UIScrollBar(Orientation orientation, UIWidget *parent)
    : UIWidget(parent, ObjectType::WidgetType)
    , m_parent(parent)
    , m_orientation(orientation)
    , m_minimum(0)
    , m_maximum(100)
    , m_value(0)
    , m_pageStep(10)
    , m_trackLength(-1)
{
    UIDragHandler *dragHandler = new UIDragHandler(this);

    if (m_orientation == Vertical) {
        dragHandler->restrictMovementX(true);
    } else {
        dragHandler->restrictMovementY(true);
    }
    
    dragHandler->onMoveValidation([this](int newX, int newY) -> bool {
        int pos = m_orientation == Vertical ? newY : newX;
        int maxPos = maxThumbPos();
        
        if (pos < 0 || pos > maxPos)
            return false;
        
        onThumbMoved(pos);
        return true;
    });
    
    setDragHandler(dragHandler);
    bringAboveSiblings();
}

UIScrollBar::~UIScrollBar()
{

}

void UIScrollBar::setRange(int min, int max) noexcept
{
    m_minimum = min;
    m_maximum = max;
    if (m_value < m_minimum) m_value = m_minimum;
    if (m_value > m_maximum) m_value = m_maximum;
}

void UIScrollBar::setPageStep(int step) noexcept
{
    m_pageStep = step;
}

void UIScrollBar::setValue(int value) noexcept
{
    if (value < m_minimum) value = m_minimum;
    if (value > m_maximum) value = m_maximum;
    
    if (m_value != value) {
        m_value = value;
        
        int maxPos = maxThumbPos();
        int range = m_maximum - m_minimum;
        
        if (range > 0 && maxPos > 0) {
            float ratio = (float)(m_value - m_minimum) / range;
            int thumbPos = ratio * maxPos;
            
            Point currentPos = pos();
            if (m_orientation == Vertical) {
                move(currentPos.x, thumbPos);
            } else {
                move(thumbPos, currentPos.y);
            }
        }
    }
}

void UIScrollBar::setTrackLength(int length) noexcept
{
    m_trackLength = length;
}

void UIScrollBar::updateThumbGeometry(int visibleSize, int totalSize)
{
    if (totalSize <= 0) return;
    
    float ratio = (float)visibleSize / totalSize;
    if (ratio >= 1.0f) {
        if (isVisible())
            hide();
        return;
    }
        
    Size currentSize = size();
    int thumbSize = ratio * visibleSize;

    if (thumbSize < THUMB_MIN_SIZE * m_dpi_ratio)
        thumbSize = THUMB_MIN_SIZE * m_dpi_ratio;
    
    if (m_orientation == Vertical) {
        setBaseSize(currentSize.width / m_dpi_ratio, thumbSize / m_dpi_ratio);
    } else {
        setBaseSize(thumbSize / m_dpi_ratio, currentSize.height / m_dpi_ratio);
    }

    if (!isVisible())
        show();
}

int UIScrollBar::value() const noexcept
{
    return m_value;
}

int UIScrollBar::maxThumbPos() const noexcept
{
    if (!m_parent) return 0;

    Size parentSize = m_parent->size();
    Size thumbSize = size();

    int trackLen = (m_trackLength >= 0) ? m_trackLength : (m_orientation == Vertical ? parentSize.height : parentSize.width);

    int maxPos = trackLen - (m_orientation == Vertical ? thumbSize.height : thumbSize.width);
    return maxPos > 0 ? maxPos : 0;
}

void UIScrollBar::onThumbMoved(int pos)
{
    int maxPos = maxThumbPos();
    if (maxPos > 0) {
        // Add tolerance for boundary values ​​(1% of the range or at least 2 pixels)
        int threshold = std::max<int>(2, maxPos / 100);
        
        // Check the boundary values ​​with tolerance
        if (pos <= threshold) {
            // Scrollbar close to top/left edge
            if (m_value != m_minimum) {
                m_value = m_minimum;
                valueChanged.emit(m_value);
            }
            return;
        }
        if (pos >= maxPos - threshold) {
            // Scrollbar close to bottom/right edge
            if (m_value != m_maximum) {
                m_value = m_maximum;
                valueChanged.emit(m_value);
            }
            return;
        }
        
        float ratio = (float)pos / maxPos;
        int range = m_maximum - m_minimum;
        int newValue = m_minimum + (int)(ratio * range + 0.5f); // Rounding
        
        if (m_value != newValue) {
            m_value = newValue;
            valueChanged.emit(m_value);
        }
    } else {
        // If maxPos == 0, set to initial position
        if (m_value != m_minimum) {
            m_value = m_minimum;
            valueChanged.emit(m_value);
        }
    }
}
