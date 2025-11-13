/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ResultsHistoryItemWidget.h"
#include "ui_ResultsHistoryItemWidget.h"

#include <QDebug>
#include <QMouseEvent>

ResultsHistoryItemWidget::ResultsHistoryItemWidget(const QString &majorText, const QString &minorText, QWidget *parent) :
	SelectableWidget(parent),
	ui(new Ui::ResultsHistoryItemWidget)
{
	ui->setupUi(this); 

	ui->majorLabel->setText(majorText);
	ui->minorLabel->setText(minorText);
	if(minorText.isEmpty())
    {
		ui->minorLabel->hide();
        ui->majorLabel->setAlignment(Qt::AlignVCenter);
    }
    ui->arrowLabel->hide();
    
    connect(this, SIGNAL(selected()), this, SLOT(onSelected()));
    connect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
}

ResultsHistoryItemWidget::~ResultsHistoryItemWidget()
{
    delete ui;
}

void ResultsHistoryItemWidget::setMajorBold(bool enable)
{
    if(enable)
    {
        ui->majorLabel->setStyleSheet("QLabel#majorLabel { font-weight: bold; } ");
    }
    else
    {
        ui->majorLabel->setStyleSheet("QLabel#majorLabel { font-weight: normal; } ");
    }
}

QString ResultsHistoryItemWidget::majorText()
{
    if(ui->majorLabel)
    {
        return ui->majorLabel->text();
    }
    return "";
}

QString ResultsHistoryItemWidget::minorText()
{
    if(ui->minorLabel)
    {
        return ui->minorLabel->text();
    }
    return "";
}

void ResultsHistoryItemWidget::setSelectable(bool set)
{
	if(set)
	{
	    connect(this, SIGNAL(selected()), this, SLOT(onSelected()));
		connect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
	}
	else
	{
		disconnect(this, SIGNAL(selected()), this, SLOT(onSelected()));
		disconnect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
	}
}

void ResultsHistoryItemWidget::showOpenArrow(bool show)
{
	ui->arrowLabel->setVisible(show);
}

void ResultsHistoryItemWidget::onSelected()
{
    QString ss = QString("QWidget { background-color: %1; }").arg(property("selectColor").toString());
    ui->containerWidget->setStyleSheet(ss);
}

void ResultsHistoryItemWidget::onDeselected()
{
    QString ss = QString("QWidget { background-color: %1; }").arg(property("unselectColor").toString());
    ui->containerWidget->setStyleSheet(ss);
}
