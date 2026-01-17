#include "uilistview.h"
#include "uibutton.h"
#include "uitoolbutton.h"
#include "uiscrollbar.h"
#include <algorithm>

UIListView::UIListView(UIWidget *parent) :
    UIAbstractScrollArea(parent),
    m_firstVisibleIndex(0),
    m_lastVisibleIndex(0),
    m_rowBaseHeight(32),
    m_currentIndex(-1),
    m_itemCount(0),
    m_activateOnMouseUp(false),
    m_scrollMode(ScrollPerItem)
{
    UIWidget *content = new UIWidget(this);
    content->setObjectGroupId(_T("ListViewViewport"));
    setContentWidget(content);
    setWheelScrollStep(m_rowBaseHeight);

    resizeSignal.connect([this](int, int) {
        updateVisibleItems();
    });
}

UIListView::~UIListView()
{

}

void UIListView::addItem(const tstring &text, uintptr_t data)
{
    m_items.push_back({text, data});
    m_itemCount++;
    
    int totalHeight = m_itemCount * m_rowBaseHeight * m_dpi_ratio;
    setContentHeight(totalHeight);
    
    updateVisibleItems();
#ifdef __linux__
    updateGeometry();
#endif
}

void UIListView::removeItem(int index)
{
    if (index < 0 || index >= m_itemCount) return;
    
    m_items.erase(m_items.begin() + index);
    m_itemCount--;
    
    bool changed = false;
    if (m_currentIndex == index) {
        m_currentIndex = -1;
        changed = true;
    } else if (m_currentIndex > index) {
        m_currentIndex--;
        changed = true;
    }

    if (changed) {
        indexChangedSignal.emit(m_currentIndex);
    }
    
    int totalHeight = m_itemCount * m_rowBaseHeight * m_dpi_ratio;
    setContentHeight(totalHeight);
    
    // Adjust the scroll position if the content has become smaller
    int maxOffset = maxScrollOffsetY();
    if (m_scrollOffsetY < -maxOffset) {
        m_scrollOffsetY = -maxOffset;
        if (m_scrollOffsetY > 0) {
            m_scrollOffsetY = 0;
        }
        // Important: We call onScrollOffsetChanged because we changed m_scrollOffsetY manually
        onScrollOffsetChanged();
    }
    
    // Update visible elements
    updateVisibleItems();
    
#ifdef __linux__
    updateGeometry();
#endif
}

void UIListView::clearList()
{
    for (UIAbstractButton* widget : m_visibleWidgets) {
        delete widget;
    }
    m_visibleWidgets.clear();
    
    m_items.clear();
    m_itemCount = 0;

    bool changed = (m_currentIndex != -1);
    m_currentIndex = -1;
    m_firstVisibleIndex = 0;
    m_lastVisibleIndex = 0;
    
    if (changed) {
        indexChangedSignal.emit(m_currentIndex);
    }

    setContentHeight(1);
    scrollToTop();
}

void UIListView::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_itemCount) return;

    if (m_currentIndex != index) {
        // Deselect the previous item if it is visible
        if (m_currentIndex != -1) {
            int oldWidgetIndex = m_currentIndex - m_firstVisibleIndex;
            if (oldWidgetIndex >= 0 && oldWidgetIndex < (int)m_visibleWidgets.size()) {
                m_visibleWidgets[oldWidgetIndex]->setSelected(false);
            }
        }

        m_currentIndex = index;

        // Select the new item if it is visible
        if (m_currentIndex != -1) {
            int newWidgetIndex = m_currentIndex - m_firstVisibleIndex;
            if (newWidgetIndex >= 0 && newWidgetIndex < (int)m_visibleWidgets.size()) {
                m_visibleWidgets[newWidgetIndex]->setSelected(true);
            }
        }

        indexChangedSignal.emit(m_currentIndex);
    }
}

void UIListView::setRowBaseHeight(int height) noexcept
{
    m_rowBaseHeight = height;
    if (m_scrollMode == ScrollPerPixel) {
        setWheelScrollStep(m_rowBaseHeight / 3);
    } else {
        setWheelScrollStep(m_rowBaseHeight);
    }
}

void UIListView::setScrollMode(ScrollMode mode) noexcept
{
    m_scrollMode = mode;
    if (m_scrollMode == ScrollPerPixel) {
        setWheelScrollStep(m_rowBaseHeight / 3);
    } else {
        setWheelScrollStep(m_rowBaseHeight);
    }
    updateVisibleItems();
}

void UIListView::setActivateOnMouseUp(bool enable) noexcept
{
    m_activateOnMouseUp = enable;
}

uintptr_t UIListView::itemData(int index) const
{
    if (index < 0 || index >= (int)m_items.size()) return 0;
    return m_items[index].data;
}

int UIListView::currentIndex() const noexcept
{
    return m_currentIndex;
}

int UIListView::rowBaseHeight() const noexcept
{
    return m_rowBaseHeight;
}

UIListView::ScrollMode UIListView::scrollMode() const noexcept
{
    return m_scrollMode;
}

int UIListView::count() const noexcept
{
    return m_itemCount;
}

bool UIListView::containsItem(const tstring &text) const
{
    for (const auto &item : m_items) {
        if (item.text == text) {
            return true;
        }
    }
    return false;
}

#ifdef _WIN32
bool UIListView::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_DPICHANGED_NOTIFY: {
        // Save the relative scroll position before DPI change
        float oldDpiRatio = m_dpi_ratio;
        
        UIAbstractScrollArea::event(msg, wParam, lParam, result);

        if (m_itemCount > 0 && m_rowBaseHeight > 0 && m_dpi_ratio > 0) {
            int totalHeight = m_itemCount * m_rowBaseHeight * m_dpi_ratio;
            setContentHeight(totalHeight);

            // Scale the scroll position proportionally to the new DPI
            if (oldDpiRatio > 0) {
                m_scrollOffsetY = (int)(m_scrollOffsetY * m_dpi_ratio / oldDpiRatio);
            }

            // Clamp to valid range
            int maxOffset = maxScrollOffsetY();
            if (m_scrollOffsetY < -maxOffset) {
                m_scrollOffsetY = -maxOffset;
            }
            if (m_scrollOffsetY > 0) {
                m_scrollOffsetY = 0;
            }
            
            // Synchronize the scrollbar with the new offset
            if (m_verticalScrollBar) {
                m_verticalScrollBar->setValue(-m_scrollOffsetY);
            }

            updateVisibleItems();
        }
        break;
    }
    default:
        break;
    }
    return UIAbstractScrollArea::event(msg, wParam, lParam, result);
}
#else
bool UIListView::event(uint ev_type, void *param)
{
    switch (ev_type) {
    default:
        break;
    }
    return UIAbstractScrollArea::event(ev_type, param);
}
#endif

void UIListView::onScrollOffsetChanged()
{
    updateVisibleItems();
}

void UIListView::createVisibleWidgets()
{
    UIWidget *view = contentWidget();
    if (!view) return;
    
    Size viewportSize = view->size();
    if (viewportSize.height <= 0) return;
    
    // Calculate how many widgets fit into the visible height
    int rowHeight = m_rowBaseHeight * m_dpi_ratio;
    if (rowHeight <= 0) return;
    
    // If there are no elements, hide all widgets and exit to avoid creating empty ones
    if (m_itemCount == 0) {
        for (auto* widget : m_visibleWidgets) {
            if (widget->isVisible()) widget->hide();
        }
        return;
    }

    int maxVisibleCount = (viewportSize.height + rowHeight - 1) / rowHeight + 1; // +1 for partially visible
    
    // Optimization: don't create more widgets than there are items in the list
    if (maxVisibleCount > m_itemCount) {
        maxVisibleCount = m_itemCount;
    }
    
    while ((int)m_visibleWidgets.size() < maxVisibleCount) {
        int widgetIndex = (int)m_visibleWidgets.size();
        UIAbstractButton *btn = nullptr;
        if (m_activateOnMouseUp) {
            btn = new UIButton(view, _T(""));
        } else {
            btn = new UIToolButton(view, _T(""));
        };
#ifdef _WIN32
        btn->enableClipSiblings();
#endif
        btn->setObjectGroupId(_T("ListViewItem"));
        btn->setSelectable(true);
        btn->setBaseSize(50, m_rowBaseHeight);
        
        btn->move(0, widgetIndex * rowHeight);
        btn->resize(viewportSize.width, rowHeight);
        
        btn->clickSignal.connect([this, widgetIndex]() {
            int dataIndex = m_firstVisibleIndex + widgetIndex;
            if (dataIndex >= 0 && dataIndex < m_itemCount) {
                setCurrentIndex(dataIndex);
            }
        });
        
        m_visibleWidgets.push_back(btn);
    }
    
    // Calculate the offset for smoothing at the edges of the list
    int shift = 0;
    int currentScrollOffset = -m_scrollOffsetY;
    
    if (m_scrollMode == ScrollPerPixel) {
        shift = currentScrollOffset - (m_firstVisibleIndex * rowHeight);
    } else {
        // Only move if we are within one line of the start or end of the scroll
        int totalHeight = m_itemCount * rowHeight;
        int maxOffset = maxScrollOffsetY();
        
        bool isTop = (currentScrollOffset < rowHeight);
        
        // Calculate the last point where we can snap to the grid
        int lastSnapPoint = (maxOffset / rowHeight) * rowHeight;
        bool isBottom = (currentScrollOffset >= lastSnapPoint && maxOffset > 0);
        
        if (isTop || isBottom) {
            shift = currentScrollOffset - (m_firstVisibleIndex * rowHeight);
        }
    }

    int visibleCount = m_lastVisibleIndex - m_firstVisibleIndex + 1;
    
    for (int i = 0; i < (int)m_visibleWidgets.size(); ++i) {
        UIAbstractButton *widget = m_visibleWidgets[i];
        
        if (i < visibleCount) {
            int dataIndex = m_firstVisibleIndex + i;
            
            // Update the text
            updateItemWidget(widget, dataIndex);
            
            // Positioning with offset (only at edges)
            int yPos = (i * rowHeight) - shift;
            if (widget->pos().y != yPos || widget->pos().x != m_scrollOffsetX) {
                widget->move(m_scrollOffsetX, yPos);
            }
            
            // Update the width if it has changed
            if (widget->size().width != viewportSize.width) {
                widget->resize(viewportSize.width, rowHeight);
            }
            
            if (!widget->isVisible()) {
                widget->show();
            }
        } else {
            if (widget->isVisible()) {
                widget->hide();
            }
        }
    }
}

void UIListView::updateVisibleItems()
{
    int firstVisible = getFirstVisibleIndex();
    int lastVisible = getLastVisibleIndex();

    m_firstVisibleIndex = firstVisible;
    m_lastVisibleIndex = lastVisible;

    // Create or update widgets
    createVisibleWidgets();
}

void UIListView::updateItemWidget(UIAbstractButton* button, int dataIndex)
{
    if (dataIndex >= 0 && dataIndex < (int)m_items.size()) {
        button->setText(m_items[dataIndex].text);
        
        bool selected = (dataIndex == m_currentIndex);
        if (button->isSelected() != selected) {
            button->setSelected(selected);
        }
    }
}

int UIListView::getFirstVisibleIndex() noexcept
{
    if (m_rowBaseHeight <= 0 || m_dpi_ratio <= 0) return 0;
    
    int firstIndex = -m_scrollOffsetY / (m_rowBaseHeight * m_dpi_ratio);
    
    // Don't use a buffer to snap to the row grid
    return std::max<int>(0, firstIndex);
}

int UIListView::getLastVisibleIndex() noexcept
{
    if (m_itemCount == 0) return -1;
    if (m_rowBaseHeight <= 0 || m_dpi_ratio <= 0) return 0;
    
    UIWidget *view = contentWidget();
    if (!view) return 0;
    
    Size sz = view->size();

    // Calculate the last visible element, rounding up
    int lastIndex = (-m_scrollOffsetY + sz.height + m_rowBaseHeight * m_dpi_ratio - 1) / (m_rowBaseHeight * m_dpi_ratio);
    
    // Add a buffer for smoothness
    lastIndex = std::min<int>(m_itemCount - 1, lastIndex + 1);
    return std::max<int>(0, lastIndex);
}
