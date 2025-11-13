/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "GroupWidget.h"
#include "ui_GroupWidget.h"
#include "TestWidget.h"
#include <QMouseEvent>
#include "Dictionary.h"

GroupWidget::GroupWidget(const QString &text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupWidget),
    checked_(true),
    m_title(text)
{
    ui->setupUi(this);
    
    m_devicePixelRatio = 1.0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_devicePixelRatio = devicePixelRatio();
#endif

    if(m_devicePixelRatio > 1.0)
    {
        ui->checkLabel->setStyleSheet(getCheckCSS(checked_));
    }
    setChecked(true);
    pressed_ = 0;
    
    localize();
}

GroupWidget::~GroupWidget()
{
    delete ui;
}

GroupWidget *GroupWidget::createSelectWidgetWithOnOffscreen(const QString &title, QWidget *parent)
{
	GroupWidget *w = new GroupWidget(title, parent);
	w->showOnscreenOffscreen(true);
	w->showCheckBox(true);
	return w;
}

GroupWidget *GroupWidget::createSelectWidget(const QString &title, QWidget *parent)
{
	GroupWidget *w = new GroupWidget(title, parent);
	w->showOnscreenOffscreen(false);
	w->showCheckBox(true);
	return w;
}

GroupWidget *GroupWidget::createResultWidgetWithOnOffscreen(const QString &title, QWidget *parent)
{
	GroupWidget *w = new GroupWidget(title, parent);
	w->showOnscreenOffscreen(true);
	w->showCheckBox(false);
	return w;
}

GroupWidget *GroupWidget::createResultWidget(const QString &title, QWidget *parent)
{
	GroupWidget *w = new GroupWidget(title, parent);
	w->showOnscreenOffscreen(false);
	w->showCheckBox(false);
	return w;
}

void GroupWidget::localize()
{
    ui->groupLabel->setText(QtUI::dict(m_title));
    ui->onscreenLabel->setText(QtUI::dict("Onscreen"));
    ui->offscreenLabel->setText(QtUI::dict("Offscreen"));
}

void GroupWidget::showOnscreenOffscreen(bool show)
{
	ui->onOffscreenWidget->setVisible(show);
	ui->verticalLine0->setVisible(show);
}

void GroupWidget::showCheckBox(bool show)
{
	ui->checkLabel->setVisible(show);
}

void GroupWidget::setChecked(bool checked)
{
    checked_ = checked;
    ui->checkLabel->setStyleSheet(getCheckCSS(checked_));
}
void GroupWidget::setEnabled(bool enable)
{
    QWidget::setEnabled(enable);
}

void GroupWidget::setDisabled(bool disable)
{
    setEnabled(!disable);
}

bool GroupWidget::isChecked()
{
    return checked_;
}

QString GroupWidget::title()
{
    return m_title;
}

void GroupWidget::click()
{
    if(ui->checkLabel->isVisible())
	{
		setChecked(!checked_);
		emit toggled(checked_);
	}
	emit clicked();
}

void GroupWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
		pressed_ = this;
    }
}

void GroupWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        if(pressed_ == this)
        {
            this->click();
        }
        pressed_ = 0;
    }
}

QString GroupWidget::getCheckCSS(bool checked)
{
    QString css = "QLabel#checkLabel { border-image: url(:/%1) 0 0 0 0 stretch stretch; }";
    if(m_devicePixelRatio > 1.0)
    {
        if(checked)
            return css.arg("retina_group_checkbox_check.png");
        else
            return css.arg("retina_group_checkbox_uncheck.png");
    }
    else
    {
        if(checked)
            return css.arg("group_checkbox_check.png");
        else
            return css.arg("group_checkbox_uncheck.png");
    }
}