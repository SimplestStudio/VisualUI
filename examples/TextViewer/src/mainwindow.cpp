#include "mainwindow.h"
#include "uiapplication.h"
#include "uipopupmessage.h"
#include "uifiledialog.h"
#include "uidialog.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uimenu.h"
#include "uicheckbox.h"
#include "uitogglebutton.h"
#include "uilineedit.h"
#include "uimetrics.h"
#include "uipalette.h"
#include "uiboxlayout.h"
#include "uispacer.h"
#include "uicaption.h"
#include "resource.h"
#include "uistyle.h"
#include <fstream>
#ifdef _WIN32
# define tifstream std::wifstream
#else
# include "uidrawningengine.h"
# define tifstream std::ifstream
#endif

#define SetMetrics(role, value) metrics()->setMetrics(role, value)
#define SetColor(role, state, value) palette()->setColor(role, state, value)


static bool readFile(const tstring &filePath, tstring &text)
{
    tifstream file(filePath, std::ios_base::in);
    if (!file.is_open())
        return false;

    tstring line;
    while (std::getline(file, line)) {
        text.append(line);
        text.push_back(_T('\n'));
    }

    if (!file.eof() && file.fail()) {
        file.close();
        text.clear();
        return false;
    }
    file.close();
    return true;
}

MainWindow::MainWindow(const Rect &rc, BYTE flags) :
    UIWindow(nullptr, rc, flags),
    m_toggleChecked(false)
{
    setObjectGroupId(_T("MainWindow"));

    UIWidget *cw = new UIWidget(this);
    cw->setObjectGroupId(_T("CentralWidget"));
    setCentralWidget(cw);
    setContentsMargins(0,0,0,0);

    UIBoxLayout *chlut = new UIBoxLayout(UIBoxLayout::Vertical);
    chlut->setContentMargins(0, 0, 0, 0);
    chlut->setSpacing(0);
    cw->setLayout(chlut);

    /* Top panel */
    UICaption *caption = new UICaption(cw);
    caption->setObjectGroupId(_T("HeaderBar"));
    caption->setVectorIcon(IDI_LOGO, 16, 16);
    caption->setText(_T("Text Viewer"));

    UIBoxLayout *thlut = new UIBoxLayout(UIBoxLayout::Horizontal);
    thlut->setContentMargins(0, 0, 0, 0);
    thlut->setSpacing(0);
    caption->setLayout(thlut);

    UISpacer *captionStretch = new UISpacer(5,5, SizePolicy::Expanding, SizePolicy::Fixed);
    thlut->addSpacer(captionStretch);

    UIButton *btn1 = new UIButton(caption);
    btn1->setObjectGroupId(_T("HeaderCloseButton"));
    btn1->setStockIcon(UIButton::CloseIcon);
    btn1->setIconSize(10, 10);
    btn1->clickSignal.connect([=]() {
        printf("onClose... 1\n");
        fflush(stdout);
        close();
    });

    UIButton *btn2 = new UIButton(caption);
    btn2->setObjectGroupId(_T("HeaderButton"));
    btn2->setStockIcon(UIButton::RestoreIcon);
    btn2->setIconSize(10, 10);
    btn2->setSupportSnapLayouts();
    btn2->clickSignal.connect([=]() {
        printf("onMaximized... 1\n");
        fflush(stdout);
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });

    UIButton *btn3 = new UIButton(caption);
    btn3->setObjectGroupId(_T("HeaderButton"));
    btn3->setStockIcon(UIButton::MinimizeIcon);
    btn3->setIconSize(10, 10);

    btn3->clickSignal.connect([=]() {
        printf("onMinimized... 1\n");
        fflush(stdout);
        showMinimized();
    });
    btn1->activationChangedSignal.connect([btn1, btn2, btn3](bool active) {
        UIStyle *style = UIApplication::instance()->style();        
        btn1->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn2->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn3->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn1->update();
        btn2->update();
        btn3->update();
    });

    thlut->addWidget(btn3);
    thlut->addWidget(btn2);
    thlut->addWidget(btn1);
    chlut->addWidget(caption);

    /* App panel */
    UIWidget *cenPanel = new UIWidget(cw);
    cenPanel->setObjectGroupId(_T("CentralWidget"));
    cenPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    cenPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    cenPanel->setBaseSize(50,28);
    chlut->addWidget(cenPanel);

    UIBoxLayout *cvlut = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHRight | UIBoxLayout::AlignVCenter);
    cvlut->setContentMargins(0, 0, 0, 0);
    cvlut->setSpacing(0);
    cenPanel->setLayout(cvlut);

    /* Left Panel*/
    UIWidget *leftPanel = new UIWidget(cenPanel);
    leftPanel->setObjectGroupId(_T("LeftWidget"));
    leftPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
    leftPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    leftPanel->setBaseSize(40, 50);
    cvlut->addWidget(leftPanel);

    UIBoxLayout *lvlut = new UIBoxLayout(UIBoxLayout::Vertical);
    lvlut->setContentMargins(0, 0, 0, 0);
    lvlut->setSpacing(6);
    leftPanel->setLayout(lvlut);

    UIButton *btn4 = new UIButton(leftPanel);
    btn4->setObjectGroupId(_T("ToolButton"));
    btn4->setVectorIcon(IDI_OPENFILE, 16, 16);
    btn4->setToolTip(_T("Open file"));
    lvlut->addWidget(btn4);

    UIButton *btn5 = new UIButton(leftPanel);
    btn5->setObjectGroupId(_T("ToolButton"));
    btn5->setVectorIcon(IDI_SAVEFILE, 16, 16);
    btn5->setToolTip(_T("Save file"));
    btn5->clickSignal.connect([=]() {
        Point wndPos = pos();
        Size wndSize = size();
        int w = 336 * dpiRatio();
        int h = 189 * dpiRatio();
        int x = wndPos.x + (wndSize.width - w) / 2;
        int y = wndPos.y + (wndSize.height - h) / 2;
        UIPopupMessage *msg = new UIPopupMessage(this, Rect(x, y, w, h));
        msg->setObjectGroupId(_T("PopupMsg"));
        msg->setCaptionText(_T("Save changes"));
        msg->setDescriptionText(_T("Do you want to save changes?"));
        msg->addButton(_T("Save"), UIPopupMessage::ButtonRole::Save);
        msg->addButton(_T("Don`t save"), UIPopupMessage::ButtonRole::Close);
        msg->addButton(_T("Cancel"), UIPopupMessage::ButtonRole::Cancel);
        int res = msg->runDialog();
        delete msg;
        printf("%d\n", res);
        fflush(stdout);
    });
    lvlut->addWidget(btn5);

    UIButton *btn6 = new UIButton(leftPanel);
    btn6->setObjectGroupId(_T("ToolButton"));
    btn6->setVectorIcon(IDI_MENUEMF, 16, 16);
    btn6->setToolTip(_T("Options"));
    lvlut->addWidget(btn6);

    UISpacer *leftStretch = new UISpacer(6, 6, SizePolicy::Fixed, SizePolicy::Expanding);
    lvlut->addSpacer(leftStretch);

    UIButton *btn7 = new UIButton(leftPanel);
    btn7->setObjectGroupId(_T("ToolButton"));
    btn7->setVectorIcon(IDI_ABOUTEMF, 16, 16);
    btn7->setToolTip(_T("About"));
    btn7->clickSignal.connect([=]() {

    });
    lvlut->addWidget(btn7);

    /* Right Panel*/
    UIWidget *rightPanel = new UIWidget(cenPanel);
    rightPanel->setObjectGroupId(_T("PanelWidget"));
    rightPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
    rightPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    rightPanel->setBaseSize(200, 50);
    cvlut->addWidget(rightPanel);

    UIBoxLayout *vlut = new UIBoxLayout(UIBoxLayout::Vertical, UILayout::AlignHCenter | UILayout::AlignVTop);
    vlut->setContentMargins(0, 6, 0, 24);
    vlut->setSpacing(6);
    rightPanel->setLayout(vlut);

    UILineEdit *le = new UILineEdit(rightPanel);
    le->setPlaceholderText(_T("Search in file..."));
    le->setObjectGroupId(_T("LineEdit"));
    le->setVectorIcon(IDI_SEARCHEMF, 14, 14);
    vlut->addWidget(le);

    UILabel *resultPanel = new UILabel(rightPanel);
    resultPanel->setObjectGroupId(_T("PanelWidget"));
    resultPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    resultPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    resultPanel->setVectorIcon(IDI_NOTFOUND, 156, 190);
    vlut->addWidget(resultPanel);

    /* View panel */
    UILabel *viewPanel = new UILabel(cenPanel);
    viewPanel->setObjectGroupId(_T("ViewWidget"));
    viewPanel->setFont({DEFAULT_FONT_NAME, 14});
    viewPanel->setText(_T("Drag & Drop a file"));
    viewPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    viewPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    viewPanel->setBaseSize(50, 50);
    viewPanel->setAcceptDrops(true);
    cvlut->addWidget(viewPanel);

    viewPanel->contextMenuSignal.connect([this](int x, int y) {
        int width = 149 * dpiRatio(), height = 245 * dpiRatio();
        UIMenu *menu = new UIMenu(this, Rect(x, y, width, height));
        menu->setObjectGroupId(_T("ToolTip"));
        UIButton *b1 = menu->addSection(_T("Close"));
        b1->clickSignal.connect([menu]() {
            menu->close();
        });
        menu->addSection(_T("Close Saved"));
        menu->addSection(_T("Close All"));
        menu->addSeparator();
        UIButton *b2 = menu->addSection(_T("Reopen With..."));
        menu->addSeparator();
        UIButton *b3 = menu->addSection(_T("Split Up"));
        b3->clickSignal.connect([b2]() {
            b2->setDisabled(true);
        });
        UIButton *b4 = menu->addSection(_T("Split Down"));
        b4->clickSignal.connect([b2]() {
            b2->setDisabled(false);
        });
        menu->addSection(_T("Copy to Window"));
        menu->addSeparator();
        menu->addSection(_T("Open New"));
        menu->showAll();
    });

    btn6->clickSignal.connect([this, btn4, btn5, btn6, btn7, le]() {
        Point wndPos = pos();
        Size wndSize = size();
        int w = 336 * dpiRatio();
        int h = 189 * dpiRatio();
        int x = wndPos.x + (wndSize.width - w) / 2;
        int y = wndPos.y + (wndSize.height - h) / 2;
        UIDialog *dlg = new UIDialog(this, Rect(x, y, w, h));
        dlg->setObjectGroupId(_T("PanelWidget"));
        dlg->setWindowTitle(_T("Options"));

        UIBoxLayout *hlut = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVBottom);
        hlut->setContentMargins(24, 22, 24, 17);
        hlut->setSpacing(12);
        dlg->setLayout(hlut);

        UIToggleButton *tbt = new UIToggleButton(dlg);
        tbt->setObjectGroupId(_T("ToggleButton"));
        tbt->setText(_T("Switch theme"));
        tbt->setChecked(m_toggleChecked);
        hlut->addWidget(tbt);
        tbt->clickSignal.connect([this, tbt, btn4, btn5, btn6, btn7, le]() {
            m_toggleChecked = tbt->isChecked();
            UIStyle *style = UIApplication::instance()->style();
            style->setTheme(m_toggleChecked ? _T("Light") : _T("Dark"));
            btn4->setVectorIcon(m_toggleChecked ? IDI_OPENFILEDARK : IDI_OPENFILE, 16, 16);
            btn5->setVectorIcon(m_toggleChecked ? IDI_SAVEFILEDARK : IDI_SAVEFILE, 16, 16);
            btn6->setVectorIcon(m_toggleChecked ? IDI_MENUDARKEMF : IDI_MENUEMF, 16, 16);
            btn7->setVectorIcon(m_toggleChecked ? IDI_ABOUTDARKEMF : IDI_ABOUTEMF, 16, 16);
            le->setVectorIcon(m_toggleChecked ? IDI_SEARCHDARKEMF : IDI_SEARCHEMF, 14, 14);
        });

        UICheckBox *chk = new UICheckBox(dlg);
        chk->setObjectGroupId(_T("Selector"));
        chk->setText(_T("Fit text to window"));
        hlut->addWidget(chk);

        UISpacer *dlgStretch = new UISpacer(1, 1, SizePolicy::Fixed, SizePolicy::Expanding);
        hlut->addSpacer(dlgStretch);

        UIButton *apply = new UIButton(dlg, _T("Apply"));
        apply->setObjectGroupId(_T("PushButton"));
        connect(apply->clickSignal, dlg, &UIDialog::accept);
        hlut->addWidget(apply);

        int res = dlg->runDialog();
        delete dlg;
        printf("%d\n", res);
        fflush(stdout);
    });

    auto setViewText = [viewPanel](const tstring &path) {
        tstring text;
        if (readFile(path, text)) {
            viewPanel->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
            viewPanel->setFont({DEFAULT_FONT_NAME, 10});
            viewPanel->setText(text, true);
        } else {
            viewPanel->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignCenter);
            viewPanel->setFont({DEFAULT_FONT_NAME, 14});
            viewPanel->setText(_T("Drag & Drop a file"));
        }
    };

    viewPanel->dropFilesSignal.connect([setViewText](const std::vector<tstring> &paths) {
        if (!paths.empty()) {
            setViewText(paths[0]);
        }
    });

    btn4->clickSignal.connect([this, setViewText]() {
        UIFileDialog *dlg = new UIFileDialog(this);
        dlg->setTitle(_T("Open"));
        dlg->setMode(UIFileDialog::Mode::OpenFile);
        dlg->setFilter(_T("Text files (*.txt);;All Files (*.*)"));
        auto paths = dlg->exec();
        if (!paths.empty()) {
            setViewText(paths[0]);
        }
    });

#ifdef __linux
    auto roundCorners = [=](bool maximized) {
        int radius = 0;
        unsigned char corners = UIDrawingEngine::CornerAll;
        if (!maximized)
            corners = cornersPlacementAndRadius(radius);
        if (radius > 0)
            radius--;

        cw->setCorners(corners);
        cw->SetMetrics(Metrics::BorderRadius, radius);

        caption->setCorners(corners & (UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop));
        caption->SetMetrics(Metrics::BorderRadius, radius);

        btn1->setCorners(corners & UIDrawingEngine::CornerRTop);
        btn1->SetMetrics(Metrics::BorderRadius, radius);

        cenPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom | UIDrawingEngine::CornerRBottom));
        cenPanel->SetMetrics(Metrics::BorderRadius, radius);

        leftPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom));
        leftPanel->SetMetrics(Metrics::BorderRadius, radius);

        btn7->setCorners(corners & UIDrawingEngine::CornerLBottom);
        btn7->SetMetrics(Metrics::BorderRadius, radius);

        viewPanel->setCorners(corners & (UIDrawingEngine::CornerRBottom));
        viewPanel->SetMetrics(Metrics::BorderRadius, radius);
    };

    roundCorners(false);
#endif

    stateChangedSignal.connect([=](int state){
        if (state == UIWindow::Normal) {
            btn2->setStockIcon(UIButton::RestoreIcon);
#ifdef __linux
            roundCorners(false);
#endif
        } else
        if (state == UIWindow::Maximized) {
            btn2->setStockIcon(UIButton::MaximizeIcon);
#ifdef __linux
            roundCorners(true);
#endif
        }
    });
}

MainWindow::~MainWindow()
{

}
