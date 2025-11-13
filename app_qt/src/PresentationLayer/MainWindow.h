/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_MainWindow.h"

#include "SplashScreen.h"
#include "HomePage.h"
#include "ResultsPage.h"
#include "ComparePage.h"
#include "InfoPage.h"
#include "OptionsPage.h"
#include "BenchmarkPage.h"
#include "LoadingScreen.h"
#include "CursorListModel.h"
#include "ConfigurationListModel.h"
#include "TestListModel.h"

#include "benchmarkservice.h"

#include <QMainWindow>
#include <QPushButton>
#include <QScopedPointer>
#include <QThread>

class ApiSelector;

namespace QtUI
{
    class MainWindow : public QMainWindow, public BenchmarkServiceCallback
	{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
        void setBenchmarkService(BenchmarkService *benchmarkService);

        void BENCHMARK_SERVICE_API localizationChanged() {}
        void BENCHMARK_SERVICE_API askSynchronization(long long bytesToSynchronize);
        void BENCHMARK_SERVICE_API reportError(
                const char* errorId,
                const char* errorMessage,
                bool isFatal);
        void BENCHMARK_SERVICE_API showMessage(const char* message);
        void BENCHMARK_SERVICE_API showSplash();
        void BENCHMARK_SERVICE_API initializationFinished();
        void BENCHMARK_SERVICE_API showResults();
        void BENCHMARK_SERVICE_API updateInitMessage(const char* message);
        void BENCHMARK_SERVICE_API updateSyncProgress(
                double progress,
                long long bytesNeeded,
                long long bytesWritten);
        void BENCHMARK_SERVICE_API loggedIn(const char* username);
        void BENCHMARK_SERVICE_API loggedOut();
        void BENCHMARK_SERVICE_API deletedUser();
        void BENCHMARK_SERVICE_API signedUp();
        void BENCHMARK_SERVICE_API eventReceived();
        void BENCHMARK_SERVICE_API createContext(
                const char* testId,
                const char* loadingImage,
                const tfw::Graphics& graphics,
                const tfw::Descriptor& config);
        void BENCHMARK_SERVICE_API testLoaded();
	protected:
		void closeEvent(QCloseEvent *event);


        private slots:
		void menuButtonClicked();
        void onScreenResized(int screen);
        void localize();
        void processEvents();
	private:
        Ui::MainWindow *ui;
        BenchmarkService *m_benchmarkService;
        QThread* m_backgroundThread;
        bool m_isQuitting;

        QString getRetinaCSS(const QString &buttonName);

		QList<QPushButton*> m_buttonList;
		QMap<QPushButton*, QWidget*> m_pageList;
		QPushButton *m_activeButton;
        QDesktopWidget *m_desktop;
		ApiSelector *m_apiSelector;

        SplashScreen *m_splash;
		HomePage *m_homePage;
		ResultsPage *m_resultsPage;
		ComparePage *m_comparePage;
		InfoPage *m_infoPage;
		OptionsPage *m_optionsPage;
		QScopedPointer<BenchmarkPage> m_benchmarkPage;

        CursorListModel m_compareListModel;
        CursorListModel m_bestResultListModel;
        CursorListModel m_duelListModel;
        CursorListModel m_resultListModel;
        CursorListModel m_sessionListModel;
        CursorListModel m_systemInfoListModel;
        CursorListModel m_resultDetailListModel;
        TestListModel m_testListModel;
        ConfigurationListModel m_configurationListModel;
	};
}

#endif // MAINWINDOW_H
