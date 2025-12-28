#include "uiabstractwindow.h"
#ifdef __linux__
# include "uiapplication.h"
#endif

#ifdef _WIN32
static BOOL CALLBACK ShowChildrenProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
    return TRUE;
}

static BOOL CALLBACK NcActivationNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    SendMessage(hwnd, WM_PARENT_ACTIVATION_NOTIFY, params->wParam, 0);
    return TRUE;
}

static BOOL CALLBACK ToplevelNcActivationNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_ACTIVATION_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}

static BOOL CALLBACK ToplevelButtondownNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_BUTTONDOWN_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}

static BOOL CALLBACK ToplevelKeydownNotifyProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    NotifyParams* params = (NotifyParams*)lParam;
    if (hwnd != params->senderHwnd && GetWindow(hwnd, GW_OWNER) == params->senderHwnd) {
        SendMessage(hwnd, WM_TOPLEVEL_KEYDONW_NOTIFY, params->wParam, 0);
    }
    return TRUE;
}
#else
static void EnumChildLayouts(GtkWidget *widget, gpointer data)
{
    GList **result_list = (GList**)data;
    if (GTK_IS_LAYOUT(widget)) {
        *result_list = g_list_append(*result_list, widget);
    }
    if (GTK_IS_CONTAINER(widget)) {
        gtk_container_foreach(GTK_CONTAINER(widget), EnumChildLayouts, data);
    }
}

static void ParentNotifyProc(GtkWidget *sender, uint event_type, void *param)
{
    GList *children = NULL;
    gtk_container_foreach(GTK_CONTAINER(sender), (GtkCallback)EnumChildLayouts, (gpointer)&children);
    GList *it;
    for (it = children; it != NULL; it = it->next) {
        GtkWidget *child = GTK_WIDGET(it->data);
        UIApplication::sendEvent(child, event_type, param);
    }
    g_list_free(children);
}

static void ToplevelNotifyProc(GtkWidget *sender, uint event_type, void *param)
{
    GList *windows = gtk_window_list_toplevels();
    GList *it;
    for (it = windows; it != NULL; it = it->next) {
        GtkWindow *window = GTK_WINDOW(it->data);
        if (GTK_WIDGET(window) != sender && GTK_WIDGET(gtk_window_get_transient_for(window)) == sender) {
            UIApplication::sendEvent(GTK_WIDGET(window), event_type, param);
        }
    }
    g_list_free(windows);
}
#endif

UIAbstractWindow::UIAbstractWindow(UIWidget *parent, ObjectType type, const Rect &rc) :
    UIWidget(parent, type, nullptr, rc)
{
#ifdef __linux
    gtk_window_set_position(GTK_WINDOW(m_hWindow), GtkWindowPosition::GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(m_hWindow, TRUE);
    GdkScreen *scr = gtk_widget_get_screen(m_hWindow);
    if (GdkVisual *vis = gdk_screen_get_rgba_visual(scr))
        gtk_widget_set_visual(m_hWindow, vis);

    m_gtk_layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(m_hWindow), m_gtk_layout);
#endif
}

UIAbstractWindow::~UIAbstractWindow()
{

}

void UIAbstractWindow::setWindowTitle(const tstring &title)
{
    m_title = title;
#ifdef _WIN32
    SetWindowText(m_hWindow, title.c_str());
#else
    gtk_window_set_title(GTK_WINDOW(m_hWindow), title.c_str());
#endif
}

#ifdef __linux__
void UIAbstractWindow::setGeometry(int x, int y, int w, int h)
{
    gtk_window_resize(GTK_WINDOW(m_hWindow), w, h);
    gtk_window_move(GTK_WINDOW(m_hWindow), x, y);
}

void UIAbstractWindow::move(int x, int y)
{
    gtk_window_move(GTK_WINDOW(m_hWindow), x, y);
}

void UIAbstractWindow::resize(int w, int h)
{
    gtk_window_resize(GTK_WINDOW(m_hWindow), w, h);
}
#endif

Size UIAbstractWindow::size() const noexcept
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    return Size(rc.right - rc.left, rc.bottom - rc.top);
#else
    int w = 0, h = 0;
    gtk_window_get_size(GTK_WINDOW(m_hWindow), &w, &h);
    return Size(w, h);
#endif
}

void UIAbstractWindow::size(int *width, int *height) const noexcept
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    *width = rc.right - rc.left;
    *height =  rc.bottom - rc.top;
#else
    gtk_window_get_size(GTK_WINDOW(m_hWindow), width, height);
#endif
}

Point UIAbstractWindow::pos() const noexcept
{
#ifdef _WIN32
    RECT rc;
    GetWindowRect(m_hWindow, &rc);
    return Point(rc.left, rc.top);
#else
    int x, y;
    gtk_window_get_position(GTK_WINDOW(m_hWindow), &x, &y);
    return Point(x, y);
#endif
}

void UIAbstractWindow::showAll()
{
#ifdef _WIN32
    ShowWindow(m_hWindow, SW_SHOWNORMAL);
    UpdateWindow(m_hWindow);
    EnumChildWindows(m_hWindow, ShowChildrenProc, 0);
    SetForegroundWindow(m_hWindow);
    SetFocus(m_hWindow);
#else
    gtk_widget_show_all(m_hWindow);
#endif
}

#ifdef _WIN32
bool UIAbstractWindow::event(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_CHILD_BUTTONDOWN_NOTIFY: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumThreadWindows(GetCurrentThreadId(), ToplevelButtondownNotifyProc, (LPARAM)&params);
        break;
    }

    case WM_KEYDOWN:
    case WM_CHILD_KEYDOWN_NOTIFY: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumThreadWindows(GetCurrentThreadId(), ToplevelKeydownNotifyProc, (LPARAM)&params);
        break;
    }

    case WM_NCACTIVATE: {
        NotifyParams params;
        params.senderHwnd = m_hWindow;
        params.wParam = wParam;
        EnumChildWindows(m_hWindow, NcActivationNotifyProc, (LPARAM)&params);
        EnumThreadWindows(GetCurrentThreadId(), ToplevelNcActivationNotifyProc, (LPARAM)&params);
        break;
    }

    default:
        break;
    }
    return UIWidget::event(msg, wParam, lParam, result);
}
#else
bool UIAbstractWindow::event(uint ev_type, void *param)
{
    switch (ev_type) {
    case GDK_BUTTON_PRESS:
    case GDK_CHILD_BUTTONDOWN_NOTIFY: {
        GdkEventButton *bev = (GdkEventButton*)param;
        ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_BUTTONDOWN_NOTIFY, (void*)&bev->button);
        return false;
    }

    case GDK_KEY_PRESS: {
        ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_KEYDONW_NOTIFY, (void*)param);
        break;
    }

    case GDK_HOOKED_WINDOW_STATE_AFTER: {
        GdkEvent *ev = (GdkEvent*)param;
        if (ev->window_state.changed_mask & GDK_WINDOW_STATE_FOCUSED) {
            bool focused = ev->window_state.new_window_state & GDK_WINDOW_STATE_FOCUSED;
            if (m_gtk_layout) {
                ParentNotifyProc(m_gtk_layout, GDK_PARENT_ACTIVATION_NOTIFY, (void*)&focused);
            }
            ToplevelNotifyProc(m_hWindow, GDK_TOPLEVEL_ACTIVATION_NOTIFY, (void*)&focused);
        }
        return false;
    }

    default:
        break;
    }
    return UIWidget::event(ev_type, param);
}

GtkWidget* UIAbstractWindow::gtkLayout()
{
    return m_gtk_layout;
}
#endif
