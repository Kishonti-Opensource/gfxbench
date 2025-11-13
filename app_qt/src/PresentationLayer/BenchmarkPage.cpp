/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "BenchmarkPage.h"

#include "Dictionary.h"
#include "MessageBox.h"
#include "TestMode.h"

#include "benchmarkservice.h"
#ifdef Q_OS_WIN
    #include "graphics/dxwin32graphicscontext.h"
    #include "graphics/windowsvulkangraphicscontext.h"
#endif
#ifdef Q_OS_LINUX
#include <xcb/xcb.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include "graphics/xcbgraphicscontext.h"
#endif
#ifdef HAS_METAL
    #import <AppKit/AppKit.h>
    #include "graphics/metalgraphicscontext.h"
#endif
#include "graphics/qgraphicscontext.h"
#include "glwidget.h"

#include <QDesktopWidget>
#include <QGLWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWindowStateChangeEvent>
#include <QMessageBox>

#include <cassert>



using namespace QtUI;



#ifdef Q_OS_WIN
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif



BenchmarkPage::BenchmarkPage(QWidget *parent):
    QWidget(parent),
    m_renderWidget(0),
	m_ignoreSkipAndStop(true)
{
    ui.setupUi(this);
    ui.topLine->hide();
    ui.progressBar->setRange(0, 100);
    ui.controlPanel->hide();
   // setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setFocusPolicy(Qt::StrongFocus);

    if (devicePixelRatio() > 1.0) {
        QString logo = "QLabel#logoLabel{ border-image: url(:/retina_logo.png) 0 0 0 0 stretch stretch; }";
        ui.logoLabel->setStyleSheet(logo);
        ui.stopButton->setStyleSheet(getButtonCSS("stop"));
        ui.skipButton->setStyleSheet(getButtonCSS("skip"));
    }

    bool success;
    success = QObject::connect(ui.skipAction, &QAction::triggered,
            this, &BenchmarkPage::onSkipTriggered);
    assert(success);
    addAction(ui.skipAction);

    success = QObject::connect(ui.stopAction, &QAction::triggered,
            this, &BenchmarkPage::onStopTriggered);
    assert(success);
    addAction(ui.stopAction);

    Q_UNUSED(success)

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(avoidSleep()));
    m_timer.start(5000);
}



void BenchmarkPage::setBenchmarkService(BenchmarkService *benchmarkService)
{
    m_benchmarkService = benchmarkService;
}



void BenchmarkPage::changeEvent(QEvent *event)
{
    if ((event->type() == QEvent::WindowStateChange) && (windowState() == Qt::WindowMinimized)) {
        if (isFullScreen()) {
            //m_benchmarkService->stopTests();
        }
    }
}



void BenchmarkPage::showEvent(QShowEvent* event)
{
    TestMode::instance()->enter();
#ifdef Q_OS_MAC
    //TODO test on mac if works enable it
   // macUtil.disableScreensaver();
#else
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, SPIF_SENDWININICHANGE);
    if (SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED) == NULL)
    {
        // try XP variant as well just to make sure
        SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
    }
#endif
}



void BenchmarkPage::hideEvent(QHideEvent* event)
{
    TestMode::instance()->leave();
#ifdef Q_OS_MAC
    //TODO test on mac if works enable it
 //   macUtil.releaseScreensaverLock();
#else
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, SPIF_SENDWININICHANGE);
    // set state back to normal
    SetThreadExecutionState(ES_CONTINUOUS);
#endif
}



void BenchmarkPage::closeEvent(QCloseEvent* event)
{
	if (!m_ignoreSkipAndStop){
		m_benchmarkService->stopTests();
		event->ignore();

		m_ignoreSkipAndStop = true;
	}
}


void BenchmarkPage::leaveEvent(QEvent *event)
{
    if (!m_ignoreSkipAndStop) {
        m_benchmarkService->stopTests();
        event->ignore();

        m_ignoreSkipAndStop = true;
    }
    disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(avoidSleep()));
}


void BenchmarkPage::focusOutEvent(QFocusEvent*)
{
    if(isFullScreen()) {
        //m_benchmarkService->stopTests();
    }
}



void BenchmarkPage::onSkipTriggered()
{
	if (!m_ignoreSkipAndStop){
		m_benchmarkService->skipTest();
		m_ignoreSkipAndStop = true;
	}

}



void BenchmarkPage::onStopTriggered()
{
	if (!m_ignoreSkipAndStop){
		m_benchmarkService->stopTests();
		m_ignoreSkipAndStop = true;
	}
}



void BenchmarkPage::prepareTest(
        const char* testId,
        const char* loadingImage,
        const tfw::Graphics& graphics,
        QThread *renderThread)
{
	m_ignoreSkipAndStop = true;
    ui.loadingScreen->setTestItem(testId, loadingImage);
    ui.stackedWidget->setCurrentWidget(ui.loadingScreen);

    if (graphics.isFullScreen()) {
        setCursor(QCursor(Qt::BlankCursor));
#ifdef Q_OS_WIN
        /* Workaround to start progress bar */
        show();
#endif
        showFullScreen();
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
        show();
    }
    repaint();
    setFocus();

    if (m_renderWidget) {
        ui.stackedWidget->removeWidget(m_renderWidget);
        delete m_renderWidget;
        m_renderWidget = 0;
    }
    m_context.reset();

    if (graphics.versions().front().type() == tfw::ApiDefinition::DX || graphics.versions().front().type() == tfw::ApiDefinition::DX12) {
        createDxContext(graphics);
	} else if (graphics.versions().front().type() == tfw::ApiDefinition::METAL) {
		createMetalContext(graphics);
	} else if (graphics.versions().front().type() == tfw::ApiDefinition::VULKAN){
		createVulkanContext(graphics);
    } else {
        createGlContext(graphics, renderThread);
    }

    ui.stackedWidget->setCurrentWidget(ui.loadingScreen);

    QString testName = dict(testId);
    m_progressText = QString("%1 - %2%").arg(testName);
    m_benchmarkService->loadTest(m_context.data(), width() * devicePixelRatio(),
            height() * devicePixelRatio());
}



void BenchmarkPage::showTest()
{
    if (m_renderWidget) {
        m_renderWidget->resize(ui.stackedWidget->size());
        ui.stackedWidget->setCurrentWidget(m_renderWidget);
    }

	// enable skip and stop test
	m_ignoreSkipAndStop = false;
}



QString BenchmarkPage::getButtonCSS(const QString &name)
{
    QString css =
    "QPushButton#<name>Button \
    { \
        border-image: url(:/retina_<name>.png) 0 0 0 0 stetch stretch; \
    } \
    \
    QPushButton#<name>Button:hover \
    { \
        border-image: url(:/retina_<name>_hover.png) 0 0 0 0 stetch stretch; \
    }";
    css.replace("<name>", name);
    return css;
}



void BenchmarkPage::createDxContext(const tfw::Graphics& graphics)
{
#ifdef Q_OS_WIN
    m_renderWidget = new QWidget(this);
    ui.stackedWidget->addWidget(m_renderWidget);
    m_renderWidget->setFixedSize(QSize(width(), height()));
    SetWindowPos(
            (HWND)m_renderWidget->winId(),
            HWND_TOP,
            0,
            0,
            width(),
            height(),
            SWP_NOMOVE); // XXX Force update

	int major = graphics.versions().front().major();
	int minor = graphics.versions().front().minor();
	GraphicsContext *context;
	if (graphics.versions().front().type() == tfw::ApiDefinition::DX)
	{
		context = new DxWin32GraphicsContext((HWND)m_renderWidget->winId());
		dynamic_cast<DxWin32GraphicsContext*>(context)->init(graphics.deviceId(), graphics.config().isVsync());
	}
	else if(graphics.versions().front().type() == tfw::ApiDefinition::DX12)
	{
	    context = new DxBareGraphicsContext((HWND)m_renderWidget->winId(), major, minor, GraphicsContext::DIRECTX12);
	}
    m_context.reset(context);
#endif
    Q_UNUSED(graphics)
}


void BenchmarkPage::createVulkanContext(const tfw::Graphics& graphics)
{
    m_renderWidget = new QWidget(this);
	ui.stackedWidget->addWidget(m_renderWidget);
	m_renderWidget->setFixedSize(QSize(width(), height()));

#ifdef Q_OS_WIN
	SetWindowPos(
		(HWND)m_renderWidget->winId(),
		HWND_TOP,
		0,
		0,
		width(),
		height(),
		SWP_NOMOVE); // XXX Force update

        WindowsVulkanGraphicsContext* context = new WindowsVulkanGraphicsContext((HWND)m_renderWidget->winId());
	m_context.reset(context);
#elif defined(Q_OS_OSX)
    //simply do nothing
#else
    QPlatformNativeInterface *native =  QGuiApplication::platformNativeInterface();
    xcb_connection_t* connection = static_cast<xcb_connection_t *>(native->nativeResourceForWindow("connection", nullptr));
    xcb_screen_t * screen = static_cast<xcb_screen_t *>(native->nativeResourceForScreen("display", QGuiApplication::screens()[0] ));
    xcb_window_t xcb_window = static_cast<xcb_window_t>(m_renderWidget->winId());
    Q_ASSERT(screen);
    Q_ASSERT(native);
    Q_ASSERT(connection);
    Q_ASSERT(xcb_window);
    XCBGraphicsContext *xcb_ctx = new XCBGraphicsContext();
    xcb_ctx->init(connection, xcb_window, screen);

    m_context.reset(xcb_ctx);
#endif
	Q_UNUSED(graphics)
}



void BenchmarkPage::createMetalContext(const tfw::Graphics& graphics)
{
#ifdef HAS_METAL
    m_renderWidget = new QWidget(this);
    ui.stackedWidget->addWidget(m_renderWidget);
    MetalGraphicsContextImp* context = new MetalGraphicsContextImp();
    CAMetalLayer* layer = [CAMetalLayer new];
    layer.opaque = YES;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = YES;
    NSColor *bgColor = [NSColor colorWithCalibratedRed:0.0f green:0.0f blue:0.0f alpha:1.0f];
    [layer setBackgroundColor:[bgColor CGColor]];
    NSView* view = (__bridge NSView*)(void*)m_renderWidget->effectiveWinId();
    view.wantsLayer = YES;
    [view setLayer:layer];
    m_renderWidget->setFixedSize(QSize(width(), height()));

    layer.contentsScale = devicePixelRatio();

    context->setMetalLayer(layer);
    m_context.reset(context);
#endif
    Q_UNUSED(graphics)
}




void BenchmarkPage::createGlContext(const tfw::Graphics& graphics, QThread* renderThread)
{
    QGLWidget* widget = new GLWidget(glFormatFromDescriptor(graphics), this);
    ui.stackedWidget->addWidget(widget);
    ui.stackedWidget->setCurrentWidget(widget);

#ifdef Q_OS_OSX
	// do not remove; ensure correct context creation on macOS
	widget->makeCurrent();
    widget->doneCurrent();
#endif

    widget->context()->moveToThread(renderThread);
    m_context.reset(new QGraphicsContext(widget));
    m_renderWidget = widget;
}



QGLFormat BenchmarkPage::glFormatFromDescriptor(const tfw::Graphics& graphics)
{
    QGLFormat format;

    const std::vector<tfw::ApiDefinition> &versions = graphics.versions();
    tfw::ApiDefinition reqVersion;
    for (size_t i = 0; i < versions.size(); i++) {
        if (versions[i].type() == tfw::ApiDefinition::GL) {
            reqVersion = versions[i];
        }
    }
    format.setVersion(reqVersion.major(), reqVersion.minor());
    if (reqVersion.major() >= 4) {
        format.setProfile(QGLFormat::CoreProfile);
    } else {
        format.setProfile(QGLFormat::CompatibilityProfile);

#ifdef __linux__
        // Note: __linux__ define is enabled under Android too
        if (reqVersion.major() == 3 && reqVersion.minor() == 3) {
            // Manhattan 3.0, ALU2, Driver2... with extensions
            format.setProfile(QGLFormat::CoreProfile);
        } else {
            // On Linux some drivers can not create 3.0 profile correctly (T-Rex, ALU, Driver...etc)
            // We fall back to 2.1 to get the legacy GL functions, like rendering without VAO
            format.setVersion(2, 1);
        }
#endif
    }

    const tfw::Config &config = graphics.config();
    format.setRedBufferSize(config.red());
    format.setGreenBufferSize(config.green());
    format.setBlueBufferSize(config.blue());
    if (config.depth() >= 0) {
        format.setDepthBufferSize(config.depth());
    } else {
        format.setDepth(false);
    }
    if (config.alpha() >= 0) {
        format.setAlphaBufferSize(config.alpha());
    } else {
        format.setAlpha(false);
    }
    if (config.samples() >= 0) {
        format.setSamples(config.samples());
    }

    return format;
}

void QtUI::BenchmarkPage::avoidSleep()
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_I, Qt::NoModifier);
    QCoreApplication::postEvent(this, event);

    event = new QKeyEvent(QEvent::KeyRelease, Qt::Key_I, Qt::NoModifier);
    QCoreApplication::postEvent(this, event);
}
