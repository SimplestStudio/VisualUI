#ifndef UILISTVIEW_H
#define UILISTVIEW_H

#include "uiabstractscrollarea.h"
#include <vector>

class UIToolButton;
class DECL_VISUALUI UIListView : public UIAbstractScrollArea
{
public:
    enum ScrollMode { ScrollPerPixel, ScrollPerItem };

    explicit UIListView(UIWidget *parent = nullptr);
    ~UIListView();

    void addItem(const tstring &text, uintptr_t data = 0);
    void removeItem(int index);
    void clearList();
    void setCurrentIndex(int index);
    void setRowBaseHeight(int height) noexcept;
    void setScrollMode(ScrollMode mode) noexcept;
    uintptr_t itemData(int index) const;
    int currentIndex() const noexcept;
    int rowBaseHeight() const noexcept;
    ScrollMode scrollMode() const noexcept;
    int count() const noexcept;

    /* Signals */
    Signal<int> indexChangedSignal;

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onScrollOffsetChanged() override;

private:
    void createVisibleWidgets();
    void updateVisibleItems();
    void updateItemWidget(UIToolButton* button, int dataIndex);
    int getFirstVisibleIndex() noexcept;
    int getLastVisibleIndex() noexcept;
    
    struct Item {
        tstring text;
        uintptr_t data;
    };

    std::vector<UIToolButton*> m_visibleWidgets;
    std::vector<Item> m_items;
    int m_firstVisibleIndex;
    int m_lastVisibleIndex;
    int m_rowBaseHeight;
    int m_currentIndex;
    int m_itemCount;
    ScrollMode m_scrollMode;
};

#endif // UILISTVIEW_H
