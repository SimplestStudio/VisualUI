#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"
#include "uipixmap.h"

class MainWindow : public UIWindow
{
public:
    MainWindow(const Rect &rc, BYTE flags);
    ~MainWindow();

private:
    UIPixmap *m_image;
    double m_scale;
    int m_progress;
};

#endif // MAINWINDOW_H
