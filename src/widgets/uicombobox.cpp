#include "uicombobox.h"
#include "uibutton.h"
#include "uimenu.h"
#include "uimetrics.h"
#include "uiscalaranimation.h"
#ifdef _WIN32
# include <windowsx.h>
#else
#endif

static const int DEFAULT_ITEM_HEIGHT = 32;

UIComboBox::UIComboBox(UIWidget *parent) :
    UILineEdit(parent),
    m_menu(nullptr),
    m_itemHeight(DEFAULT_ITEM_HEIGHT),
    m_currentIndex(-1),
    m_skipNextClick(false),
    m_mousePressed(false)
{
    UIScalarAnimation *rotateAnimation = new UIScalarAnimation;
    rotateAnimation->setDuration(180);
    setIconRotateAnimation(rotateAnimation);
}

UIComboBox::~UIComboBox()
{
    if (m_menu) {
        delete m_menu; m_menu = nullptr;
    }
}

#ifdef _WIN32
bool UIComboBox::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN: {
        m_mousePressed = true;
        break;
    }
    case WM_LBUTTONUP: {
        UILineEdit::event(msg, wParam, lParam, result);
        onClick();
        m_mousePressed = false;
        return true;
    }

    default:
        break;
    }
    return UILineEdit::event(msg, wParam, lParam, result);
}
#else
bool UIComboBox::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS: {
        m_mousePressed = true;
        break;
    }
    case GDK_BUTTON_RELEASE: {
        UILineEdit::event(ev_type, param);
        onClick();
        m_mousePressed = false;
        return true;
    }

    default:
        break;
    }
    return UILineEdit::event(ev_type, param);
}
#endif

void UIComboBox::onClick()
{
    if (m_skipNextClick) {
        m_skipNextClick = false;
        return;
    }

    if (m_menu) {
        return;
    }

    metrics()->setMetrics(Metrics::BorderWidth, 2);

    UIScalarAnimation *ra = iconRotateAnimation();
    ra->stopAnimation();
    double val = ra->currentValue();
    if (val != 180.0) {
        ra->setTargetValue(180.0);
        ra->startAnimation();
    }

    Point pt = mapToGlobal(Point(0, 0));
    Size sz = size();
    int height = calculateMenuHeight();
    m_menu = new UIMenu(topLevelWidget(), Rect(pt.x, pt.y + sz.height + 6 * m_dpi_ratio, sz.width, height));
    m_menu->setObjectGroupId(_T("ToolTip"));
    m_menu->aboutToDestroySignal.connect([this]() {
        m_menu = nullptr;
        m_skipNextClick = underMouse() && m_mousePressed;

        UIScalarAnimation *ra = iconRotateAnimation();
        ra->stopAnimation();
        double val = ra->currentValue();
        if (val != 0.0) {
            ra->setTargetValue(0.0);
            ra->startAnimation();
        }
        metrics()->setMetrics(Metrics::BorderWidth, 1);
    });

    populateMenu();
    m_menu->showAll();
}

void UIComboBox::populateMenu()
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        const tstring &itemText = m_items[i].text;
        UIButton *button = m_menu->addSection(itemText);
        button->setBaseSize(50, m_itemHeight);
        button->clickSignal.connect([this, button, i]() {
            setText(button->text());
            m_menu->close();
            indexChangedSignal.emit(i);
        });
    }
}

int UIComboBox::calculateMenuHeight() const
{
    int itemsHeight = m_items.size() * m_itemHeight * m_dpi_ratio;
    int padding = 12 * m_dpi_ratio;
    return itemsHeight + padding;
}

void UIComboBox::addItem(const tstring &text, uintptr_t data)
{
    m_items.push_back({text, data});
}

void UIComboBox::clearItems()
{
    m_items.clear();
}

void UIComboBox::setCurrentIndex(int index, bool emitSignal) noexcept
{
    if (index > -1 && index < (int)m_items.size()) {
        m_currentIndex = index;
        const tstring &itemText = m_items[index].text;
        setText(itemText);
        if (emitSignal)
            indexChangedSignal.emit(index);
    }
}

void UIComboBox::setItemHeight(int height) noexcept
{
    m_itemHeight = height;
}

uintptr_t UIComboBox::itemData(int index)
{
    if (index > -1 && index < (int)m_items.size()) {
        return m_items[index].data;
    }
    return 0;
}

int UIComboBox::currentIndex() const noexcept
{
    return m_currentIndex;
}

int UIComboBox::count() const noexcept
{
    return m_items.size();
}

bool UIComboBox::containsItem(const tstring &text) const
{
    for (const auto &item : m_items) {
        if (item.text == text) {
            return true;
        }
    }
    return false;
}
