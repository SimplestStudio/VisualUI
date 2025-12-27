#include "uiabstractscrollarea.h"
#include "uiscrollbar.h"

static const int DEFAULT_WHEEL_SCROLL_STEP = 3;
static const int SCROLLBAR_BASE_THICKNESS = 11;
static const int SCROLLBAR_OFFSET = 8;

UIAbstractScrollArea::UIAbstractScrollArea(UIWidget *parent) :
    UIWidget(parent, ObjectType::WidgetType),
    m_contentWidget(nullptr),
    m_verticalScrollBar(nullptr),
    m_horizontalScrollBar(nullptr),
    m_scrollOffsetX(0),
    m_scrollOffsetY(0),
    m_wheelScrollStep(DEFAULT_WHEEL_SCROLL_STEP),
    m_contentHeight(0),
    m_contentWidth(0),
    m_verticalScrollBarPolicy(ScrollBarAsNeeded),
    m_horizontalScrollBarPolicy(ScrollBarAsNeeded),
    m_verticalContentResizeMode(ResizeToVisible),
    m_horizontalContentResizeMode(ResizeToVisible)
{
    resizeSignal.connect([this](int w, int h) {
        if (m_contentWidget) {
            int maxOffsetY = maxScrollOffsetY();
            int maxOffsetX = maxScrollOffsetX();
            bool offsetChanged = false;
            
            if (m_scrollOffsetY < -maxOffsetY) {
                m_scrollOffsetY = -maxOffsetY;
                if (m_scrollOffsetY > 0)
                    m_scrollOffsetY = 0;
                offsetChanged = true;
            }

            if (m_scrollOffsetX < -maxOffsetX) {
                m_scrollOffsetX = -maxOffsetX;
                if (m_scrollOffsetX > 0)
                    m_scrollOffsetX = 0;
                offsetChanged = true;
            }
            
            updateScrollBarGeometry();
            updateScrollBarPosition();
            
            if (offsetChanged) {
                onScrollOffsetChanged();
                scrollPositionChanged.emit(m_scrollOffsetY);
            }
        }
    });
}

UIAbstractScrollArea::~UIAbstractScrollArea()
{

}

void UIAbstractScrollArea::setContentWidget(UIWidget *widget) noexcept
{
    m_contentWidget = widget;
#ifdef _WIN32
    m_contentWidget->enableClipSiblings();
#endif
    m_contentWidget->move(0, 0);
}

void UIAbstractScrollArea::setContentWidth(int width)
{
    m_contentWidth = width;
    if (m_contentWidget) {
        updateScrollBarGeometry();
        updateScrollBarPosition();
    }
}

void UIAbstractScrollArea::setContentHeight(int height)
{
    m_contentHeight = height;
    if (m_contentWidget) {
        updateScrollBarGeometry();
        updateScrollBarPosition();
    }
}

void UIAbstractScrollArea::setContentSize(int width, int height)
{
    m_contentWidth = width;
    m_contentHeight = height;
    if (m_contentWidget) {
        updateScrollBarGeometry();
        updateScrollBarPosition();
    }
}

void UIAbstractScrollArea::setWheelScrollStep(int step) noexcept
{
    m_wheelScrollStep = step;
}

void UIAbstractScrollArea::setVerticalScrollBarPolicy(ScrollBarPolicy policy)
{
    m_verticalScrollBarPolicy = policy;
    updateScrollBarGeometry();
}

void UIAbstractScrollArea::setHorizontalScrollBarPolicy(ScrollBarPolicy policy)
{
    m_horizontalScrollBarPolicy = policy;
    updateScrollBarGeometry();
}

void UIAbstractScrollArea::setVerticalContentResizeMode(ContentResizeMode mode)
{
    m_verticalContentResizeMode = mode;
    updateScrollBarGeometry();
}

void UIAbstractScrollArea::setHorizontalContentResizeMode(ContentResizeMode mode)
{
    m_horizontalContentResizeMode = mode;
    updateScrollBarGeometry();
}

void UIAbstractScrollArea::scrollToTop()
{
    m_scrollOffsetY = 0;

    onScrollOffsetChanged();

    if (m_verticalScrollBar) {
        m_verticalScrollBar->setValue(0);
        m_verticalScrollBar->move(m_verticalScrollBar->pos().x, 0);
    }

#ifdef __linux__
    updateGeometry();
#endif
}

void UIAbstractScrollArea::scrollToBottom()
{
    int maxOffset = maxScrollOffsetY();
    m_scrollOffsetY = -maxOffset;

    onScrollOffsetChanged();

    if (m_verticalScrollBar) {
        m_verticalScrollBar->setValue(maxOffset);
    }

#ifdef __linux__
    updateGeometry();
#endif
}

void UIAbstractScrollArea::scrollToLeft()
{
    m_scrollOffsetX = 0;

    onScrollOffsetChanged();

    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->setValue(0);
        m_horizontalScrollBar->move(0, m_horizontalScrollBar->pos().y);
    }

#ifdef __linux__
    updateGeometry();
#endif
}

void UIAbstractScrollArea::scrollToRight()
{
    int maxOffset = maxScrollOffsetX();
    m_scrollOffsetX = -maxOffset;

    onScrollOffsetChanged();

    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->setValue(maxOffset);
    }

#ifdef __linux__
    updateGeometry();
#endif
}

UIWidget *UIAbstractScrollArea::contentWidget() noexcept
{
    return m_contentWidget;
}

int UIAbstractScrollArea::wheelScrollStep() const  noexcept
{
    return m_wheelScrollStep;
}

int UIAbstractScrollArea::scrollBarThickness() const noexcept
{
    return SCROLLBAR_BASE_THICKNESS * m_dpi_ratio;
}

UIAbstractScrollArea::ScrollBarPolicy UIAbstractScrollArea::verticalScrollBarPolicy() const noexcept
{
    return m_verticalScrollBarPolicy;
}

UIAbstractScrollArea::ScrollBarPolicy UIAbstractScrollArea::horizontalScrollBarPolicy() const noexcept
{
    return m_horizontalScrollBarPolicy;
}

UIAbstractScrollArea::ContentResizeMode UIAbstractScrollArea::verticalContentResizeMode() const noexcept
{
    return m_verticalContentResizeMode;
}

UIAbstractScrollArea::ContentResizeMode UIAbstractScrollArea::horizontalContentResizeMode() const noexcept
{
    return m_horizontalContentResizeMode;
}

#ifdef _WIN32
bool UIAbstractScrollArea::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        delta *= m_wheelScrollStep * m_dpi_ratio;
        onMouseWheel(delta);
        *result = 0;
        return true;
    }
    default:
        break;
    }

    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIAbstractScrollArea::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_SCROLL: {
        GdkEventScroll *scroll = (GdkEventScroll *)param;
        int delta = -scroll->delta_y * m_wheelScrollStep * m_dpi_ratio;
        onMouseWheel(delta);
        return GDK_EVENT_STOP;
    }
    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}
#endif

int UIAbstractScrollArea::maxScrollOffsetX() const noexcept
{
    if (!m_contentWidget)
        return 0;

    ScrollBarState state = calculateScrollBarState();
    int maxOffset = m_contentWidth - state.availableWidth;
    return maxOffset > 0 ? maxOffset : 0;
}

int UIAbstractScrollArea::maxScrollOffsetY() const noexcept
{
    if (!m_contentWidget)
        return 0;

    ScrollBarState state = calculateScrollBarState();
    int maxOffset = m_contentHeight - state.availableHeight;
    return maxOffset > 0 ? maxOffset : 0;
}

UIAbstractScrollArea::ScrollBarState UIAbstractScrollArea::calculateScrollBarState() const noexcept
{
    ScrollBarState state = { false, false, 0, 0 };
    
    Size sz = size();
    int sbThickness = scrollBarThickness();
    
    state.availableWidth = sz.width;
    state.availableHeight = sz.height;
    
    // Check policies - AlwaysOn first
    if (m_verticalScrollBarPolicy == ScrollBarAlwaysOn) {
        state.needVertical = true;
        state.availableWidth -= sbThickness;
    }
    if (m_horizontalScrollBarPolicy == ScrollBarAlwaysOn) {
        state.needHorizontal = true;
        state.availableHeight -= sbThickness;
    }
    
    // Check content size if AsNeeded
    if (m_verticalScrollBarPolicy == ScrollBarAsNeeded && !state.needVertical) {
        if (m_contentHeight > state.availableHeight) {
            state.needVertical = true;
            state.availableWidth -= sbThickness;
        }
    }
    
    if (m_horizontalScrollBarPolicy == ScrollBarAsNeeded && !state.needHorizontal) {
        if (m_contentWidth > state.availableWidth) {
            state.needHorizontal = true;
            state.availableHeight -= sbThickness;
        }
    }
    
    // Re-check vertical if horizontal was added and vertical is AsNeeded
    if (m_verticalScrollBarPolicy == ScrollBarAsNeeded && !state.needVertical) {
        if (m_contentHeight > state.availableHeight) {
            state.needVertical = true;
            state.availableWidth -= sbThickness;
        }
    }
    
    return state;
}

void UIAbstractScrollArea::onMouseWheel(int delta)
{
    int maxOffset = maxScrollOffsetY();
    
    m_scrollOffsetY += delta;
    
    if (m_scrollOffsetY > 0) {
        m_scrollOffsetY = 0;
    } else
    if (m_scrollOffsetY < -maxOffset) {
        m_scrollOffsetY = -maxOffset;
    }

    onScrollOffsetChanged();
    
    if (m_verticalScrollBar) {
        m_verticalScrollBar->setValue(-m_scrollOffsetY);
    }
    
    scrollPositionChanged.emit(m_scrollOffsetY);
    
#ifdef __linux__
    updateGeometry();
#endif
}

void UIAbstractScrollArea::createScrollBars()
{
    if (!m_verticalScrollBar) {
        m_verticalScrollBar = new UIScrollBar(UIScrollBar::Vertical, this);
        m_verticalScrollBar->setRange(0, 0);
        m_verticalScrollBar->setBaseSize(5, 40);
        m_verticalScrollBar->hide();

        m_verticalScrollBar->valueChanged.connect([this](int value) {
            m_scrollOffsetY = -value;
            onScrollOffsetChanged();
#ifdef __linux__
            updateGeometry();
#endif
            scrollPositionChanged.emit(m_scrollOffsetY);
        });
    }

    if (!m_horizontalScrollBar) {
        m_horizontalScrollBar = new UIScrollBar(UIScrollBar::Horizontal, this);
        m_horizontalScrollBar->setRange(0, 0);
        m_horizontalScrollBar->setBaseSize(40, 5);
        m_horizontalScrollBar->hide();

        m_horizontalScrollBar->valueChanged.connect([this](int value) {
            m_scrollOffsetX = -value;
            onScrollOffsetChanged();
#ifdef __linux__
            updateGeometry();
#endif
        });
    }
}

void UIAbstractScrollArea::updateScrollBarGeometry()
{
    if (!m_contentWidget)
        return;
    
    ScrollBarState state = calculateScrollBarState();
    Size sz = size();
    int sbOffset = SCROLLBAR_OFFSET * m_dpi_ratio;
    
    if ((state.needVertical || state.needHorizontal) && (!m_verticalScrollBar || !m_horizontalScrollBar)) {
        createScrollBars();
    }
    
    // Vertical ScrollBar
    if (state.needVertical) {
        int maxOffset = m_contentHeight - state.availableHeight;
        m_verticalScrollBar->setRange(0, maxOffset);
        m_verticalScrollBar->setTrackLength(state.availableHeight);
        m_verticalScrollBar->updateThumbGeometry(state.availableHeight, m_contentHeight);
        
        if (!m_verticalScrollBar->isVisible()) {
            m_verticalScrollBar->show();
        }
        
        Size thumbSize = m_verticalScrollBar->size();
        int maxScrollBarPos = std::max<int>(0, state.availableHeight - thumbSize.height);
        
        int scrollBarY = 0;
        if (maxOffset > 0) {
            scrollBarY = std::min<int>((-m_scrollOffsetY * maxScrollBarPos) / maxOffset, maxScrollBarPos);
        }
        
        m_verticalScrollBar->move(sz.width - sbOffset, scrollBarY);
    } else if (m_verticalScrollBar && m_verticalScrollBar->isVisible()) {
        m_verticalScrollBar->hide();
    }

    // Horizontal ScrollBar
    if (state.needHorizontal) {
        int maxOffset = m_contentWidth - state.availableWidth;
        m_horizontalScrollBar->setRange(0, maxOffset);
        m_horizontalScrollBar->setTrackLength(state.availableWidth);
        m_horizontalScrollBar->updateThumbGeometry(state.availableWidth, m_contentWidth);
        
        if (!m_horizontalScrollBar->isVisible()) {
            m_horizontalScrollBar->show();
        }
        
        Size thumbSize = m_horizontalScrollBar->size();
        int maxScrollBarPos = std::max<int>(0, state.availableWidth - thumbSize.width);
        
        int scrollBarX = 0;
        if (maxOffset > 0) {
            scrollBarX = std::min<int>((-m_scrollOffsetX * maxScrollBarPos) / maxOffset, maxScrollBarPos);
        }
        
        m_horizontalScrollBar->move(scrollBarX, sz.height - sbOffset);
    } else if (m_horizontalScrollBar && m_horizontalScrollBar->isVisible()) {
        m_horizontalScrollBar->hide();
    }
    
    // Resize content widget
    int viewportW = (m_horizontalContentResizeMode == ResizeToContent) 
        ? std::max<int>(state.availableWidth, m_contentWidth) 
        : state.availableWidth;
        
    int viewportH = (m_verticalContentResizeMode == ResizeToContent) 
        ? std::max<int>(state.availableHeight, m_contentHeight) 
        : state.availableHeight;

    m_contentWidget->resize(viewportW, viewportH);
}

void UIAbstractScrollArea::updateScrollBarPosition()
{
    if (m_verticalScrollBar) {
        m_verticalScrollBar->setValue(-m_scrollOffsetY);
    }
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->setValue(-m_scrollOffsetX);
    }
}
