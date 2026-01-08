#include "uipixmap.h"
#include <vector>


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

UIPixmap UIPixmap::fromRawData(const uint8_t *data, int width, int height, bool hasAlpha)
{
    if (!data || width <= 0 || height <= 0)
        return UIPixmap{};

#ifdef _WIN32
    size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

    std::vector<Gdiplus::ARGB> pixels(pixelCount);

    for (size_t i = 0; i < pixelCount; ++i) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];
        uint8_t a = hasAlpha ? data[i * 4 + 3] : 255;

        // Premultiply alpha for PARGB format
        if (hasAlpha) {
            r = static_cast<uint8_t>((r * a + 127) / 255);
            g = static_cast<uint8_t>((g * a + 127) / 255);
            b = static_cast<uint8_t>((b * a + 127) / 255);
        }

        pixels[i] = (static_cast<Gdiplus::ARGB>(a) << 24) |
                    (static_cast<Gdiplus::ARGB>(r) << 16) |
                    (static_cast<Gdiplus::ARGB>(g) << 8)  |
                    (static_cast<Gdiplus::ARGB>(b));
    }

    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppPARGB);
    if (bitmap->GetLastStatus() != Gdiplus::Ok) {
        delete bitmap;
        return UIPixmap{};
    }

    Gdiplus::Rect rect(0, 0, width, height);
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Status status = bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppPARGB, &bitmapData);
    if (status != Gdiplus::Ok) {
        delete bitmap;
        return UIPixmap{};
    }

    BYTE* dst = static_cast<BYTE*>(bitmapData.Scan0);
    const BYTE* src = reinterpret_cast<const BYTE*>(pixels.data());
    int srcStride = width * 4;
    int dstStride = bitmapData.Stride;

    for (int y = 0; y < height; ++y)
        memcpy(dst + y * dstStride, src + y * srcStride, srcStride);

    bitmap->UnlockBits(&bitmapData);
#else
    GdkPixbuf *bitmap = gdk_pixbuf_new(GDK_COLORSPACE_RGB, hasAlpha ? TRUE : FALSE, 8, width, height);
    if (!bitmap) return UIPixmap{};

    guchar *pixels = gdk_pixbuf_get_pixels(bitmap);
    int rowstride = gdk_pixbuf_get_rowstride(bitmap);
    int nChannels = gdk_pixbuf_get_n_channels(bitmap);

    // Copy pixel data from source (always RGBA, 4 bytes per pixel)
    for (int y = 0; y < height; ++y) {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; ++x) {
            int srcIdx = (y * width + x) * 4;
            row[x * nChannels + 0] = data[srcIdx + 0];  // R
            row[x * nChannels + 1] = data[srcIdx + 1];  // G
            row[x * nChannels + 2] = data[srcIdx + 2];  // B
            if (hasAlpha)
                row[x * nChannels + 3] = data[srcIdx + 3];  // A
        }
    }
#endif
    UIPixmap result;
    result.m_hBmp = bitmap;
    return result;
}
