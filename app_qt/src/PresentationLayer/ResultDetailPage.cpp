/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ResultDetailPage.h"
#include "ui_ResultDetailPage.h"

#include "ChartWidget.h"
#include "Dictionary.h"

#include "benchmarkservice.h"



ResultDetailPage::ResultDetailPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultDetailPage),
    mBenchmarkService(nullptr)
{
    ui->setupUi(this);

    bool success = connect(
            ui->closeButton, &QPushButton::clicked,
            this, &ResultDetailPage::closeClicked);
    assert(success);
    Q_UNUSED(success)

    localize();
}



ResultDetailPage::~ResultDetailPage()
{
    delete ui;
}



void ResultDetailPage::localize()
{
    ui->titleLabel->setText(QtUI::dict("TabResults"));
    ui->closeButton->setText(QtUI::dict("Close"));
}



void ResultDetailPage::setBenchmarkService(BenchmarkService* benchmarkService)
{
    mBenchmarkService = benchmarkService;
}



void ResultDetailPage::setTitle(const QString& title)
{
    ui->resultNameLabel->setText(QtUI::dict(title));
}



void ResultDetailPage::setResultRowId(long long rowId)
{
    mResultRowId = rowId;
}



void ResultDetailPage::setModel(CursorListModel* model)
{
    mModel = model;

    bool success = QObject::connect(model, &CursorListModel::modelReset,
            this, &ResultDetailPage::update);
    assert(success);

    success = QObject::connect(model, &CursorListModel::dataChanged,
            this, &ResultDetailPage::update);
    assert(success);

    Q_UNUSED(success)
}



void ResultDetailPage::update()
{
    QLayoutItem* child;
    while ((child = ui->detailsLayout->takeAt(1)) != 0) {
        delete child->widget();
        delete child;
    }
    while ((child = ui->chartLayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    for (int i = 0; i < mModel->rowCount(); ++i) {
        QModelIndex index = mModel->index(i);
        if (index.data(Kishonti::MajorRole).isValid()) {
            QLabel* label = new QLabel(this);
            label->setText(QString("%1: <FONT color=\"#4699B8\">%2 %3</FONT>").
                    arg(QtUI::dict(index.data(Qt::DisplayRole).toString())).
                    arg(index.data(Kishonti::MajorRole).toString()).
                    arg(index.data(Kishonti::MinorRole).toString()));
            ui->detailsLayout->addWidget(label);
        } else {
            ChartWidget* widget = new ChartWidget(this);
            widget->setCursor(mBenchmarkService->getChart(
                    mResultRowId, index.data(Kishonti::RowIdRole).toLongLong()));
            ui->chartLayout->addWidget(widget);
        }
    }
}
