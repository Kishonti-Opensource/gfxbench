/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "CompareWidget.h"
#include "ui_CompareWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QPointer>
#include <QVBoxLayout>
#include <QIcon>

#include "Dictionary.h"

using namespace QtUI;

int CompareWidget::s_decimals = 1.0;
double CompareWidget::s_devicePixelRatio = 0;

CompareWidget::CompareWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompareWidget)
{
    ui->setupUi(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    double devicePixelRatio = (double) this->devicePixelRatio();
#endif
    if(devicePixelRatio != s_devicePixelRatio)
    {
        s_devicePixelRatio = devicePixelRatio;
    }
    m_barColor = property("barColor").toString();
}

CompareWidget::~CompareWidget()
{
    delete ui;
}

CompareWidget *CompareWidget::createCompareWidget(
        const QString &deviceName,
        const QString &manufacturer,
        double score,
        double maxScore,
        double fps,
        const QString &metric,
        const QIcon &icon,
        QWidget *parent)
{
    CompareWidget *w = new CompareWidget(parent);
    w->setScore(score, fps);
    w->setMaxScore(maxScore);
    w->setDevice(deviceName, manufacturer);
    w->setMetric(metric);
    w->ui->deviceIconLabel->setPixmap(icon.pixmap(w->ui->deviceIconLabel->size()));
    w->localize();
    return w;
}

void CompareWidget::localize()
{
    this->setScore(m_score, m_fps);
    this->setMaxScore(m_maxScore);
    this->setDevice(m_deviceName, m_manufacturer);
    this->setMetric(m_metric);
}

void CompareWidget::setDevice(const QString &deviceName, const QString &manufacturer)
{
    m_deviceName = deviceName;
    m_manufacturer = QtUI::dict(manufacturer);
    ui->deviceNameLabel->setText(QtUI::dict(m_deviceName));
    if(m_manufacturer.isNull())
    {
        ui->manufacturerLabel->hide();
        ui->openArrowLabel->hide();
        ui->deviceNameLabel->setAlignment(Qt::AlignVCenter);
        QVBoxLayout* layout = (QVBoxLayout*) ui->descriptionPanel->layout();
        layout->setContentsMargins(6, 6, 32, 6);
        ui->deviceIconLabel->setStyleSheet("QLabel#deviceIconLabel{ background-color: transparent; }");
        m_barColor = property("yourBarColor").toString();
    }
    else
    {
        ui->manufacturerLabel->setText(QtUI::dict(m_manufacturer));
    }
}

void CompareWidget::setScore(double score, double fps)
{
    m_score = score;
    m_fps = fps;
    hide();
    if(m_score  > 0)
    {
        if(s_decimals)
        {
            m_scoreString = QString::number(m_score, 'f', s_decimals);
        }
        else
        {
            m_scoreString = QString::number(qRound(m_score-0.5));
        }
        ui->scoreLabel->setText(m_scoreString);
    }
    else
    {
        setMetric(QString::null);

        m_scoreString = "Results_NA";
        ui->scoreLabel->setText(dict(m_scoreString));
    }
    if(m_fps > 0)
    {
        QString fpsStr = QString::number(m_fps, 'f', 1);
        ui->fpsLabel->setText("(" + fpsStr + " FPS)");
        ui->fpsLabel->show();
    }
    else
    {
        ui->fpsLabel->hide();
    }
    show();
}

void CompareWidget::setMetric(const QString &metric)
{
    m_metric = metric;
    if(m_scoreString != "Results_NA")
    {
        ui->metricLabel->setText(m_metric);
        ui->metricLabel->show();
    }
    else
    {
        ui->metricLabel->hide();
    }
}

void CompareWidget::setMaxScore(double maxScore)
{
    m_maxScore = maxScore;
    (m_score > 0) ? m_percent = m_score/m_maxScore : m_percent = 0.0;
}

void CompareWidget::setDeviceId(int deviceId)
{
    m_deviceId = deviceId;
}

void CompareWidget::setToNull()
{
    setScore(-1.0, -1.0);
    setMetric(QString::null);
    hide();
}

void CompareWidget::setWidgetIndex(int index)
{
    m_widgetIndex = index;
    setDevice(m_deviceName, m_manufacturer);
}

void CompareWidget::setDecimals(int decimals)
{
    s_decimals = decimals;
}

void CompareWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(property("barBackgroundColor").toString()));
    painter.fillRect(QRect(0, 0, qRound(width()*m_percent), height()), QColor(m_barColor));
}

void CompareWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        m_pressed = true;
    }
}

void CompareWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        if(m_pressed)
        {
            emit clicked();
            m_pressed = false;
        }
    }
}

void CompareWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::Leave)
    {
        m_pressed = false;
    }
}
