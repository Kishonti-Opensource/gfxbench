/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "EULADialog.h"
#include "Dictionary.h"
#include "MessageBox.h"

#include "graphics.h"
#include "ng/log.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopWidget>



using namespace QtUI;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_benchmarkService(NULL),
    m_isQuitting(false),
    m_apiSelector(NULL),
    m_benchmarkPage(NULL)
{
    ui->setupUi(this);
    addAction(ui->quitAction);

    if (devicePixelRatio() > 1.0) {
        ui->homeButton->setStyleSheet(getRetinaCSS("home"));
        ui->resultsButton->setStyleSheet(getRetinaCSS("results"));
        ui->compareButton->setStyleSheet(getRetinaCSS("compare"));
        ui->infoButton->setStyleSheet(getRetinaCSS("info"));
        ui->settingsButton->setStyleSheet(getRetinaCSS("settings"));
    }

    m_splash = new SplashScreen();
    m_splash->hide();
    m_splash->addAction(ui->quitAction);

	m_homePage = new HomePage(this);
	m_pageList[ui->homeButton] = m_homePage;

	m_resultsPage = new ResultsPage(this);
	m_pageList[ui->resultsButton] = m_resultsPage;

	m_comparePage = new ComparePage(this);
	m_pageList[ui->compareButton] = m_comparePage;

	m_infoPage = new InfoPage(this);
	m_pageList[ui->infoButton] = m_infoPage;

	m_optionsPage = new OptionsPage(this);
	m_pageList[ui->settingsButton] = m_optionsPage;

	foreach(QPushButton* button, m_pageList.keys())
	{
		m_pageList[button]->hide();
		//change view on menu changing
		connect(button, SIGNAL(clicked()), this, SLOT(menuButtonClicked()));
	}
	m_buttonList = m_pageList.keys();

	m_activeButton = 0;
	ui->homeButton->click();

    m_desktop = QApplication::desktop();
    connect(m_desktop, SIGNAL(resized(int)), this, SLOT(onScreenResized(int)));
    connect(m_desktop, SIGNAL(workAreaResized(int)), this, SLOT(onScreenResized(int)));
    onScreenResized(m_desktop->primaryScreen());

    m_homePage->setConfigurationListModel(&m_configurationListModel);
    m_homePage->setTestListModel(&m_testListModel);
    m_resultsPage->setSessionListModel(&m_sessionListModel);
    m_resultsPage->setResultListModel(&m_resultListModel);
    m_resultsPage->setResultDetailListModel(&m_resultDetailListModel);
    m_comparePage->setResultListModel(&m_bestResultListModel);
    m_comparePage->setCompareListModel(&m_compareListModel);
    m_comparePage->setDuelListModel(&m_duelListModel);
    m_infoPage->setSystemInfoListModel(&m_systemInfoListModel);

    localize();
}



MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setBenchmarkService(BenchmarkService *benchmarkService)
{

    m_splash->setBenchmarkService(benchmarkService);
    m_homePage->setBenchmarkService(benchmarkService);
    m_resultsPage->setBenchmarkService(benchmarkService);
    m_comparePage->setBenchmarkService(benchmarkService);
    m_infoPage->setBenchmarkService(benchmarkService);
    m_optionsPage->setBenchmarkService(benchmarkService);
    localize();

    m_testListModel.setBenchmarkService(benchmarkService);
    m_testListModel.setCursor(benchmarkService->getTests());
    m_configurationListModel.setBenchmarkService(benchmarkService);
    m_configurationListModel.setCursor(benchmarkService->getConfigurations());
    m_sessionListModel.setCursor(benchmarkService->getSessions());
    m_bestResultListModel.setCursor(benchmarkService->getBestResults());
    m_resultListModel.setCursor(benchmarkService->getBestResults());
    m_systemInfoListModel.setCursor(benchmarkService->getSystemInfo());

    m_benchmarkService = benchmarkService;

    bool success = QObject::connect(
            Dictionary::instance(), &Dictionary::languageChanged,
            this, &MainWindow::localize);
    assert(success);

#if defined(COMPUBENCH) || defined(GFXBENCH) || defined(GFXBENCH_DX)
	if (m_apiSelector == nullptr)
	{
		m_apiSelector = new ApiSelector(benchmarkService, this);
		m_homePage->setApiSelector(m_apiSelector);
		m_apiSelector->show();
	}

//	#ifdef COMPUBENCH
//		#if defined(Q_OS_OSX)
//			m_apiSelector->setFixRenderApi(tfw::ApiDefinition::METAL);
//		#else
//			m_apiSelector->setFixRenderApi(tfw::ApiDefinition::GL);
//		#endif
//	#endif
#endif
    Q_UNUSED(success);
}



void BENCHMARK_SERVICE_API MainWindow::askSynchronization(long long bytesToSynchronize)
{
    QString quantity;
    long long kiloBytes = qRound((bytesToSynchronize / 1024.0) + 0.5);
    long long megaBytes = qRound((bytesToSynchronize / 1024.0 / 1024.0) + 0.5);
    if (megaBytes > 0) {
        quantity = QString::number(megaBytes) + " MB";
    } else {
        quantity = QString::number(kiloBytes) + " kB";
    }

    if (popupMsgBox(dict("SyncLotDialogTitle"), dict("SyncLotDialogBody").replace("%s", "%1").
            arg(quantity), "OK", "Cancel", this))
    {
        m_benchmarkService->acceptSynchronization();
    } else {
        m_benchmarkService->stop();
        QApplication::quit();
    }
}



void BENCHMARK_SERVICE_API MainWindow::reportError(
        const char* errorId,
        const char* errorMessage,
        bool isFatal)
{
    if (errorId == std::string("UsernameTakenDialog")) {
        m_optionsPage->onUserNameTaken();
    } else if (errorId == std::string("EmailTakenDialog")) {
        m_optionsPage->onEmailTaken();
    } else if (errorId == std::string("InvalidEmailDialog")) {
        m_optionsPage->onInvalidCredentials();
    } else if (errorId == std::string("InvalidCredentials")) {
        m_optionsPage->onInvalidEmail();
    } else if (errorId == std::string("BatteryConnected")) {
        errorId = "BatteryConnectedTitle";
        errorMessage = "BatteryConnectedBody";
    }

	popupMsgBox(QString(errorId), QString(errorMessage), "OK", this);

    if (isFatal) {
        m_isQuitting = true;
        close();
    }
}



void BENCHMARK_SERVICE_API MainWindow::showMessage(const char* message)
{
    popupMsgBox(dict("SyncLotDialogTitle"), QString::fromStdString(message), dict("OK"), this);
}



void BENCHMARK_SERVICE_API MainWindow::showSplash()
{
    m_splash->show();
}



void BENCHMARK_SERVICE_API MainWindow::initializationFinished()
{
    Dictionary::instance()->load(QString::fromStdString(m_benchmarkService->getConfig(
            BenchmarkService::SYNCHRONIZATION_PATH) + "/localization"));
    if (!m_benchmarkPage.isNull()) {
        m_benchmarkPage->hide();
        m_benchmarkPage.reset();
    }
    m_splash->hide();
    show();
}



void BENCHMARK_SERVICE_API MainWindow::showResults()
{
    if (!m_benchmarkPage.isNull()) {
        m_benchmarkPage->hide();
        m_benchmarkPage.reset();
    }
    ui->resultsButton->click();
    show();
}



void BENCHMARK_SERVICE_API MainWindow::eventReceived()
{
    m_backgroundThread = QThread::currentThread();
    bool success = QMetaObject::invokeMethod(this, "processEvents", Qt::QueuedConnection);
    assert(success);
    Q_UNUSED(success)
}



void BENCHMARK_SERVICE_API MainWindow::createContext(
        const char* testId,
        const char* loadingImage,
        const tfw::Graphics& graphics,
        const tfw::Descriptor& config)
{
    hide();

    if (m_benchmarkPage.isNull()) {
        m_benchmarkPage.reset(new BenchmarkPage());
    }
    m_benchmarkPage->setBenchmarkService(m_benchmarkService);
    if (graphics.isFullScreen()) {
        m_benchmarkPage->setGeometry(QApplication::desktop()->screenGeometry(this));
    }
    m_resultsPage->showLatest();
    m_benchmarkPage->prepareTest(testId, loadingImage, graphics, m_backgroundThread);
}



void BENCHMARK_SERVICE_API MainWindow::testLoaded()
{
    m_benchmarkPage->showTest();
    m_benchmarkService->runTest();
}



void BENCHMARK_SERVICE_API MainWindow::updateInitMessage(const char* message)
{
    m_splash->printMessage(QString::fromStdString(message));
    m_splash->setProgress(-1);
}



void BENCHMARK_SERVICE_API MainWindow::updateSyncProgress(
        double progress,
        long long bytesNeeded,
        long long bytesWritten)
{
    m_splash->updateSyncProgress(progress, bytesNeeded, bytesWritten);
}



void BENCHMARK_SERVICE_API MainWindow::loggedIn(const char* username)
{
    m_optionsPage->onLoggedIn(username);
}



void BENCHMARK_SERVICE_API MainWindow::loggedOut()
{
    m_optionsPage->onLoggedOut();
}



void BENCHMARK_SERVICE_API MainWindow::deletedUser()
{
    m_optionsPage->onDeletedUser();
}



void BENCHMARK_SERVICE_API MainWindow::signedUp()
{
    m_optionsPage->onRegistrationSucceeded();
}



void MainWindow::localize()
{
    //m_testListModel.localize();
    //m_resultListModel.localize();
    m_homePage->localize();
    m_resultsPage->localize();
    m_comparePage->localize();
    m_optionsPage->localize();

    ui->homeButton->setText(dict("TabHome"));
    ui->resultsButton->setText(dict("TabResults"));
    ui->compareButton->setText(dict("TabCompare"));
    ui->infoButton->setText(dict("TabInfo"));
    ui->settingsButton->setText(dict("TabSettings"));
}



void MainWindow::processEvents()
{
    m_benchmarkService->processEvents();
}



void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isQuitting || popupMsgBox("Close", "ConfirmDialogBody", "Yes", "No", this)) {
        m_isQuitting = true;
        m_benchmarkService->stop();
        m_splash->close();
        event->accept();
    } else {
        event->ignore();
    }
}



void MainWindow::menuButtonClicked()
{
	QWidget* oldWidget = m_pageList[m_activeButton];
	QPushButton *senderButton = (QPushButton*)sender();
	if(senderButton != m_activeButton)
	{
		senderButton->setChecked(true);
		foreach(QPushButton* button, m_buttonList)
		{
			if(button != senderButton)
				button->setChecked(false);
		}
		m_activeButton = senderButton;

		if(oldWidget)
		{
			oldWidget->hide();
			ui->contentPanel->layout()->removeWidget(oldWidget);
		}

		QWidget *newWidget = m_pageList[senderButton];
		ui->contentPanel->layout()->addWidget(newWidget);
		newWidget->show();
	}
	else
	{
		senderButton->setChecked(true);
	}
}



void MainWindow::onScreenResized(int screen)
{
    if (m_desktop->screenGeometry(screen).width() - 100 < width()) {
        resize(m_desktop->screenGeometry(screen).width() - 100, height());
    }
    if (m_desktop->screenGeometry(screen).height() - 100 < height()) {
        resize(width(), m_desktop->screenGeometry(screen).height() - 100);
    }
    int translateX = qRound((m_desktop->screenGeometry(screen).width()-width())*0.5);
	int translateY = qRound((m_desktop->screenGeometry(screen).height()-height())*0.5);
    resize(width(), height());
    move(translateX, translateY);
}



QString MainWindow::getRetinaCSS(const QString &buttonName)
{
    QString css =
    "QPushButton#<button>Button \
    { \
    border-image: url(:/retina_<button>_base.png) 0 0 0 0 stretch stretch; \
    } \
    \
    QPushButton#<button>Button:checked \
    { \
    border-image: url(:/retina_<button>_active.png) 0 0 0 0 stretch stretch; \
    }";
    css.replace("<button>", buttonName);
    return css;
}
