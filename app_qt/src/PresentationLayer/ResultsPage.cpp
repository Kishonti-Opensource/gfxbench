/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ResultsPage.h"

#include "Dictionary.h"
#include "ResultDetailPage.h"
#include "ResultsHistoryItemWidget.h"

#include "benchmarkservice.h"

#include <QDateTime>



using namespace QtUI;



ResultsPage::ResultsPage(QWidget* parent):
    QWidget(parent),
    mBenchmarkService(0),
    mShowLatest(false)
{
    bool success;

	ui.setupUi(this);
    ui.subresultsPanel->hide();
    ui.descriptionPanel->hide();
    ui.diagramPanel->hide();
    ui.resultsScrollWidget->setModel(&mGroupProxyModel);

    success = connect(ui.resultsHistoryScrollWidget,
            &SelectableScrollWidget::currentItemChanged,
            this, &ResultsPage::onResultListCurrentItemChanged);
    assert(success);
    
    success = connect(
            ui.diagramPanel, &ResultDetailPage::closeClicked,
            this, &ResultsPage::resetPanels);
    assert(success);
    
    success = connect(
            ui.descriptionPanel, &DescriptionWidget::closeClicked,
            this, &ResultsPage::resetPanels);
    assert(success);

    success = connect(
            ui.subresultsCloseButton, &QPushButton::clicked,
            this, &ResultsPage::resetPanels);
    assert(success);

    success = connect(
            ui.resultsScrollWidget, &ItemScrollWidget::resultClicked,
            this, &ResultsPage::onResultClicked);
    assert(success);

    success = connect(
            ui.resultsScrollWidget, &ItemScrollWidget::descriptionClicked,
            this, &ResultsPage::onDescriptionClicked);
    assert(success);
    
    localize();

    Q_UNUSED(success)
}



void ResultsPage::setBenchmarkService(BenchmarkService* benchmarkService)
{
    mBenchmarkService = benchmarkService;
    ui.diagramPanel->setBenchmarkService(benchmarkService);
}



void ResultsPage::setSessionListModel(CursorListModel* sessionListModel)
{
    mSessionListModel = sessionListModel;
    bool success = QObject::connect(sessionListModel, &CursorListModel::modelReset,
            this, &ResultsPage::updateSessions);
    assert(success);
    Q_UNUSED(success)
    updateSessions();
}



void ResultsPage::setResultListModel(CursorListModel* resultListModel)
{
    mResultListModel = resultListModel;
    mGroupProxyModel.setSourceModel(resultListModel);
}



void ResultsPage::setResultDetailListModel(CursorListModel* resultDetailListModel)
{
    mResultDetailListModel = resultDetailListModel;
    ui.diagramPanel->setModel(mResultDetailListModel);
}



void ResultsPage::showLatest()
{
    mShowLatest = true;
}



void ResultsPage::localize()
{
    ui.sectionTitleLabel->setText(dict("TabResults"));
    ui.subresultsCloseButton->setText(dict("Close"));
    ui.descriptionPanel->localize();
    ui.diagramPanel->localize();
}



void ResultsPage::showEvent(QShowEvent*)
{
    resetPanels();
}



void ResultsPage::onResultListCurrentItemChanged(SelectableWidget *current, SelectableWidget*)
{
    ResultsHistoryItemWidget* widget = static_cast<ResultsHistoryItemWidget*>(current);
    long long sessionId = widget->modelIndex().data(Kishonti::RowIdRole).toLongLong();
    mGroupProxyModel.setHideNA(sessionId != -1);
    mResultListModel->setCursor(mBenchmarkService->getResultForSession(sessionId));
}



void ResultsPage::onResultClicked(const QModelIndex& modelIndex)
{
    QModelIndex sourceIndex = mGroupProxyModel.sourceIndex(modelIndex);
    mResultDetailListModel->setCursor(mBenchmarkService->getResultDetails(
            sourceIndex.data(Kishonti::RowIdRole).toLongLong()));
    ui.diagramPanel->setTitle(sourceIndex.data(Qt::DisplayRole).toString());
    ui.diagramPanel->setResultRowId(sourceIndex.data(Kishonti::RowIdRole).toLongLong());
    showPanel(ui.diagramPanel);
}



void ResultsPage::onDescriptionClicked(const QModelIndex& modelIndex)
{
    if (modelIndex != mDescriptionIndex) {
        ui.descriptionPanel->setTitle(
                modelIndex.data(Qt::DisplayRole).toString());
        ui.descriptionPanel->setDescription(
                modelIndex.data(Qt::ToolTipRole).toString());
        ui.descriptionPanel->setImagePath(
                mResultListModel->data(mGroupProxyModel.sourceIndex(modelIndex),
                Kishonti::ImagePathRole).toString().replace(".png", "_full.png")); // TODO
        mDescriptionIndex = modelIndex;
        showPanel(ui.descriptionPanel);
    } else {
        showPanel(ui.resultsHistoryScrollWidget);
    }
}



void ResultsPage::resetPanels()
{
    showPanel(ui.resultsHistoryScrollWidget);
}



void ResultsPage::showPanel(QWidget* panel)
{
    ui.resultsPanel->setVisible(panel != ui.diagramPanel);
    ui.subresultsPanel->setVisible(false);
    ui.resultsHistoryScrollWidget->setVisible(false);
    ui.descriptionPanel->setVisible(false);
    ui.diagramPanel->setVisible(false);
    panel->setVisible(true);
    if (!ui.descriptionPanel->isVisible()) {
        mDescriptionIndex = QModelIndex();
    }
}



void ResultsPage::updateSessions()
{
    ui.resultsHistoryScrollWidget->blockSignals(true);
    ui.resultsHistoryScrollWidget->clear();

    for (int i = 0; i < mSessionListModel->rowCount(); ++i) {
        QModelIndex index = mSessionListModel->index(i);
        ResultsHistoryItemWidget *item = new ResultsHistoryItemWidget(
                QtUI::dict(index.data(Qt::DisplayRole).toString()),
                index.data(Qt::ToolTipRole).toString(),
                ui.resultsHistoryScrollWidget);
        item->setMajorBold(true);
        item->setModelIndex(index);
        ui.resultsHistoryScrollWidget->addWidget(item);
    }

    ui.resultsHistoryScrollWidget->blockSignals(false);

    if (mShowLatest && (ui.resultsHistoryScrollWidget->count() > 1)) {
        ui.resultsHistoryScrollWidget->setCurrentRow(1);
    }
    mShowLatest = false;
}
