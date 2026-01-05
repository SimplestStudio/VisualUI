#include "uiscrollarea.h"

UIScrollArea::UIScrollArea(UIWidget *parent) :
    UIAbstractScrollArea(parent)
{
    setVerticalContentResizeMode(ResizeToContent);
    setHorizontalContentResizeMode(ResizeToContent);
}

UIScrollArea::~UIScrollArea()
{

}

void UIScrollArea::onScrollOffsetChanged()
{
    if (m_contentWidget) {
        m_contentWidget->move(m_scrollOffsetX, m_scrollOffsetY);
    }
}
