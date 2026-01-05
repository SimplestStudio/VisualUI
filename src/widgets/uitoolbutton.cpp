#include "uitoolbutton.h"
#include "uidrawningengine.h"
#include "uipalette.h"

UIToolButton::UIToolButton(UIWidget *parent, const tstring &text) :
    UIAbstractButton(parent, text),
    UIconHandler(this)
{

}

UIToolButton::~UIToolButton()
{

}

#ifdef _WIN32
bool UIToolButton::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN: {
        if (!m_disabled) {
            UIWidget::event(msg, wParam, lParam, result);
            if (m_selectable)
                m_selected = true;
            palette()->setCurrentState(m_selected ? Palette::Active : Palette::Pressed);
            repaint();
            click();
        }
        return true;
    }
    case WM_LBUTTONUP: {
        if (!m_disabled) {
            UIWidget::event(msg, wParam, lParam, result);
            if (!m_selected) {
                palette()->setCurrentState(Palette::Hover);
                repaint();
            }
        }
        return true;
    }
    default:
        break;
    }
    return UIAbstractButton::event(msg, wParam, lParam, result);
}
#else
bool UIToolButton::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS: {
        if (!m_disabled) {
            UIWidget::event(ev_type, param);
            if (m_selectable)
                m_selected = true;
            palette()->setCurrentState(m_selected ? Palette::Active : Palette::Pressed);
            repaint();
            click();
        }
        return true;
    }
    case GDK_BUTTON_RELEASE: {
        if (!m_disabled) {
            UIWidget::event(ev_type, param);
            if (!m_selected) {
                palette()->setCurrentState(Palette::Hover);
                repaint();
            }
        }
        return true;
    }
    default:
        break;
    }
    return UIAbstractButton::event(ev_type, param);
}
#endif

void UIToolButton::onPaint(const RECT &rc)
{
    UIDrawingEngine *de = engine();
#ifdef _WIN32
    if (m_hIcon)
        de->DrawIcon(m_hIcon);
    if (m_hBmp)
        de->DrawImage(m_hBmp);
    if (m_hEmf)
        de->DrawEmfIcon(m_hEmf);
#else
    if (m_hBmp)
        de->DrawIcon(m_hBmp);
    if (m_hSvg)
        de->DrawSvgIcon(m_hSvg);
#endif
    if (!m_text.empty())
        de->DrawString(rc, m_text, m_hFont);
}
