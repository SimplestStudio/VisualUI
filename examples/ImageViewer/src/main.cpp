#include "uiapplication.h"
#include "mainwindow.h"
#include "uiutils.h"
#include "uistyle.h"
#include "resource.h"
#ifdef __linux__
# include "../res/gresource.c"
#endif


#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR cmdline, int cmdshow)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _WIN32
    UIApplication app(hInst, cmdline, cmdshow);
    app.style()->loadThemesFromResource(IDT_THEMES);
    app.style()->loadStylesFromResource(IDT_STYLES);
#else
    UIApplication app(argc, argv);
    if (GResource *res = gresource_get_resource()) {
        g_resources_register(res);
        app.style()->loadThemesFromResource(res, IDT_THEMES);
        app.style()->loadStylesFromResource(res, IDT_STYLES);
        g_resource_unref(res);
    }
#endif
    app.setFont(_T("Arial"), 9.5);
    app.style()->setDefaultTheme(_T("Dark"));

    double screenDpi = 1.0;
    Rect rc(100, 100, 900, 600);
#ifdef _WIN32
    RECT _rc{rc.x, rc.y, rc.x + rc.width, rc.y + rc.height};
    screenDpi = UIScreen::dpiAtRect(_rc);
    rc = Rect(rc.x * screenDpi, rc.y * screenDpi, rc.width * screenDpi, rc.height * screenDpi);
#endif

    MainWindow w(rc, UIWindow::RemoveSystemDecoration);
    w.setWindowTitle(_T("Image Viewer"));
    w.setIcon(IDI_MAINICON);
    w.onAboutToDestroy([&app]() {
        app.exit(0);
    });
    w.showAll();
    return app.exec();
}
