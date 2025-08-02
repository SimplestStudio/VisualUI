#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"


class MainWindow : public UIWindow
{
public:
    MainWindow(const Rect &rc, BYTE flags);
    ~MainWindow();

private:
    bool m_toggleChecked;
};

#endif // MAINWINDOW_H
