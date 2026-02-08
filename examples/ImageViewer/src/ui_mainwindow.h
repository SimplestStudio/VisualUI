#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include "uicaption.h"
#include "uibutton.h"
#include "uitogglebutton.h"
#include "uiradiobutton.h"
#include "uiprogressbar.h"
#include "uilabel.h"
#include "uiboxlayout.h"
#include "uispacer.h"
#include "uimetrics.h"
#include "resource.h"

#define SetMetrics(role, value) metrics()->setMetrics(role, value)
#define SetColor(role, state, value) palette()->setColor(role, state, value)

class Ui_MainWindow
{
public:
    UICaption *caption;
    UIButton *btnMinimize;
    UIButton *btnRestore;
    UIButton *btnClose;
    UIWidget *appPanel;
    UIWidget *leftPanel;
    UIButton *btnOpen;
    UIToggleButton *tbtSwitchTheme;
    UIRadioButton *rbtFit;
    UIRadioButton *rbtActual;
    UIProgressBar *prBar;
    UIButton *btnZoomIn;
    UIButton *btnZoomOut;
    UIButton *btnZoomReset;
    UIWidget *rightPanel;
    UILabel *viewPanel;
    UILabel *bottomPanel;

    void setupUi(UIWidget *w)
    {
        UIBoxLayout *cenVlut = new UIBoxLayout(UIBoxLayout::Vertical);
        cenVlut->setContentMargins(0, 0, 0, 0);
        cenVlut->setSpacing(0);
        w->setLayout(cenVlut);

        /* Top Panel */
        caption = new UICaption(w);
        caption->setObjectGroupId(_T("HeaderBar"));
        cenVlut->addWidget(caption);

        UIBoxLayout *topHlut = new UIBoxLayout(UIBoxLayout::Horizontal);
        topHlut->setContentMargins(0, 0, 0, 0);
        topHlut->setSpacing(0);
        caption->setLayout(topHlut);

        UISpacer *captionStretch = new UISpacer(5,5, SizePolicy::Expanding, SizePolicy::Fixed);
        topHlut->addSpacer(captionStretch);

        btnMinimize = new UIButton(caption);
        btnMinimize->setObjectGroupId(_T("HeaderButton"));
        btnMinimize->setStockIcon(UIButton::MinimizeIcon);
        btnMinimize->setIconSize(10, 10);
        topHlut->addWidget(btnMinimize);

        btnRestore = new UIButton(caption);
        btnRestore->setObjectGroupId(_T("HeaderButton"));
        btnRestore->setStockIcon(UIButton::RestoreIcon);
        btnRestore->setIconSize(10, 10);
        btnRestore->setSupportSnapLayouts();
        topHlut->addWidget(btnRestore);

        btnClose = new UIButton(caption);
        btnClose->setObjectGroupId(_T("HeaderCloseButton"));
        btnClose->setStockIcon(UIButton::CloseIcon);
        btnClose->setIconSize(10, 10);
        topHlut->addWidget(btnClose);

        /* App Panel*/
        appPanel = new UIWidget(w);
        appPanel->setObjectGroupId(_T("CentralWidget"));
        appPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        appPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        cenVlut->addWidget(appPanel);

        UIBoxLayout *appHlut = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHRight | UIBoxLayout::AlignVCenter);
        appHlut->setContentMargins(0, 1, 0, 0);
        appHlut->setSpacing(1);
        appPanel->setLayout(appHlut);

        /* Left Panel */
        leftPanel = new UIWidget(appPanel);
        leftPanel->setObjectGroupId(_T("PanelWidget"));
        leftPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
        leftPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        leftPanel->setBaseSize(210, 50);
        appHlut->addWidget(leftPanel);

        UIBoxLayout *leftVlut = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
        leftVlut->setContentMargins(20, 30, 20, 30);
        leftVlut->setSpacing(12);
        leftPanel->setLayout(leftVlut);

        btnOpen = new UIButton(leftPanel);
        btnOpen->setObjectGroupId(_T("PushButton"));
        btnOpen->SetMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
        btnOpen->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
        btnOpen->SetMetrics(Metrics::IconMarginLeft, 36);
        btnOpen->SetMetrics(Metrics::TextMarginLeft, 58);
        btnOpen->setVectorIcon(IDI_OPENFILE, 16, 16);
        leftVlut->addWidget(btnOpen);

        themeLabel = new UILabel(leftPanel);
        themeLabel->setObjectGroupId(_T("PanelLabel"));
        themeLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        themeLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
        themeLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
        themeLabel->setBaseSize(50, 32);
        leftVlut->addWidget(themeLabel);

        tbtSwitchTheme = new UIToggleButton(leftPanel);
        tbtSwitchTheme->setObjectGroupId(_T("ToggleButton"));
        leftVlut->addWidget(tbtSwitchTheme);

        modeLabel = new UILabel(leftPanel);
        modeLabel->setObjectGroupId(_T("PanelLabel"));
        modeLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        modeLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
        modeLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
        modeLabel->setBaseSize(50, 32);
        leftVlut->addWidget(modeLabel);

        rbtFit = new UIRadioButton(leftPanel);
        rbtFit->setObjectGroupId(_T("Selector"));
        rbtFit->setChecked(true);
        leftVlut->addWidget(rbtFit);

        rbtActual = new UIRadioButton(leftPanel);
        rbtActual->setObjectGroupId(_T("Selector"));
        leftVlut->addWidget(rbtActual);

        zoomLabel = new UILabel(leftPanel);
        zoomLabel->setObjectGroupId(_T("PanelLabel"));
        zoomLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        zoomLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
        zoomLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
        zoomLabel->setBaseSize(50, 32);
        leftVlut->addWidget(zoomLabel);

        UIWidget *buttonsPanel = new UIWidget(leftPanel);
        buttonsPanel->setObjectGroupId(_T("PanelWidget"));
        buttonsPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        buttonsPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
        buttonsPanel->setBaseSize(50, 28);
        leftVlut->addWidget(buttonsPanel);

        UIBoxLayout *bpHlut = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
        bpHlut->setContentMargins(0, 0, 0, 0);
        bpHlut->setSpacing(20);
        buttonsPanel->setLayout(bpHlut);

        btnZoomIn = new UIButton(buttonsPanel);
        btnZoomIn->setObjectGroupId(_T("ToolButton"));
        btnZoomIn->setVectorIcon(IDI_ZOOMIN, 16, 16);
        bpHlut->addWidget(btnZoomIn);

        btnZoomOut = new UIButton(buttonsPanel);
        btnZoomOut->setObjectGroupId(_T("ToolButton"));
        btnZoomOut->setVectorIcon(IDI_ZOOMOUT, 16, 16);
        bpHlut->addWidget(btnZoomOut);

        btnZoomReset = new UIButton(buttonsPanel);
        btnZoomReset->setObjectGroupId(_T("ToolButton"));
        btnZoomReset->setVectorIcon(IDI_ZOOMRESET, 16, 16);
        bpHlut->addWidget(btnZoomReset);

        prBar = new UIProgressBar(leftPanel);
        prBar->setObjectGroupId(_T("ProgressBar"));
        leftVlut->addWidget(prBar);

        UISpacer *spacerPanel = new UISpacer(5, 5, SizePolicy::Fixed, SizePolicy::Expanding);
        leftVlut->addSpacer(spacerPanel);

        /* Right Panel */
        rightPanel = new UIWidget(appPanel);
        rightPanel->setObjectGroupId(_T("CentralWidget"));
        rightPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        rightPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        rightPanel->setBaseSize(50, 50);
        appHlut->addWidget(rightPanel);

        UIBoxLayout *rightVlut = new UIBoxLayout(UIBoxLayout::Vertical);
        rightVlut->setContentMargins(0, 0, 0, 0);
        rightVlut->setSpacing(1);
        rightPanel->setLayout(rightVlut);        

        /* View panel */
        viewPanel = new UILabel(rightPanel);
        viewPanel->setObjectGroupId(_T("ViewWidget"));
        viewPanel->setFont({DEFAULT_FONT_NAME, 14});
        viewPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        viewPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        viewPanel->setBaseSize(50, 50);
        viewPanel->setAcceptDrops(true);
        rightVlut->addWidget(viewPanel);

        /* Bottom panel */
        bottomPanel = new UILabel(rightPanel);
        bottomPanel->setObjectGroupId(_T("ViewWidget"));
        bottomPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        bottomPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
        bottomPanel->setBaseSize(40, 28);
        rightVlut->addWidget(bottomPanel);

        retranslateUi(w);
    }

    void retranslateUi(UIWidget *w)
    {
        caption->setText(_T("Image Viewer"));
        btnOpen->setText(_T("Open Image"));
        themeLabel->setText(_T("Theme"));
        tbtSwitchTheme->setText(_T("Switch theme"));
        modeLabel->setText(_T("Mode"));
        rbtFit->setText(_T("Fit to Screen"));
        rbtActual->setText(_T("Actual Size"));
        zoomLabel->setText(_T("Zoom"));
        btnZoomIn->setToolTip(_T("Zoom In"));
        btnZoomOut->setToolTip(_T("Zoom Out"));
        btnZoomReset->setToolTip(_T("Zoom Reset"));
        viewPanel->setText(_T("Drag & Drop a file"));
    }

private:
    UILabel *themeLabel;
    UILabel *modeLabel;
    UILabel *zoomLabel;
};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
}

#endif // UI_MAINWINDOW_H
