#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef _WIN32
#define IDI_MAINICON      101
#define IDI_OPENFILE      102
#define IDI_ZOOMIN        104
#define IDI_ZOOMOUT       108
#define IDI_ZOOMRESET     111
#define IDT_THEMES        1001
#define IDT_STYLES        1002
#else
#define IDI_MAINICON      "/icons/app-icon.png"
#define IDI_OPENFILE      "/icons/open-file.svg"
#define IDI_ZOOMIN        "/icons/zoom-in.svg"
#define IDI_ZOOMOUT       "/icons/zoom-out.svg"
#define IDI_ZOOMRESET     "/icons/zoom-reset.svg"
#define IDT_THEMES        "/styles/themes.xml"
#define IDT_STYLES        "/styles/styles.xml"
#endif

#endif // RESOURCE_H
