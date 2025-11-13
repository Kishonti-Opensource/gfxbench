/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ComparePage.h"

#include "DeviceWidget.h"
#include "Dictionary.h"
#include "SelectableScrollWidget.h"
#include "CompareWidget.h"
#include "TestWidget.h"
#include "GroupWidget.h"
#include "DuelWidget.h"

#include "benchmarkservice.h"

#include <QBoxLayout>
#include <QResizeEvent>



using namespace QtUI;



ComparePage::ComparePage(QWidget *parent):
        QWidget(parent),
        mBenchmarkService(nullptr),
        mYourDeviceWidget(nullptr)
{
	ui.setupUi(this);

    if (devicePixelRatio() > 1.0) {
        QString css = "QLabel#loupeLabel \
        { \
            border-image: url(:/retina_loupe.png) 0 0 0 0 stretch stretch; \
        }";
        ui.loupeLabel->setStyleSheet(css);
    }

    connect(ui.testListWidget, &SelectableScrollWidget::currentItemChanged, this, &ComparePage::onTestItemChanged);
    connect(ui.searchLineEdit, &QLineEdit::textEdited, this, &ComparePage::onSearchFilter);
	connect(ui.compareScrollWidget, &ScrollWidget::scrolled, this, &ComparePage::onScrolled);

    //removed it because lead to an ugly UI glitch
    //connect(ui.compareScrollWidget, &ScrollWidget::resized, this, &ComparePage::onCompareScrollWidgetResized);
    connect(ui.duelSectionCloseButton, &QPushButton::clicked, this, &ComparePage::onDuelCloseClicked);

	ui.duelPanel->hide();
    mYourDeviceWidget = CompareWidget::createCompareWidget("YourDevice", QString::null, -1, 0, -1, QString::null, QIcon(":/retina_device.png"), nullptr);

    ((QHBoxLayout*)ui.compareListPanel->layout())->insertWidget(0, mYourDeviceWidget);

    mYourDeviceWidget->show();
    ui.searchLineEdit->setPlaceholderText("");

    localize();
}



void ComparePage::setBenchmarkService(BenchmarkService *benchmarkService)
{
    mBenchmarkService = benchmarkService;
}



void ComparePage::setResultListModel(CursorListModel* resultListModel)
{
    mResultListModel = resultListModel;
    connect(mResultListModel, &QAbstractListModel::dataChanged, this, &ComparePage::onResultListUpdated);
    connect(mResultListModel, &QAbstractListModel::modelReset, this, &ComparePage::onResultListUpdated);
}



void ComparePage::setCompareListModel(CursorListModel* compareListModel)
{
    mCompareListModel = compareListModel;
    connect(mCompareListModel, &QAbstractListModel::dataChanged, this, &ComparePage::onCompareListUpdated);
    connect(mCompareListModel, &QAbstractListModel::modelReset, this, &ComparePage::onCompareListUpdated);
}



void ComparePage::setDuelListModel(CursorListModel* duelListModel)
{
    mDuelListModel = duelListModel;
    connect(mDuelListModel, &QAbstractListModel::dataChanged, this, &ComparePage::onDuelListUpdated);
    connect(mDuelListModel, &QAbstractListModel::modelReset, this, &ComparePage::onDuelListUpdated);
}



void ComparePage::localize()
{
    ui.sectionTitleLabel->setText(dict("TabCompare"));
    ui.yourDeviceLabel->setText(dict(ui.yourDeviceLabel->text()));
    ui.searchLineEdit->setPlaceholderText(dict("Search"));
    mYourDeviceWidget->localize();
    //ui.compareScrollWidget->localize();
    //ui.testListWidget->localize();
}



void ComparePage::onResultListUpdated()
{
    ui.testListWidget->clear();
    for (int i = 0; i < mResultListModel->rowCount(); ++i) {
        QModelIndex index = mResultListModel->index(i);
        TestWidget *testWidget = TestWidget::createSelectableWidget(
                index.data(Qt::DisplayRole).toString(),
                index.data(Qt::ToolTipRole).toString(),
                QStringList(),
                ui.testListWidget);
        testWidget->setModelIndex(index);
        testWidget->setSubresultId(0);
        testWidget->setIcon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));
        ui.testListWidget->addWidget(testWidget);
    }
    ui.testListWidget->setCurrentRow(0);
}



void ComparePage::onCompareListUpdated()
{
    ui.compareScrollWidget->clear();
    addWidgetsBack();
}



void ComparePage::onDuelListUpdated()
{
    ui.duelScrollWidget->clear();
    for (int i = 0; i < mDuelListModel->rowCount(); ++i) {
        QModelIndex index = mDuelListModel->index(i);
        DuelWidget *duel = DuelWidget::createDuelWidget(
                index.data(Qt::DisplayRole).toString(),
                qvariant_cast<QIcon>(index.data(Qt::DecorationRole)),
                index.data(Kishonti::ScoreARole).toDouble(),
                0.0,
                index.data(Kishonti::ScoreBRole).toDouble(),
                0.0,
                index.data(Kishonti::PrimaryUnitRole).toString(),
                ui.duelScrollWidget);
        ui.duelScrollWidget->addWidget(duel);
    }
}



void ComparePage::onDeviceClicked()
{
    CompareWidget *w = (CompareWidget*)sender();
    std::shared_ptr<Cursor> cursor = mBenchmarkService->getDuelResults(
            w->modelIndex().data(Kishonti::ApiRole).toString().toUtf8(),
            "own",
            w->modelIndex().data(Kishonti::ApiRole).toString().toUtf8(),
            w->modelIndex().data(Qt::DisplayRole).toString().toUtf8());
    mDuelListModel->setCursor(cursor);
    showDuel();
}



void ComparePage::onTestItemChanged(SelectableWidget *current, SelectableWidget*)
{
    ui.testListWidget->setVisible(false);

    TestWidget* w = (TestWidget*) current;

    mSelectedResult = w->modelIndex().data(Qt::DisplayRole).toString().toStdString();
    std::shared_ptr<Cursor> cursor = mBenchmarkService->getCompareResults(mSelectedResult.c_str(), mFilter.c_str());
    mCompareListModel->setCursor(cursor);

    mYourDeviceWidget->setDevice(w->modelIndex().data(Kishonti::DeviceNameRole).toString(), "");
    mUnit = w->modelIndex().data(Kishonti::PrimaryUnitRole).toString();
    mYourDeviceWidget->setScore(w->modelIndex().data(Kishonti::PrimaryScoreRole).toDouble(), w->modelIndex().data(Kishonti::SecondaryScoreRole).toDouble());
    mYourDeviceWidget->setMetric(mUnit);
    if (mCompareListModel->rowCount() > 0) {
        mYourDeviceWidget->setMaxScore(0);
    }

    ui.testListWidget->setVisible(true);
}



void ComparePage::onSearchFilter(const QString &filter)
{
    mFilter = filter.toStdString();
    std::shared_ptr<Cursor> cursor = mBenchmarkService->getCompareResults(mSelectedResult.c_str(), mFilter.c_str());
    mCompareListModel->setCursor(cursor);
}



void ComparePage::onDuelCloseClicked()
{
    showTestList();
}



void ComparePage::onScrolled(double value)
{
    if(value > 0.9)
    {
        addWidgetsBack();
    }
}



void ComparePage::showTestList()
{
    ui.testListWidget->setVisible(true);
    ui.duelPanel->setVisible(false);
}



void ComparePage::showDuel()
{
    ui.testListWidget->setVisible(false);
    ui.duelPanel->setVisible(true);
}



void ComparePage::onCompareScrollWidgetResized(QSize)
{
#ifdef Q_OS_WIN
	int scrollBarWidth = 0;
	if(ui.compareScrollWidget->containedHeightSum() > ui.compareScrollWidget->height()) {
		scrollBarWidth = 26;
	}
	mYourDeviceWidget->resize(ui.compareScrollWidget->size() - QSize(scrollBarWidth, 0));
#else
    mYourDeviceWidget->resize(ui.compareScrollWidget->size());
#endif
}



void ComparePage::addWidgetsBack()
{
    mYourDeviceWidget->hide();

    int last = std::min(ui.compareScrollWidget->count() + 25, mCompareListModel->rowCount());
    double maxScore = 1000.0;
    if (mCompareListModel->rowCount() > 0) {
        maxScore = mCompareListModel->index(0).data(Kishonti::PrimaryScoreRole).toDouble();
        mYourDeviceWidget->setMaxScore(maxScore);
    }

    for (int i = ui.compareScrollWidget->count(); i < last; ++i) {
        QModelIndex index = mCompareListModel->index(i);
        CompareWidget *cw = CompareWidget::createCompareWidget(
                index.data(Qt::DisplayRole).toString(),
                index.data(Kishonti::ApiRole).toString(),
                index.data(Kishonti::PrimaryScoreRole).toDouble(),
                maxScore,
                index.data(Kishonti::SecondaryScoreRole).toDouble(),
                mUnit,
                qvariant_cast<QIcon>(index.data(Qt::DecorationRole)),
                ui.compareScrollWidget);
        cw->setModelIndex(index);
        connect(cw, &CompareWidget::clicked, this, &ComparePage::onDeviceClicked);
        ui.compareScrollWidget->addWidget(cw);
    }
    mYourDeviceWidget->show();
}
