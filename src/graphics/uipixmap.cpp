#include "uipixmap.h"


UIPixmap::UIPixmap() :
    m_hBmp(nullptr)
{

}

UIPixmap::UIPixmap(const UIPixmap &other) :
    UIPixmap()
{
#ifdef _WIN32
    Gdiplus::Bitmap *hBmp = other.m_hBmp;
    if (hBmp)
        m_hBmp = hBmp->Clone(0, 0, hBmp->GetWidth(), hBmp->GetHeight(), hBmp->GetPixelFormat());
#else
    if (other.m_hBmp)
        m_hBmp = gdk_pixbuf_copy(other.m_hBmp);
#endif
}

UIPixmap::UIPixmap(UIPixmap&& other) noexcept
{
    m_hBmp = other.m_hBmp;
    other.m_hBmp = nullptr;
}

UIPixmap::UIPixmap(const tstring &path) :
    UIPixmap()
{
#ifdef _WIN32
    m_hBmp = new Gdiplus::Bitmap(path.c_str());
#else
    m_hBmp = gdk_pixbuf_new_from_file(path.c_str(), NULL);
#endif
}

UIPixmap::~UIPixmap()
{
    reset();
}

UIPixmap& UIPixmap::operator=(const UIPixmap &other)
{
    if (this != &other) {
        reset();
#ifdef _WIN32
        Gdiplus::Bitmap *hBmp = other.m_hBmp;
        if (hBmp)
            m_hBmp = hBmp->Clone(0, 0, hBmp->GetWidth(), hBmp->GetHeight(), hBmp->GetPixelFormat());
#else
        if (other.m_hBmp)
            m_hBmp = gdk_pixbuf_copy(other.m_hBmp);
#endif
    }
    return *this;
}

UIPixmap& UIPixmap::operator=(UIPixmap&& other) noexcept
{
    if (this != &other) {
        reset();
        m_hBmp = other.m_hBmp;
        other.m_hBmp = nullptr;
    }
    return *this;
}

void UIPixmap::reset()
{
    if (m_hBmp) {
#ifdef _WIN32
        delete m_hBmp;
#else
        g_object_unref(m_hBmp);
#endif
        m_hBmp = nullptr;
    }
}

bool UIPixmap::isValid() const noexcept
{
    return (m_hBmp != nullptr);
}

Size UIPixmap::imageSize() const noexcept
{
    int w = -1, h = -1;
#ifdef _WIN32
    if (m_hBmp) {
        w = m_hBmp->GetWidth();
        h = m_hBmp->GetHeight();
    }
#else
    if (m_hBmp) {
        w = gdk_pixbuf_get_width(m_hBmp);
        h = gdk_pixbuf_get_height(m_hBmp);
    }
#endif
    return Size(w, h);
}

UIPixmap UIPixmap::scaled(int width, int height) const
{
    UIPixmap result;
    if (!m_hBmp || width <= 0 || height <= 0)
        return result;
#ifdef _WIN32
    Gdiplus::Bitmap *scaled = new Gdiplus::Bitmap(width, height, m_hBmp->GetPixelFormat());
    if (scaled) {
        Gdiplus::Graphics gr(scaled);
        gr.SetInterpolationMode(Gdiplus::InterpolationModeLowQuality);
        gr.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
        gr.DrawImage(m_hBmp, 0, 0, width, height);
        result.m_hBmp = scaled;
    }
#else
    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(m_hBmp, width, height, GDK_INTERP_BILINEAR);
    if (scaled)
        result.m_hBmp = scaled;
#endif
    return result;
}
