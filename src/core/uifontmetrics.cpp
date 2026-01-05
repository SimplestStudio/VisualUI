#include "uifontmetrics.h"
#include "uiwidget.h"
#ifdef _WIN32
# include <gdiplus.h>
#endif

UIFontMetrics::UIFontMetrics(UIWidget *widget) :
    m_widget(widget)
{

}

UIFontMetrics::~UIFontMetrics()
{

}

void UIFontMetrics::textSize(const tstring &text, int &width, int &height) const
{
    PlatformWindow hWindow = m_widget->platformWindow();
#ifdef _WIN32
    HDC hdc = GetDC(hWindow);
    Gdiplus::Graphics gr(hdc);
    LOGFONTW logFont = {0};
    GetObject(m_widget->m_hFont, sizeof(LOGFONTW), &logFont);
    Gdiplus::Font font(hdc, &logFont);

    Gdiplus::StringFormat strFmt(Gdiplus::StringFormat::GenericTypographic());
    strFmt.SetFormatFlags(strFmt.GetFormatFlags() | Gdiplus::StringFormatFlagsMeasureTrailingSpaces);

    Gdiplus::RectF boxRc;
    gr.MeasureString(text.c_str(), text.length(), &font, Gdiplus::PointF(0,0), &strFmt, &boxRc);
    ReleaseDC(hWindow, hdc);

    width = boxRc.Width;
    height = boxRc.Height;
#else
    PangoLayout *lut = gtk_widget_create_pango_layout(hWindow, text.c_str());
    pango_layout_set_font_description(lut, m_widget->m_hFont->desc);
    pango_layout_get_size(lut, &width, &height);
    width /= PANGO_SCALE;
    height /= PANGO_SCALE;
    g_object_unref(lut);
#endif
}
