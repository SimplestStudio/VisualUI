#ifndef UIUTILS_H
#define UIUTILS_H

#ifdef _WIN32
# include <Windows.h>
# include <gdiplus.h>
#else
# include <gio/gio.h>
#endif
#include "uidefines.h"


namespace UIUtils
{
#ifdef _WIN32
    enum WinVer : BYTE {
        Undef, WinXP, WinVista, Win7, Win8, Win8_1, Win10, Win11, WinFuture
    };
    WinVer DECL_VISUALUI winVersion() noexcept;
    std::wstring DECL_VISUALUI utf8ToWStr(const std::string &str);
    std::wstring DECL_VISUALUI currentUserSID();
    DWORD DECL_VISUALUI regQueryDwordValue(HKEY rootKey, LPCWSTR subkey, LPCWSTR value);
    double DECL_VISUALUI screenDpiAtPoint(const POINT &pt);
    double DECL_VISUALUI screenDpiAtRect(const RECT &rc);
    void DECL_VISUALUI loadImageResource(Gdiplus::Bitmap* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadEmfResource(Gdiplus::Metafile* &hBmp, int id, LPCWSTR type);
    void DECL_VISUALUI loadStringResource(tstring &str, int id);
    bool DECL_VISUALUI isRtlLanguage(unsigned long lcid);
#else
    enum class DesktopEnv : unsigned char {
        UNDEF, UNITY, GNOME, KDE, XFCE, CINNAMON, OTHER
    };
    enum class WindowServer : unsigned char {
        UNDEF, X11, WAYLAND, OTHER
    };
    DesktopEnv DECL_VISUALUI desktopEnv();
    WindowServer DECL_VISUALUI windowServer();
    void DECL_VISUALUI loadStringResource(tstring &str, GResource *res, const char *resourcePath);
    bool DECL_VISUALUI isRtlLanguage(const char *locale);
#endif
    bool isAllocOnHeap(void *addr);
};

#endif // UIUTILS_H
