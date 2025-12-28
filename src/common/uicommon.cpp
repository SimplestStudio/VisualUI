#include "uicommon.h"


Margins::Margins() noexcept :
    left(0), top(0), right(0), bottom(0)
{}

Margins::Margins(int l, int t, int r, int b) noexcept :
    left(l), top(t), right(r), bottom(b)
{}


Rect::Rect() noexcept :
    x(0), y(0), width(0), height(0)
{}

Rect::Rect(int x, int y, int w, int h) noexcept :
    x(x), y(y), width(w), height(h)
{}


Point::Point() noexcept :
    x(0), y(0)
{}

Point::Point(int x, int y) noexcept :
    x(x), y(y)
{}


Size::Size() noexcept :
    width(0), height(0)
{}

Size::Size(int w, int h) noexcept :
    width(w), height(h)
{}
