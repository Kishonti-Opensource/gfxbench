/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "DescriptionWidget.h"
#include "ui_DescriptionWidget.h"
#include "Dictionary.h"
#include <QScrollBar>
#include <QDebug>



DescriptionWidget::DescriptionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DescriptionWidget)
{
    ui->setupUi(this);
    connect(ui->descriptionCloseButton, SIGNAL(clicked()), this, SIGNAL(closeClicked()));
    localize();
}

DescriptionWidget::~DescriptionWidget()
{
    delete ui;
}

void DescriptionWidget::localize()
{
    ui->descriptionCloseButton->setText(QtUI::dict("Close"));
}

void DescriptionWidget::setPixmap(const QPixmap &pixmap)
{
    ui->testPictureLabel->setPixmap(pixmap);
}

void DescriptionWidget::setTitle(const QString &title)
{
    m_title = title;
    ui->descriptionTitleLabel->setText(QtUI::dict(m_title));
}

void DescriptionWidget::setImagePath(const QString& imagePath)
{
    QString ss = QString("QLabel#testPictureLabel { border-image: url(%1) 0 0 0 0 stretch stretch; }").arg(imagePath);
    ui->testPictureLabel->setStyleSheet(ss);
}

void DescriptionWidget::setDescription(const QString &description, const QStringList &args)
{
    m_description = description;
    m_args = args;
    ui->descriptionLabel->setText(QtUI::dict(m_description, m_args));
}
