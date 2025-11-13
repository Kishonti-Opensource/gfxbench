/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DUELWIDGET_H
#define DUELWIDGET_H

#include <QWidget>

namespace Ui {
class DuelWidget;
}

class DuelWidget : public QWidget
{
    Q_OBJECT

public:
    static DuelWidget *createDuelWidget(const QString &testName, const QIcon& icon, double yourScore, double yourFps, double otherScore, double otherFps, const QString &metric, QWidget *parent = 0);
    
    void setTestName(const QString &name);
    QString testName();
    void setYourScore(double score, double fps = -1);
    void setOtherScore(double score, double fps = -1);
    void setMetric(const QString &metric);
    QString getMetric();
    
    static void setDecimals(int decimals);

public slots:
    void localize();
    
protected:
    explicit DuelWidget(QWidget *parent = 0);
    ~DuelWidget();
    
private:
    void calculateAndShow();
    void correctAlignment();
    void showRed(const QString &text);
    void showGreen(const QString &text);
    void showBlue(const QString &text);
    void showNone();
    
    Ui::DuelWidget *ui;
    
    static int s_decimals;
    
    QString m_testName;
    double m_yourScore, m_otherScore;
    double m_yourFps, m_otherFps;
    double m_percentLimit;
    QString m_metric;
    QString m_yourScoreStr, m_otherScoreStr;
};

#endif // DUELWIDGET_H
