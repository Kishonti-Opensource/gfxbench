/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "InfoDetailWidget.h"
#include "ui_InfoDetailWidget.h"

InfoDetailWidget::InfoDetailWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoDetailWidget)
{
    ui->setupUi(this);
}

InfoDetailWidget::~InfoDetailWidget()
{
    delete ui;
}

InfoDetailWidget *InfoDetailWidget::createDetail(const QString &key, const QString &value, QWidget *parent)
{
    InfoDetailWidget *w = new InfoDetailWidget(parent);
    w->setKey(key);
    w->setValue(value);
    return w;
}

void InfoDetailWidget::setKey(const QString &text)
{
    ui->keyLabel->setText(text);
}

QString InfoDetailWidget::key() const
{
    return ui->keyLabel->text();
}

void InfoDetailWidget::setValue(const QString &text)
{
    ui->valueLabel->setText(text);
}

QString InfoDetailWidget::value() const
{
    return ui->valueLabel->text();
}

void InfoDetailWidget::setStretch(int left, int right)
{
    QBoxLayout *layout = (QBoxLayout*) ui->mainWidget->layout();
    layout->setStretch(0, left);
    layout->setStretch(2, right);
}