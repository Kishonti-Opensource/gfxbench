/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "InfoPage.h"

#include "InfoWidget.h"
#include "InfoDetailWidget.h"
#include "Dictionary.h"

#include <cassert>



using namespace QtUI;



InfoPage::InfoPage(QWidget *parent):
    QWidget(parent),
    mInfoShown(nullptr),
    mBenchmarkService(nullptr)
{
	ui.setupUi(this);
    ui.detailPanel->hide();

    const bool success = QObject::connect(
            ui.detailCloseButton, &QPushButton::clicked,
            this, &InfoPage::onDetailCloseButtonClicked);
    assert(success);

    localize();

    Q_UNUSED(success)
}



void InfoPage::setBenchmarkService(BenchmarkService* benchmarkService)
{
    mBenchmarkService = benchmarkService;
}



void InfoPage::setSystemInfoListModel(CursorListModel* systemInfoListModel)
{
    mSystemInfoListModel = systemInfoListModel;

    bool success = QObject::connect(systemInfoListModel, &CursorListModel::modelReset, this, &InfoPage::updateSystemInfo);
    assert(success);

    success = QObject::connect(systemInfoListModel, &CursorListModel::dataChanged, this, &InfoPage::updateSystemInfo);
    assert(success);

    success = QObject::connect(&mAttributeListModel, &CursorListModel::modelReset, this, &InfoPage::updateAttributes);
    assert(success);

    success = QObject::connect(&mAttributeListModel, &CursorListModel::dataChanged, this, &InfoPage::updateAttributes);
    assert(success);

    Q_UNUSED(success)

    updateSystemInfo();
}



void InfoPage::localize()
{
    ui.sectionTitleLabel->setText(dict("TabInfo"));
    ui.detailCloseButton->setText(dict("Close"));
    QString appNamelower = QApplication::applicationName().toLower();
    if(appNamelower.contains("gfxbench"))
    {
        ui.detailTitleLabel->setText(dict("TabGlInfo"));
    }
    else if(appNamelower.contains("compubench"))
    {
        ui.detailTitleLabel->setText(dict("TabClInfo"));
    }

    for(int i(0); i < ui.systemInfoScrollWidget->count(); i++)
    {
        InfoWidget *w = (InfoWidget*) ui.systemInfoScrollWidget->widgetAt(i);
        w->localize();
    }
}



void InfoPage::onInfoClicked()
{
    InfoWidget* info = (InfoWidget*)sender();
    if (info == mInfoShown) {
        ui.detailPanel->setVisible(!ui.systemInfoDetailsScrollWidget->isVisible());
        return;
    }

    auto cursor = mBenchmarkService->getSystemInfoAttributes(info->modelIndex().data(Kishonti::RowIdRole).toLongLong());
    mAttributeListModel.setCursor(cursor);
    ui.detailTitleLabel->setText(info->modelIndex().data(Qt::DisplayRole).toString());
    mInfoShown = info;
    if(!ui.detailPanel->isVisible()) {
        ui.detailPanel->setVisible(!ui.systemInfoDetailsScrollWidget->isVisible());
    }
}



void InfoPage::onDetailCloseButtonClicked()
{
    ui.detailPanel->setVisible(!ui.systemInfoDetailsScrollWidget->isVisible());
}



void InfoPage::updateSystemInfo()
{
    ui.systemInfoScrollWidget->blockSignals(true);
    ui.systemInfoScrollWidget->clear();

    for (int i = 0; i < mSystemInfoListModel->rowCount(); ++i) {
        QModelIndex index = mSystemInfoListModel->index(i);
        InfoWidget *infoWidget = new InfoWidget(
            index.data(Qt::DisplayRole).toString(),
            qvariant_cast<QIcon>(index.data(Qt::DecorationRole)),
            index.data(Kishonti::MajorRole).toString(),
            index.data(Kishonti::MinorRole).toString(),
            this);
        infoWidget->setModelIndex(index);
        bool success = QObject::connect(infoWidget, &InfoWidget::clicked,
            this, &InfoPage::onInfoClicked);
        assert(success);
        Q_UNUSED(success);

        infoWidget->showOpenArrow(index.flags().testFlag(Qt::ItemIsEnabled));

        if (!index.flags().testFlag(Qt::ItemIsEnabled) &&
            !index.data(Kishonti::AttributesRole).toString().isEmpty())
        {
            infoWidget->setGadgets(mBenchmarkService->getSystemInfoAttributes(
                    index.data(Kishonti::RowIdRole).toLongLong()));
        }
        ui.systemInfoScrollWidget->addWidget(infoWidget);
    }

    ui.systemInfoScrollWidget->blockSignals(false);
}



void InfoPage::updateAttributes()
{
    ui.systemInfoDetailsScrollWidget->clear();
    for (int i = 0; i < mAttributeListModel.rowCount(); ++i) {
        QModelIndex index = mAttributeListModel.index(i);
        InfoDetailWidget *detail = InfoDetailWidget::createDetail(
                index.data(Qt::DisplayRole).toString(),
                index.data(Kishonti::ValueRole).toString(),
                ui.systemInfoDetailsScrollWidget);
        ui.systemInfoDetailsScrollWidget->addWidget(detail);
    }
}
