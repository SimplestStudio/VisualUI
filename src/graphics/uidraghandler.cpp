#include "uidraghandler.h"
#include "uiwidget.h"


UIDragHandler::UIDragHandler(UIWidget *target) :
    m_target(target),
    m_dragStart({0,0}),
    m_winStart({0,0}),
    m_dragging(false),
    m_restrictX(false),
    m_restrictY(false)
{
    m_hWindow = target->platformWindow();
#ifdef _WIN32
    target->enableClipSiblings();
#else
    m_parent = gtk_widget_get_parent(m_hWindow);
#endif
}

UIDragHandler::~UIDragHandler()
{

}

void UIDragHandler::handleButtonDownEvent(int x, int y)
{
    m_dragging = true;
    m_dragStart.x = x;
    m_dragStart.y = y;
    m_target->bringAboveSiblings();
    m_target->grabMouse();
#ifdef _WIN32
#else
    int child_x = 0, child_y = 0;
    if (m_parent) {
        gtk_widget_translate_coordinates(m_hWindow, m_parent, 0, 0, &child_x, &child_y);
    }
    m_winStart.x = child_x;
    m_winStart.y = child_y;
#endif
}

void UIDragHandler::handleButtonUpEvent()
{
    if (m_dragging) {
        m_dragging = false;
        m_target->ungrabMouse();
    }
}

void UIDragHandler::handleMouseMoveEvent(int x, int y)
{
    if (m_dragging) {
        int delta_x = x - m_dragStart.x;
        int delta_y = y - m_dragStart.y;

        int new_x = 0;
        int new_y = 0;
#ifdef _WIN32
        RECT rc;
        GetWindowRect(m_hWindow, &rc);

        POINT new_pos = { rc.left, rc.top };
        HWND parent = GetParent(m_hWindow);
        if (parent)
            ScreenToClient(parent, &new_pos);

        if (!m_restrictX) {
            new_x = new_pos.x + delta_x;
        } else {
            new_x = new_pos.x;
        }

        if (!m_restrictY) {
            new_y = new_pos.y + delta_y;
        } else {
            new_y = new_pos.y;
        }

        if (validateMove(new_x, new_y))
            SetWindowPos(m_hWindow, NULL, new_x, new_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE /*| SWP_NOCOPYBITS*/);
#else
        if (!m_restrictX) {
            new_x = m_winStart.x + delta_x;
        } else {
            new_x = m_winStart.x;
        }

        if (!m_restrictY) {
            new_y = m_winStart.y + delta_y;
        } else {
            new_y = m_winStart.y;
        }

        if (validateMove(new_x, new_y)) {
            m_target->move(new_x, new_y);
            m_target->updateGeometry();
        }
#endif
    }
}

void UIDragHandler::restrictMovementX(bool restrict) noexcept
{
    m_restrictX = restrict;
}

void UIDragHandler::restrictMovementY(bool restrict) noexcept
{
    m_restrictY = restrict;
}

void UIDragHandler::onMoveValidation(DragValidationCallback callback)
{
    m_validationCallback = callback;
}

bool UIDragHandler::validateMove(int x, int y) const
{
    if (!m_validationCallback) {
        return true;
    }
    return m_validationCallback(x, y);
}
