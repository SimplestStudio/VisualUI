#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uiwindow.h"
#include "uipixmap.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public UIWindow
{
public:
    MainWindow(const Rect &rc, BYTE flags);
    ~MainWindow();

private:
    void calculateFitSize(const Size &imgSize, int w, int h, double &drawW, double &drawH) const noexcept;

    Ui::MainWindow *ui;
    UIPixmap *m_image;
    double m_scale;
    int m_progress;
};

#endif // MAINWINDOW_H
