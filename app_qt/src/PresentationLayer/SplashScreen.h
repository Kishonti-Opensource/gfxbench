/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>



namespace Ui {
class SplashScreen;
}



class BenchmarkService;



class SplashScreen: public QWidget
{
    Q_OBJECT
public:
    explicit SplashScreen(QWidget *parent = 0);
    ~SplashScreen();
    
    void setBenchmarkService(BenchmarkService *benchmarkService);
    void printMessage(const QString &message, const QString &detail = QString());
    void setProgress(double value);
    void updateSyncProgress(
            double totalProgress,
            qint64 totalBytesNeeded,
            qint64 totalBytesWritten);
private:
    void moveToCenter();
    
    Ui::SplashScreen *ui;
    BenchmarkService *m_benchmarkService;
    bool m_isQuitting;
};



#endif // SPLASHSCREEN_H
