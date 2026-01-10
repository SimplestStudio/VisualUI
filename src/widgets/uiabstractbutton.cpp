#include "uiabstractbutton.h"
#include "uifontmetrics.h"
#include "uitooltiphandler.h"
#include "uipalette.h"
#include "uimetrics.h"
#include "uistyle.h"
#ifdef _WIN32
# include <windowsx.h>
#else
# include "uiapplication.h"
#endif


UIAbstractButton::UIAbstractButton(UIWidget *parent, const tstring &text) :
    UIWidget(parent, ObjectType::WidgetType),
    m_text(text),
    m_tooltipHandler(nullptr),
    m_checked(false),
    m_selected(false),
    m_selectable(false),
    m_restrictedClickArea(false)
{
#ifdef _WIN32
    DWORD dwStyle = ::GetClassLong(m_hWindow, GCL_STYLE);
    dwStyle &= ~CS_DBLCLKS;
    ::SetClassLong(m_hWindow, GCL_STYLE, dwStyle);
#endif
}

UIAbstractButton::~UIAbstractButton()
{
    if (m_tooltipHandler) {
        delete m_tooltipHandler; m_tooltipHandler = nullptr;
    }
}

void UIAbstractButton::setText(const tstring &text)
{
    m_text = text;
    update();
}

void UIAbstractButton::setChecked(bool checked)
{
    m_checked = checked;
    update();
}

void UIAbstractButton::setSelected(bool enabled) noexcept
{
    if (!m_selectable) return;

    m_selected = enabled;
    if (!m_disabled) {
        palette()->setCurrentState(m_selected ? Palette::Active : Palette::Normal);
        update();
    }
}

void UIAbstractButton::setSelectable(bool enabled) noexcept
{
    m_selectable = enabled;
}

void UIAbstractButton::setToolTip(const tstring &text) noexcept
{
    if (!m_tooltipHandler)
        m_tooltipHandler = new UIToolTipHandler(this);
    m_tooltipHandler->setToolTipText(text);
}

tstring UIAbstractButton::text() const noexcept
{
    return m_text;
}

bool UIAbstractButton::isSelected() const noexcept
{
    return m_selected;
}

bool UIAbstractButton::isChecked() const noexcept
{
    return m_checked;
}

void UIAbstractButton::restrictClickArea(bool restrict) noexcept
{
    m_restrictedClickArea = restrict;
}

void UIAbstractButton::adjustSizeBasedOnContent()
{
    int width = 0, height = 0;
    const Metrics *mtr = metrics();
    UIFontMetrics fm(this);
    fm.textSize(m_text, width, height);
    width /= m_dpi_ratio;
    height /= m_dpi_ratio;
    int w = width + mtr->value(Metrics::IconWidth) + mtr->value(Metrics::TextMarginLeft) + mtr->value(Metrics::TextMarginRight);
#ifdef _WIN32
    int h = max(height + mtr->value(Metrics::TextMarginTop) + mtr->value(Metrics::TextMarginBottom), mtr->value(Metrics::IconHeight));
#else
    int h = std::max(height + mtr->value(Metrics::TextMarginTop) + mtr->value(Metrics::TextMarginBottom), mtr->value(Metrics::IconHeight));
#endif
    setBaseSize(w, h);
}

#ifdef _WIN32
bool UIAbstractButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_NCHITTEST: {
        if (!m_restrictedClickArea)
            break;
        *result = checkInputRegion(lParam, m_check_rc);
        return true;
    }

    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (!m_disabled) {
            if (m_selectable)
                m_selected = true;
            palette()->setCurrentState(m_selected ? Palette::Active : Palette::Hover);
            repaint();
            click();
        }
        break;
    }

    case WM_MOUSEENTER: {
        if (!m_disabled) {
            if (!m_selected) {
                palette()->setCurrentState(Palette::Hover);
                repaint();
            }
        }
        break;
    }

    case WM_MOUSELEAVE: {
        if (!m_disabled) {
            if (m_tooltipHandler)
                m_tooltipHandler->skipToolTip();
            if (!m_selected) {
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case WM_MOUSEMOVE: {
        if (!m_disabled) {
            if (m_tooltipHandler)
                m_tooltipHandler->handleMouseMove();
        }
        break;
    }

    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIAbstractButton::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS: {
        UIWidget::event(ev_type, param);
        if (!m_disabled) {
            palette()->setCurrentState(Palette::Pressed);
            repaint();
        }
        return true;
    }

    case GDK_BUTTON_RELEASE: {
        UIWidget::event(ev_type, param);
        if (!m_disabled) {
            if (m_selectable)
                m_selected = true;
            palette()->setCurrentState(m_selected ? Palette::Active : Palette::Hover);
            repaint();
            click();
        }
        return true;
    }

    case GDK_ENTER_NOTIFY: {
        if (!m_disabled) {
            if (!m_selected) {
                palette()->setCurrentState(Palette::Hover);
                repaint();
            }
        }
        break;
    }

    case GDK_LEAVE_NOTIFY: {
        if (!m_disabled) {
            if (m_tooltipHandler)
                m_tooltipHandler->skipToolTip();
            if (!m_selected) {
                palette()->setCurrentState(Palette::Normal);
                repaint();
            }
        }
        break;
    }

    case GDK_MOTION_NOTIFY: {
        if (!m_disabled) {
            if (m_tooltipHandler)
                m_tooltipHandler->handleMouseMove();
        }
        break;
    }

    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}
#endif

void UIAbstractButton::click()
{
    if (underMouse()) {
        if (m_tooltipHandler)
            m_tooltipHandler->skipToolTip();
        clickSignal.emit();
    }
}
