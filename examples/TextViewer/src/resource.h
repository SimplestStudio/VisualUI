#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef _WIN32
#define IDI_MAINICON      101
#define IDI_LOGO          102
#define IDI_OPENFILE      103
#define IDI_MENUEMF       104
#define IDI_SAVEFILE      105
#define IDI_ABOUTEMF      106
#define IDI_SEARCHEMF     107
#define IDI_NOTFOUND      108
#define IDI_LOGODARK      109
#define IDI_OPENFILEDARK  110
#define IDI_MENUDARKEMF   111
#define IDI_SAVEFILEDARK  112
#define IDI_ABOUTDARKEMF  113
#define IDI_SEARCHDARKEMF 114
#define IDI_TOOLS         115
#define IDT_THEMES        1001
#define IDT_STYLES        1002
#else
#define IDI_MAINICON      "/icons/app-icon.png"
#define IDI_LOGO          "/icons/logo.svg"
#define IDI_OPENFILE      "/icons/open-file.svg"
#define IDI_MENUEMF       "/icons/menu.svg"
#define IDI_SAVEFILE      "/icons/save-file.svg"
#define IDI_ABOUTEMF      "/icons/about.svg"
#define IDI_SEARCHEMF     "/icons/search.svg"
#define IDI_NOTFOUND      "/icons/not-found.svg"
#define IDI_LOGODARK      "/icons/logo_dark.svg"
#define IDI_OPENFILEDARK  "/icons/open-file_dark.svg"
#define IDI_MENUDARKEMF   "/icons/menu_dark.svg"
#define IDI_SAVEFILEDARK  "/icons/save-file_dark.svg"
#define IDI_ABOUTDARKEMF  "/icons/about_dark.svg"
#define IDI_SEARCHDARKEMF "/icons/search_dark.svg"
#define IDI_TOOLS         "/icons/tools.png"
#define IDT_THEMES        "/styles/themes.xml"
#define IDT_STYLES        "/styles/styles.xml"
#endif

#endif // RESOURCE_H
