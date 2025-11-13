/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "DuelWidget.h"
#include "ui_DuelWidget.h"
#include "Dictionary.h"

#include <QDebug>
#include <QPixmap>
#include <QIcon>

using namespace QtUI;

int DuelWidget::s_decimals = 0;

DuelWidget::DuelWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DuelWidget),
    m_yourScore(-1.0),
    m_otherScore(-1.0),
    m_yourFps(-1.0),
    m_otherFps(-1.0),
    m_percentLimit(5.0)
{
    ui->setupUi(this);
}

DuelWidget::~DuelWidget()
{
    delete ui;
}

DuelWidget *DuelWidget::createDuelWidget(const QString &testName, const QIcon& icon, double yourScore, double yourFps, double otherScore, double otherFps, const QString &metric, QWidget *parent)
{
    DuelWidget *w = new DuelWidget(parent);
    w->setTestName(testName);
    w->setYourScore(yourScore, yourFps);
    w->setOtherScore(otherScore, otherFps);
    w->setMetric(metric);
    w->localize();
    w->ui->testIconLabel->setPixmap(icon.pixmap(w->ui->testIconLabel->size()));
    return w;
}

void DuelWidget::localize()
{
    this->setTestName(m_testName);
    this->setYourScore(m_yourScore, m_yourFps);
    this->setOtherScore(m_otherScore, m_otherFps);
    this->setMetric(m_metric);
}

void DuelWidget::setTestName(const QString &name)
{
    m_testName = name;
    ui->testNameLabel->setText(QtUI::dict(m_testName));
}

QString DuelWidget::testName()
{
    return m_testName;
}

void DuelWidget::setYourScore(double score, double fps)
{
    m_yourScore = score;
    m_yourFps = fps;
    if(m_yourScore  > 0)
    {
        if(s_decimals)
        {
            m_yourScoreStr = QString::number(m_yourScore, 'f', s_decimals);
        }
        else
        {
            m_yourScoreStr = QString::number(qRound(m_yourScore-0.5));
        }
        ui->yourScoreLabel->setText(m_yourScoreStr);
    }
    else
    {
        m_yourScoreStr = "Results_NA";
        ui->yourScoreLabel->setText(dict(m_yourScoreStr));
    }
    if(m_yourFps > 0)
    {
        QString fpsStr = QString::number(m_yourFps, 'f', 1);
        ui->yourFpsLabel->setText("(" + fpsStr + " FPS)");
        ui->yourFpsLabel->show();
    }
    else
    {
        ui->yourFpsLabel->hide();
    }

    if(m_yourScore == -1)
    {
        ui->yourMetricLabel->hide();
    }
    else
    {
        ui->yourMetricLabel->show();
    }

    if(m_otherScore == -1)
    {
        ui->otherMetricLabel->hide();
    }
    else
    {
        ui->otherMetricLabel->show();
    }

    calculateAndShow();
}

void DuelWidget::setOtherScore(double score, double fps)
{
    m_otherScore = score;
    m_otherFps = fps;
    if(m_otherScore  > 0)
    {
        if(s_decimals)
        {
            m_otherScoreStr = QString::number(m_otherScore, 'f', s_decimals);
        }
        else
        {
            m_otherScoreStr = QString::number(qRound(m_otherScore-0.5));
        }
        ui->otherScoreLabel->setText(m_otherScoreStr);
    }
    else
    {
        m_otherScoreStr = "Results_NA";
        ui->otherScoreLabel->setText(dict(m_otherScoreStr));
    }
    if(m_otherFps > 0)
    {
        QString fpsStr = QString::number(m_otherFps, 'f', 1);
        ui->otherFpsLabel->setText("(" + fpsStr + " FPS)");
        ui->otherFpsLabel->show();
    }
    else
    {
        ui->otherFpsLabel->hide();
    }

    if(m_yourScore == -1)
    {
        ui->yourMetricLabel->hide();
    }
    else
    {
        ui->yourMetricLabel->show();
    }

    if(m_otherScore == -1)
    {
        ui->otherMetricLabel->hide();
    }
    else
    {
        ui->otherMetricLabel->show();
    }
    calculateAndShow();
}

void DuelWidget::setMetric(const QString &metric)
{
    m_metric = metric;
    if(m_yourScore == -1)
    {
        ui->yourMetricLabel->hide();
    }
    else
    {
        ui->yourMetricLabel->setText(m_metric);
        ui->yourMetricLabel->show();
    }

    if(m_otherScore == -1)
    {
        ui->otherMetricLabel->hide();
    }
    else
    {
        ui->otherMetricLabel->setText(m_metric);
        ui->otherMetricLabel->show();
    }
    calculateAndShow();
}

QString DuelWidget::getMetric()
{
    return m_metric;
}

void DuelWidget::setDecimals(int decimals)
{
    s_decimals = decimals;
}

void DuelWidget::calculateAndShow()
{
    if(m_otherScore == -1 || m_yourScore == -1)
    {
        showNone();
        return;
    }
    double g, l;
    g = m_otherScore;
    l = m_yourScore;
    if(m_yourScore > m_otherScore)
    {
        g = m_yourScore;
        l = m_otherScore;
    }

    double ratio = (g/l) - 1.0;
    if(ratio < m_percentLimit*0.01)
    {
        QString percent = QString::number(ratio*100.0, 'f', 1);
        showBlue(percent.append("%"));
    }
    else if(ratio < 0.3)
    {
        QString percent = QString::number(ratio*100.0, 'f', 2);
        if(m_yourScore >= m_otherScore)
        {
            showGreen(percent.append("%"));
        }
        else
        {
            showRed(percent.append("%"));
        }
    }
    else if(ratio < 1.0)
    {
        QString percent = QString::number(ratio*100.0, 'f', 2);
        if(m_yourScore >= m_otherScore)
        {
            showGreen(percent.append("%"));
        }
        else
        {
            showRed(percent.append("%"));
        }
    }
    else
    {
        QString sratio = QString::number(ratio+1, 'f', 2);
        if(m_yourScore >= m_otherScore)
        {
            showGreen(QString("x").append(sratio));
        }
        else
        {
            showRed(QString("x").append(sratio));
        }
    }

}

void DuelWidget::showRed(const QString &text)
{
    QString css = "QLabel#indicatorLabel%1{ border-image: url(%2) stretch stretch 0 0 0 0; }";
    ui->indicatorLabelRight->setStyleSheet(css.arg("Right", property("redArrow").toString()));
    ui->indicatorLabelRight->setText(text);
    ui->indicatorLabelLeft->setStyleSheet(css.arg("Left", property("blankArrow").toString()));
    ui->indicatorLabelLeft->setText("");
    ui->indicatorLabelLeft->show();
    ui->indicatorLabelRight->show();
}

void DuelWidget::showGreen(const QString &text)
{
    QString css = "QLabel#indicatorLabel%1{ border-image: url(%2) stretch stretch 0 0 0 0; }";
    ui->indicatorLabelLeft->setStyleSheet(css.arg("Left", property("greenArrow").toString()));
    ui->indicatorLabelLeft->setText(text);
    ui->indicatorLabelRight->setStyleSheet(css.arg("Right", property("blankArrow").toString()));
    ui->indicatorLabelRight->setText("");
    ui->indicatorLabelRight->show();
    ui->indicatorLabelLeft->show();
}

void DuelWidget::showBlue(const QString&)
{
    QString css = "QLabel#indicatorLabel%1{ border-image: url(%2) stretch stretch 0 0 0 0; }";
    ui->indicatorLabelLeft->setStyleSheet(css.arg("Left", property("blueArrowLeft").toString()));
    ui->indicatorLabelRight->setStyleSheet(css.arg("Right", property("blueArrowRight").toString()));
    ui->indicatorLabelLeft->setText("");
    ui->indicatorLabelRight->setText("");
    ui->indicatorLabelLeft->show();
    ui->indicatorLabelRight->show();
}

void DuelWidget::showNone()
{
    ui->indicatorLabelLeft->hide();
    ui->indicatorLabelRight->hide();
}
