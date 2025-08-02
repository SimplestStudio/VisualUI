#include "mainwindow.h"
#include "uiapplication.h"
#include "uifiledialog.h"
#include "uimenu.h"
#include "uibutton.h"
#include "uilabel.h"
#include "uiradiobutton.h"
#include "uitogglebutton.h"
#include "uiprogressbar.h"
#include "uimetrics.h"
#include "uipalette.h"
#include "uiboxlayout.h"
#include "uispacer.h"
#include "uicaption.h"
#include "resource.h"
#include "uistyle.h"
#ifdef _WIN32
# define to_tstring std::to_wstring
#else
# include "uidrawningengine.h"
# define to_tstring std::to_string
#endif

#define SetMetrics(role, value) metrics()->setMetrics(role, value)
#define SetColor(role, state, value) palette()->setColor(role, state, value)


MainWindow::MainWindow(const Rect &rc, BYTE flags) :
    UIWindow(nullptr, rc, flags),
    m_image(nullptr),
    m_scale(1.0),
    m_progress(25)
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

    /* Top Panel*/
    UIWidget *topPanel = new UIWidget(cw);
    topPanel->setObjectGroupId(_T("CentralWidget"));
    topPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    topPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    topPanel->setBaseSize(50, 28);
    chlut->addWidget(topPanel);

    UIBoxLayout *thlut = new UIBoxLayout(UIBoxLayout::Horizontal);
    thlut->setContentMargins(0, 0, 0, 0);
    thlut->setSpacing(0);
    topPanel->setLayout(thlut);

    UICaption *wgt5 = new UICaption(topPanel);
    wgt5->setObjectGroupId(_T("HeaderBar"));
    wgt5->setText(_T("Image Viewer"));

    UIButton *btn1 = new UIButton(topPanel);
    btn1->setObjectGroupId(_T("HeaderCloseButton"));
    btn1->setStockIcon(UIButton::CloseIcon);
    btn1->setIconSize(10, 10);
    btn1->onClick([=]() {
        printf("onClose... 1\n");
        fflush(stdout);
        close();
    });
    btn1->onActivationChanged([btn1](bool active) {
        UIStyle *style = UIApplication::instance()->style();
        btn1->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn1->update();
    });

    UIButton *btn2 = new UIButton(topPanel);
    btn2->setObjectGroupId(_T("HeaderButton"));
    btn2->setStockIcon(UIButton::RestoreIcon);
    btn2->setIconSize(10, 10);
    btn2->setSupportSnapLayouts();
    btn2->onClick([=]() {
        printf("onMaximized... 1\n");
        fflush(stdout);
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    btn2->onActivationChanged([btn2](bool active) {
        UIStyle *style = UIApplication::instance()->style();
        btn2->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn2->update();
    });

    UIButton *btn3 = new UIButton(topPanel);
    btn3->setObjectGroupId(_T("HeaderButton"));
    btn3->setStockIcon(UIButton::MinimizeIcon);
    btn3->setIconSize(10, 10);
    btn3->onClick([=]() {
        printf("onMinimized... 1\n");
        fflush(stdout);
        showMinimized();
    });
    btn3->onActivationChanged([btn3](bool active) {
        UIStyle *style = UIApplication::instance()->style();
        btn3->SetColor(Palette::Primitive, Palette::Normal, style->themeColor(active ? _T("TEXT_NORMAL") : _T("TEXT_DISABLED")));
        btn3->update();
    });

    thlut->addWidget(wgt5);
    thlut->addWidget(btn3);
    thlut->addWidget(btn2);
    thlut->addWidget(btn1);

    /* App Panel*/
    UIWidget *cenPanel = new UIWidget(cw);
    cenPanel->setObjectGroupId(_T("CentralWidget"));
    cenPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    cenPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    cenPanel->setBaseSize(50,28);
    chlut->addWidget(cenPanel);

    UIBoxLayout *cvlut = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHLeft | UIBoxLayout::AlignVCenter);
    cvlut->setContentMargins(0, 1, 0, 0);
    cvlut->setSpacing(1);
    cenPanel->setLayout(cvlut);

    /* Left Panel*/
    UIWidget *leftPanel = new UIWidget(cenPanel);
    leftPanel->setObjectGroupId(_T("PanelWidget"));
    leftPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Fixed);
    leftPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    leftPanel->setBaseSize(210, 50);
    cvlut->addWidget(leftPanel);

    UIBoxLayout *lvlut = new UIBoxLayout(UIBoxLayout::Vertical, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
    lvlut->setContentMargins(20, 30, 20, 30);
    lvlut->setSpacing(12);
    leftPanel->setLayout(lvlut);

    UIButton *inst = new UIButton(leftPanel);
    inst->setObjectGroupId(_T("PushButton"));
    inst->setText(_T("Open Image"));
    inst->SetMetrics(Metrics::IconAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
    inst->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVCenter);
    inst->SetMetrics(Metrics::IconMarginLeft, 36);
    inst->SetMetrics(Metrics::TextMarginLeft, 58);
    inst->setVectorIcon(IDI_OPENFILE, 16, 16);
    lvlut->addWidget(inst);

    UILabel *themeLabel = new UILabel(leftPanel);
    themeLabel->setObjectGroupId(_T("PanelLabel"));
    themeLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    themeLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    themeLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
    themeLabel->setBaseSize(50, 32);
    themeLabel->setText(_T("Theme"));
    lvlut->addWidget(themeLabel);

    UIToggleButton *tbt = new UIToggleButton(leftPanel);
    tbt->setObjectGroupId(_T("ToggleButton"));
    tbt->setText(_T("Switch theme"));
    lvlut->addWidget(tbt);
    tbt->onClick([tbt]() {
        bool isChecked = tbt->isChecked();
        UIStyle::instance().setTheme(isChecked ? _T("Light") : _T("Dark"));
    });

    UILabel *modeLabel = new UILabel(leftPanel);
    modeLabel->setObjectGroupId(_T("PanelLabel"));
    modeLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    modeLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    modeLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
    modeLabel->setBaseSize(50, 32);
    modeLabel->setText(_T("Mode"));
    lvlut->addWidget(modeLabel);

    UIRadioButton *rbtFit = new UIRadioButton(leftPanel);
    rbtFit->setObjectGroupId(_T("Selector"));
    rbtFit->setText(_T("Fit to Screen"));
    rbtFit->setChecked(true);
    lvlut->addWidget(rbtFit);

    UIRadioButton *rbtActual = new UIRadioButton(leftPanel);
    rbtActual->setObjectGroupId(_T("Selector"));
    rbtActual->setText(_T("Actual Size"));
    lvlut->addWidget(rbtActual);

    rbtFit->onClick([rbtFit, rbtActual]() {
        rbtFit->setChecked(true);
        rbtActual->setChecked(false);
    });
    rbtActual->onClick([rbtFit, rbtActual]() {
        rbtFit->setChecked(false);
        rbtActual->setChecked(true);
    });

    UILabel *zoomLabel = new UILabel(leftPanel);
    zoomLabel->setObjectGroupId(_T("PanelLabel"));
    zoomLabel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    zoomLabel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    zoomLabel->SetMetrics(Metrics::TextAlignment, Metrics::AlignHLeft | Metrics::AlignVBottom);
    zoomLabel->setBaseSize(50, 32);
    zoomLabel->setText(_T("Zoom"));
    lvlut->addWidget(zoomLabel);

    UIWidget *buttonsPanel = new UIWidget(leftPanel);
    buttonsPanel->setObjectGroupId(_T("PanelWidget"));
    buttonsPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    buttonsPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    buttonsPanel->setBaseSize(50, 28);
    lvlut->addWidget(buttonsPanel);

    UIBoxLayout *bphlut = new UIBoxLayout(UIBoxLayout::Horizontal, UIBoxLayout::AlignHCenter | UIBoxLayout::AlignVCenter);
    bphlut->setContentMargins(0, 0, 0, 0);
    bphlut->setSpacing(20);
    buttonsPanel->setLayout(bphlut);

    UIProgressBar *pb = new UIProgressBar(leftPanel);
    pb->setObjectGroupId(_T("ProgressBar"));
    pb->setProgress(m_progress);

    UIButton *btnZoomIn = new UIButton(buttonsPanel);
    btnZoomIn->setObjectGroupId(_T("ToolButton"));
    btnZoomIn->setVectorIcon(IDI_ZOOMIN, 16, 16);
    btnZoomIn->onClick([this, pb]() {
        m_progress = m_progress + 1;
        pb->setProgress(m_progress);
    });
    bphlut->addWidget(btnZoomIn);
    btnZoomIn->setToolTip(_T("Zoom In"));

    UIButton *btnZoomOut = new UIButton(buttonsPanel);
    btnZoomOut->setObjectGroupId(_T("ToolButton"));
    btnZoomOut->setVectorIcon(IDI_ZOOMOUT, 16, 16);
    btnZoomOut->onClick([this, pb]() {
        m_progress = m_progress - 1;
        pb->setProgress(m_progress);
    });
    bphlut->addWidget(btnZoomOut);
    btnZoomOut->setToolTip(_T("Zoom Out"));

    UIButton *btnZoomReset = new UIButton(buttonsPanel);
    btnZoomReset->setObjectGroupId(_T("ToolButton"));
    btnZoomReset->setVectorIcon(IDI_ZOOMRESET, 16, 16);
    btnZoomReset->onClick([this, pb]() {

    });
    bphlut->addWidget(btnZoomReset);
    btnZoomReset->setToolTip(_T("Zoom Reset"));

    lvlut->addWidget(pb);    

    UISpacer *spacerPanel = new UISpacer(5, 5, SizePolicy::Fixed, SizePolicy::Expanding);
    lvlut->addSpacer(spacerPanel);

    /* Right Panel*/
    UIWidget *rightPanel = new UIWidget(cenPanel);
    rightPanel->setObjectGroupId(_T("CentralWidget"));
    rightPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    rightPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    rightPanel->setBaseSize(50, 50);
    cvlut->addWidget(rightPanel);

    UIBoxLayout *vlut = new UIBoxLayout(UIBoxLayout::Vertical);
    vlut->setContentMargins(0, 0, 0, 0);
    vlut->setSpacing(1);
    rightPanel->setLayout(vlut);

    /* Bottom Panel*/
    UILabel *bottomPanel = new UILabel(rightPanel);
    bottomPanel->setObjectGroupId(_T("ViewWidget"));
    bottomPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    bottomPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Fixed);
    bottomPanel->setBaseSize(40, 28);

    /* View Panel*/
    UILabel *viewPanel = new UILabel(rightPanel);
    viewPanel->setObjectGroupId(_T("ViewWidget"));
    viewPanel->setSizePolicy(SizePolicy::HSizeBehavior, SizePolicy::Expanding);
    viewPanel->setSizePolicy(SizePolicy::VSizeBehavior, SizePolicy::Expanding);
    viewPanel->setBaseSize(50, 50);
    viewPanel->setFont(_T("Segoe UI"), 14);
    viewPanel->setText(_T("Drag & Drop a file"));
    viewPanel->setAcceptDrops(true);

    viewPanel->onContextMenu([this](int x, int y) {
        int width = 149 * dpiRatio(), height = 245 * dpiRatio();
        UIMenu *menu = new UIMenu(this, Rect(x, y, width, height));
        menu->setObjectGroupId(_T("ToolTip"));
        UIButton *b1 = menu->addSection(_T("Close"));
        b1->onClick([menu]() {
            menu->close();
        });
        menu->addSection(_T("Close Saved"));
        menu->addSection(_T("Close All"));
        menu->addSeparator();
        UIButton *b2 = menu->addSection(_T("Reopen With..."));
        menu->addSeparator();
        UIButton *b3 = menu->addSection(_T("Split Up"));
        b3->onClick([b2]() {
            b2->setDisabled(true);
        });
        UIButton *b4 = menu->addSection(_T("Split Down"));
        b4->onClick([b2]() {
            b2->setDisabled(false);
        });
        menu->addSection(_T("Copy to Window"));
        menu->addSeparator();
        menu->addSection(_T("Open New"));
        menu->showAll();
    });

    auto setImage = [this, viewPanel, bottomPanel](const tstring &path) {
        if (m_image) {
            delete m_image, m_image = nullptr;
        }
        m_image = new UIPixmap(path);
        if (m_image->isValid()) {
            viewPanel->setText(_T(""));
            Size sz = m_image->imageSize();
            tstring txt = to_tstring(sz.width) + _T(" x ") + to_tstring(sz.height);
            bottomPanel->setText(txt);

            int x = 0, y = 0;
            viewPanel->size(&x, &y);
            double dpi = dpiRatio();
            double rectWidth = x / dpi;
            double rectHeight = y / dpi;
            double imgAspect = (double)sz.width / sz.height;
            double drawWidth, drawHeight;
            if (rectWidth / rectHeight > imgAspect) {
                drawHeight = rectHeight;
                drawWidth = drawHeight * imgAspect;
            } else {
                drawWidth = rectWidth;
                drawHeight = drawWidth / imgAspect;
            }
            drawWidth *= m_scale;
            drawHeight *= m_scale;
            viewPanel->setPixmap(*m_image);
            viewPanel->setIconSize(drawWidth, drawHeight);
        } else {
            viewPanel->setPixmap(*m_image);
            viewPanel->setText(_T("Drag & Drop a file"));
            bottomPanel->setText(_T(""));
            delete m_image; m_image = nullptr;
        }
    };
    viewPanel->onDropFiles([setImage](std::vector<tstring> paths) {
        if (!paths.empty()) {
            setImage(paths[0]);
        }
    });
    inst->onClick([=]() {
        UIFileDialog *dlg = new UIFileDialog(this);
        dlg->setTitle(_T("Open"));
        dlg->setMode(UIFileDialog::Mode::OpenFile);
        dlg->setFilter(_T("JPEG Images (*.jpg);;All Files (*.*)"));
        auto paths = dlg->exec();
        if (!paths.empty()) {
            setImage(paths[0]);
        }
    });
    viewPanel->onResize([this, viewPanel](int w, int h) {
        if (m_image) {
            Size sz = m_image->imageSize();
            double dpi = dpiRatio();
            double rectWidth = w / dpi;
            double rectHeight = h / dpi;
            double imgAspect = (double)sz.width / sz.height;
            double drawWidth, drawHeight;
            if (rectWidth / rectHeight > imgAspect) {
                drawHeight = rectHeight;
                drawWidth = drawHeight * imgAspect;
            } else {
                drawWidth = rectWidth;
                drawHeight = drawWidth / imgAspect;
            }
            drawWidth *= m_scale;
            drawHeight *= m_scale;
            viewPanel->SetMetrics(Metrics::IconWidth, drawWidth);
            viewPanel->SetMetrics(Metrics::IconHeight, drawHeight);
        }
    });
    vlut->addWidget(viewPanel);
    vlut->addWidget(bottomPanel);

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

        wgt5->setCorners(corners & (UIDrawingEngine::CornerLTop));
        wgt5->SetMetrics(Metrics::BorderRadius, radius);

        leftPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom));
        leftPanel->SetMetrics(Metrics::BorderRadius, radius);

        bottomPanel->setCorners(corners & UIDrawingEngine::CornerRBottom);
        bottomPanel->SetMetrics(Metrics::BorderRadius, radius);

        topPanel->setCorners(corners & (UIDrawingEngine::CornerRTop | UIDrawingEngine::CornerLTop));
        topPanel->SetMetrics(Metrics::BorderRadius, radius);

        cenPanel->setCorners(corners & (UIDrawingEngine::CornerLBottom | UIDrawingEngine::CornerRBottom));
        cenPanel->SetMetrics(Metrics::BorderRadius, radius);

        btn1->setCorners(corners & UIDrawingEngine::CornerRTop);
        btn1->SetMetrics(Metrics::BorderRadius, radius);
    };

    roundCorners(false);
#endif

    onStateChanged([=](int state) {
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
    if (m_image) {
        delete m_image, m_image = nullptr;
    }
}
