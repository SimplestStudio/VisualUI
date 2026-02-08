#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uiapplication.h"
#include "uipopupmessage.h"
#include "uifiledialog.h"
#include "uidialog.h"
#include "uimenu.h"
#include "uicheckbox.h"
#include "uitogglebutton.h"
#include "uimetrics.h"
#include "uipalette.h"
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
    ui(new Ui::MainWindow),
    m_toggleChecked(false)
{
    setObjectGroupId(_T("MainWindow"));

    UIWidget *cw = new UIWidget(this);
    cw->setObjectGroupId(_T("CentralWidget"));
    setCentralWidget(cw);
    setContentsMargins(0,0,0,0);
    ui->setupUi(cw);

    /* Top panel */
    ui->btnClose->clickSignal.connect([=]() {
        close();
    });
    ui->btnClose->activationChangedSignal.connect([this](bool active) {
        UIStyle *style = UIApplication::instance()->style();
        ui->btnClose->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        ui->btnClose->update();
        ui->btnRestore->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        ui->btnRestore->update();
        ui->btnMinimize->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        ui->btnMinimize->update();
    });
    ui->btnRestore->clickSignal.connect([=]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    ui->btnMinimize->clickSignal.connect([=]() {
        showMinimized();
    });

    ui->btnSave->clickSignal.connect([=]() {
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

    ui->btnAbout->clickSignal.connect([=]() {

    });

    ui->viewPanel->contextMenuSignal.connect([this](int x, int y) {
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

    ui->btnMenu->clickSignal.connect([this]() {
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
        tbt->clickSignal.connect([this, tbt]() {
            m_toggleChecked = tbt->isChecked();
            UIStyle *style = UIApplication::instance()->style();
            style->setTheme(m_toggleChecked ? _T("Light") : _T("Dark"));
            ui->btnOpen->setVectorIcon(m_toggleChecked ? IDI_OPENFILEDARK : IDI_OPENFILE, 16, 16);
            ui->btnSave->setVectorIcon(m_toggleChecked ? IDI_SAVEFILEDARK : IDI_SAVEFILE, 16, 16);
            ui->btnMenu->setVectorIcon(m_toggleChecked ? IDI_MENUDARKEMF : IDI_MENUEMF, 16, 16);
            ui->btnAbout->setVectorIcon(m_toggleChecked ? IDI_ABOUTDARKEMF : IDI_ABOUTEMF, 16, 16);
            ui->lineEdit->setVectorIcon(m_toggleChecked ? IDI_SEARCHDARKEMF : IDI_SEARCHEMF, 14, 14);
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

    auto setViewText = [this](const tstring &path) {
        tstring text;
        if (readFile(path, text)) {
            ui->viewPanel->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVTop);
            ui->viewPanel->setFont({DEFAULT_FONT_NAME, 10});
            ui->viewPanel->setText(text, true);
        } else {
            ui->viewPanel->metrics()->setMetrics(Metrics::TextAlignment, Metrics::AlignCenter);
            ui->viewPanel->setFont({DEFAULT_FONT_NAME, 14});
            ui->viewPanel->setText(_T("Drag & Drop a file"));
        }
    };

    ui->viewPanel->dropFilesSignal.connect([setViewText](const std::vector<tstring> &paths) {
        if (!paths.empty()) {
            setViewText(paths[0]);
        }
    });

    ui->btnOpen->clickSignal.connect([this, setViewText]() {
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

        ui->caption->setCorners(corners & (UIDrawingEngine::CornerLTop | UIDrawingEngine::CornerRTop));
        ui->caption->SetMetrics(Metrics::BorderRadius, radius);

        ui->btnClose->setCorners(corners & UIDrawingEngine::CornerRTop);
        ui->btnClose->SetMetrics(Metrics::BorderRadius, radius);

        ui->appPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom | UIDrawingEngine::CornerRBottom));
        ui->appPanel->SetMetrics(Metrics::BorderRadius, radius);

        ui->leftPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom));
        ui->leftPanel->SetMetrics(Metrics::BorderRadius, radius);

        ui->btnAbout->setCorners(corners & UIDrawingEngine::CornerLBottom);
        ui->btnAbout->SetMetrics(Metrics::BorderRadius, radius);

        ui->viewPanel->setCorners(corners & (UIDrawingEngine::CornerRBottom));
        ui->viewPanel->SetMetrics(Metrics::BorderRadius, radius);
    };

    roundCorners(false);
#endif

    stateChangedSignal.connect([=](int state) {
        if (state == UIWindow::Normal) {
            ui->btnRestore->setStockIcon(UIButton::RestoreIcon);
#ifdef __linux
            roundCorners(false);
#endif
        } else
        if (state == UIWindow::Maximized) {
            ui->btnRestore->setStockIcon(UIButton::MaximizeIcon);
#ifdef __linux
            roundCorners(true);
#endif
        }
    });
}

MainWindow::~MainWindow()
{

}
