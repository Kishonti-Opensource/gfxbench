/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "LoadingScreen.h"
#include "ui_LoadingScreen.h"

#include "Dictionary.h"

using namespace QtUI;



LoadingScreen::LoadingScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadingScreen)
{
    ui->setupUi(this);
    ui->contentWidget->setStyleSheet("QWidget#contentWidget { border-image: none; }");
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->testNameLabel->setText("");
    ui->descriptionLabel->setText("");
}



LoadingScreen::~LoadingScreen()
{
    delete ui;
}


void LoadingScreen::setTestItem(const QString& testId, const QString& imagePath)
{
    QString css = "QWidget#contentWidget{border-image: url(" + imagePath +
            ") 0 0 0 0 stretch stretch;}";
    ui->contentWidget->setStyleSheet(css);
    ui->testNameLabel->setText(dict(testId));
    if (testId.endsWith("_off")) {
        QStringList arguments;
        arguments << dict(testId.mid(0, testId.size() - 4));
        ui->descriptionLabel->setText(dict("TestLoading", arguments));
    } else {
        ui->descriptionLabel->setText("");
    }
    
    ui->loadingTextLabel->setText(dict("IsLoading").arg(QApplication::applicationName()));
}
