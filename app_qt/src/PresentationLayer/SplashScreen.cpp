/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "SplashScreen.h"
#include "ui_SplashScreen.h"

#include "Dictionary.h"
#include "MessageBox.h"

#include "benchmarkservice.h"

#include <QDesktopWidget>



using namespace QtUI;



SplashScreen::SplashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SplashScreen),
    m_benchmarkService(0),
    m_isQuitting(false)
{
    ui->setupUi(this);

#ifdef OS_WIN
	QString css = "QWidget#contentWidget { background-color: #fff; border: 2px solid #dadada; }";
	ui->contentWidget->setStyleSheet(css);
#endif
    
    if (devicePixelRatio() > 1.0)
    {
        QString css =
        "QLabel#logoLabel \
        { \
            border-image: url(:/retina_logo.png); \
        }";
        ui->logoLabel->setStyleSheet(css);
    }
    
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    moveToCenter();

    printMessage("Loading");
    ui->progressBar->setValue(0);
}



SplashScreen::~SplashScreen()
{
    delete ui;
}



void SplashScreen::setBenchmarkService(BenchmarkService *benchmarkService)
{
    m_benchmarkService = benchmarkService;
}



void SplashScreen::printMessage(const QString &message, const QString &detail)
{
    setWindowTitle(QApplication::applicationName() + " " + QApplication::applicationVersion() +
            " - " + dict(message));
    ui->messageLabel->setText(dict(message));
    ui->detailLabel->setText(detail);
}



void SplashScreen::setProgress(double value)
{
    if ((value < 0) && (ui->progressBar->maximum() != 0)) {
        ui->progressBar->setMaximum(0);
    }
    if ((value >= 0) && (ui->progressBar->maximum() == 0)) {
        ui->progressBar->setMaximum(1000);
    }
    if (value >= 0) {
        int ival = qRound(ui->progressBar->maximum() * value);
        ui->progressBar->setValue(ival);
    }
}



void SplashScreen::updateSyncProgress(
    double totalProgress,
    qint64 totalBytesNeeded,
    qint64 totalBytesWritten)
{
    QStringList args;
    args.append(QString::number(qRound(totalProgress * 100.0)));
    args.append(QString::number(qRound((totalBytesWritten / 1024.0 / 1024.0) + 0.5)));
    args.append(QString::number(qRound((totalBytesNeeded / 1024.0 / 1024.0) + 0.5)));
    setProgress(totalProgress);
    printMessage(dict("SyncronizingLoadingString", args).replace("%%", "%"));
}



void SplashScreen::moveToCenter()
{
    QDesktopWidget* desktop = QApplication::desktop();
    QRect primary = desktop->screenGeometry(desktop->primaryScreen());
    QRect newRect = rect();
    newRect.moveCenter(primary.center());
    move(newRect.topLeft());
}
