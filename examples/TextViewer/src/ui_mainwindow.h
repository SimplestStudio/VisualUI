#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include "uicaption.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uiboxlayout.h"
#include "uispacer.h"
#include "uilineedit.h"
#include "resource.h"


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
    UIButton *btnSave;
    UIButton *btnMenu;
    UIButton *btnAbout;
    UILineEdit *lineEdit;
    UILabel *viewPanel;

    void setupUi(UIWidget *w)
    {
        UIBoxLayout *cenVlut = new UIBoxLayout(UIBoxLayout::Vertical);
        cenVlut->setContentMargins(0, 0, 0, 0);
        cenVlut->setSpacing(0);
        w->setLayout(cenVlut);

        /* Top Panel */
        caption = new UICaption(w);
        caption->setObjectGroupId(_T("HeaderBar"));
        caption->setVectorIcon(IDI_LOGO, 16, 16);
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
        appHlut->setContentMargins(0, 0, 0, 0);
        appHlut->setSpacing(0);
        appPanel->setLayout(appHlut);

        /* Left Panel */
        leftPanel = new UIWidget(appPanel);
        leftPanel->setObjectGroupId(_T("LeftWidget"));
        leftPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
        leftPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        leftPanel->setBaseSize(40, 50);
        appHlut->addWidget(leftPanel);

        UIBoxLayout *leftVlut = new UIBoxLayout(UIBoxLayout::Vertical);
        leftVlut->setContentMargins(0, 0, 0, 0);
        leftVlut->setSpacing(6);
        leftPanel->setLayout(leftVlut);

        btnOpen = new UIButton(leftPanel);
        btnOpen->setObjectGroupId(_T("ToolButton"));
        btnOpen->setVectorIcon(IDI_OPENFILE, 16, 16);
        leftVlut->addWidget(btnOpen);

        btnSave = new UIButton(leftPanel);
        btnSave->setObjectGroupId(_T("ToolButton"));
        btnSave->setVectorIcon(IDI_SAVEFILE, 16, 16);
        leftVlut->addWidget(btnSave);

        btnMenu = new UIButton(leftPanel);
        btnMenu->setObjectGroupId(_T("ToolButton"));
        btnMenu->setVectorIcon(IDI_MENUEMF, 16, 16);
        leftVlut->addWidget(btnMenu);

        UISpacer *leftStretch = new UISpacer(6, 6, SizePolicy::Fixed, SizePolicy::Expanding);
        leftVlut->addSpacer(leftStretch);

        btnAbout = new UIButton(leftPanel);
        btnAbout->setObjectGroupId(_T("ToolButton"));
        btnAbout->setVectorIcon(IDI_ABOUTEMF, 16, 16);
        leftVlut->addWidget(btnAbout);

        /* Right Panel */
        UIWidget *rightPanel = new UIWidget(appPanel);
        rightPanel->setObjectGroupId(_T("PanelWidget"));
        rightPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
        rightPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        rightPanel->setBaseSize(200, 50);
        appHlut->addWidget(rightPanel);

        UIBoxLayout *rightVlut = new UIBoxLayout(UIBoxLayout::Vertical, UILayout::AlignHCenter | UILayout::AlignVTop);
        rightVlut->setContentMargins(0, 6, 0, 24);
        rightVlut->setSpacing(6);
        rightPanel->setLayout(rightVlut);

        lineEdit = new UILineEdit(rightPanel);
        lineEdit->setObjectGroupId(_T("LineEdit"));
        lineEdit->setVectorIcon(IDI_SEARCHEMF, 14, 14);
        rightVlut->addWidget(lineEdit);

        UILabel *resultPanel = new UILabel(rightPanel);
        resultPanel->setObjectGroupId(_T("PanelWidget"));
        resultPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        resultPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        resultPanel->setVectorIcon(IDI_NOTFOUND, 156, 190);
        rightVlut->addWidget(resultPanel);

        /* View panel */
        viewPanel = new UILabel(appPanel);
        viewPanel->setObjectGroupId(_T("ViewWidget"));
        viewPanel->setFont({DEFAULT_FONT_NAME, 14});
        viewPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
        viewPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
        viewPanel->setBaseSize(50, 50);
        viewPanel->setAcceptDrops(true);
        appHlut->addWidget(viewPanel);

        retranslateUi(w);
    }

    void retranslateUi(UIWidget *w)
    {
        caption->setText(_T("Text Viewer"));
        btnOpen->setToolTip(_T("Open file"));
        btnSave->setToolTip(_T("Save file"));
        btnMenu->setToolTip(_T("Options"));
        btnAbout->setToolTip(_T("About"));
        lineEdit->setPlaceholderText(_T("Search in file..."));
        viewPanel->setText(_T("Drag & Drop a file"));
    }

private:

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
}

#endif // UI_MAINWINDOW_H
