#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public UIWindow
{
public:
    MainWindow(const Rect &rc, BYTE flags);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool m_toggleChecked;
};

#endif // MAINWINDOW_H
