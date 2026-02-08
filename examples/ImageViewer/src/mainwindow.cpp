#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uiapplication.h"
#include "uifiledialog.h"
#include "uimenu.h"
#include "uipalette.h"
#include "uistyle.h"
#ifdef _WIN32
# define to_tstring std::to_wstring
#else
# include "uidrawningengine.h"
# define to_tstring std::to_string
#endif


MainWindow::MainWindow(const Rect &rc, BYTE flags) :
    UIWindow(nullptr, rc, flags),
    ui(new Ui::MainWindow),
    m_image(nullptr),
    m_scale(1.0),
    m_progress(25)
{
    setObjectGroupId(_T("MainWindow"));

    UIWidget *cw = new UIWidget(this);
    cw->setObjectGroupId(_T("CentralWidget"));
    setCentralWidget(cw);
    setContentsMargins(0,0,0,0);
    ui->setupUi(cw);

    /* Top Panel*/
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

    ui->tbtSwitchTheme->clickSignal.connect([this]() {
        bool isChecked = ui->tbtSwitchTheme->isChecked();
        UIStyle::instance().setTheme(isChecked ? _T("Light") : _T("Dark"));
    });

    ui->rbtFit->clickSignal.connect([this]() {
        ui->rbtFit->setChecked(true);
        ui->rbtActual->setChecked(false);
    });
    ui->rbtActual->clickSignal.connect([this]() {
        ui->rbtFit->setChecked(false);
        ui->rbtActual->setChecked(true);
    });

    ui->btnZoomIn->clickSignal.connect([this]() {
        m_progress = m_progress + 1;
        ui->prBar->setProgress(m_progress);
    });
    ui->btnZoomOut->clickSignal.connect([this]() {
        m_progress = m_progress - 1;
        ui->prBar->setProgress(m_progress);
    });
    ui->btnZoomReset->clickSignal.connect([this]() {

    });

    ui->prBar->setProgress(m_progress);

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

    auto setImage = [this](const tstring &path) {
        if (m_image) {
            delete m_image, m_image = nullptr;
        }
        m_image = new UIPixmap(path);
        if (!m_image->isValid()) {
            ui->viewPanel->setPixmap(UIPixmap());
            ui->viewPanel->setText(_T("Drag & Drop a file"));
            ui->bottomPanel->setText(_T(""));
            delete m_image; m_image = nullptr;
            return;
        }

        ui->viewPanel->setText(_T(""));
        Size sz = m_image->imageSize();
        tstring txt = to_tstring(sz.width) + _T(" x ") + to_tstring(sz.height);
        ui->bottomPanel->setText(txt);

        int w = 0, h = 0;
        ui->viewPanel->size(&w, &h);
        double drawWidth = 0, drawHeight = 0;
        calculateFitSize(sz, w, h, drawWidth, drawHeight);

        ui->viewPanel->setPixmap(*m_image);
        ui->viewPanel->setIconSize(drawWidth, drawHeight);
    };
    ui->viewPanel->dropFilesSignal.connect([setImage](const std::vector<tstring> &paths) {
        if (!paths.empty()) {
            setImage(paths[0]);
        }
    });
    ui->btnOpen->clickSignal.connect([=]() {
        UIFileDialog *dlg = new UIFileDialog(this);
        dlg->setTitle(_T("Open"));
        dlg->setMode(UIFileDialog::Mode::OpenFile);
        dlg->setFilter(_T("JPEG Images (*.jpg);;All Files (*.*)"));
        auto paths = dlg->exec();
        if (!paths.empty()) {
            setImage(paths[0]);
        }
    });

    ui->viewPanel->resizeSignal.connect([this](int w, int h) {
        if (m_image) {
            Size sz = m_image->imageSize();
            double drawWidth = 0, drawHeight = 0;
            calculateFitSize(sz, w, h, drawWidth, drawHeight);

            ui->viewPanel->SetMetrics(Metrics::IconWidth, drawWidth);
            ui->viewPanel->SetMetrics(Metrics::IconHeight, drawHeight);
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

        ui->caption->setCorners(corners & (UIDrawingEngine::CornerRTop | UIDrawingEngine::CornerLTop));
        ui->caption->SetMetrics(Metrics::BorderRadius, radius);

        ui->leftPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom));
        ui->leftPanel->SetMetrics(Metrics::BorderRadius, radius);

        ui->bottomPanel->setCorners(corners & UIDrawingEngine::CornerRBottom);
        ui->bottomPanel->SetMetrics(Metrics::BorderRadius, radius);
        ui->appPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom | UIDrawingEngine::CornerRBottom));
        ui->appPanel->SetMetrics(Metrics::BorderRadius, radius);

        ui->rightPanel->setCorners(corners & UIDrawingEngine::CornerRBottom);
        ui->rightPanel->SetMetrics(Metrics::BorderRadius, radius);
        ui->btnClose->setCorners(corners & UIDrawingEngine::CornerRTop);
        ui->btnClose->SetMetrics(Metrics::BorderRadius, radius);
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
    if (m_image) {
        delete m_image, m_image = nullptr;
    }
}

void MainWindow::calculateFitSize(const Size &imgSize, int w, int h, double &drawW, double &drawH) const noexcept
{
    double rectW = w / m_dpi_ratio;
    double rectH = h / m_dpi_ratio;
    double aspect = (double)imgSize.width / imgSize.height;
    if (rectW / rectH > aspect) {
        drawH = rectH;
        drawW = drawH * aspect;
    } else {
        drawW = rectW;
        drawH = drawW / aspect;
    }
    drawW *= m_scale;
    drawH *= m_scale;
}
