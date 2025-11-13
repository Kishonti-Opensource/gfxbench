/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "HomePage.h"

#include "ConfigurationListModel.h"
#include "Dictionary.h"
#include "TestWidget.h"
#include "MessageBox.h"
#include "TestMode.h"
#include "TestListModel.h"

#include "benchmarkservice.h"

#include <cassert>



using namespace QtUI;



HomePage::HomePage(QWidget *parent):
    QWidget(parent),
    m_apiselector(nullptr), 
    mBenchmarkService(0),
    mTestListModel(0),
	m_apiSelectorShown(true)
{
	ui.setupUi(this);
    
    if (devicePixelRatio() > 1.0)
    {
        ui.startButton->setStyleSheet(getButtonCSS("start"));
        ui.selectButton->setStyleSheet(getButtonCSS("select"));
        QString logo = "QLabel#logoLabel{ border-image: url(:/retina_logo.png) 0 0 0 0 stretch stretch; }";
        ui.logoLabel->setStyleSheet(logo);
    }
	ui.testSelectionPanel->hide();
    ui.descriptionPanel->hide();
    ui.testSelectScrollWidget->setModel(&mConcatProxyModel);

    bool success = QObject::connect(
            ui.startButton, &QPushButton::clicked,
            this, &HomePage::onStartButtonClicked);
    assert(success);

    success = connect(
            ui.selectButton, &QPushButton::clicked,
            this, &HomePage::onTestSelectButtonClicked);
    assert(success);

    success = connect(
            ui.selectorCloseButton, &QPushButton::clicked,
            this, &HomePage::onTestSelectButtonClicked);
    assert(success);

    success = connect(
            ui.selectorStartButton, &QPushButton::clicked,
            this, &HomePage::onStartButtonClicked);
    assert(success);

    success = connect(
            ui.descriptionPanel, &DescriptionWidget::closeClicked,
            this, &HomePage::onCloseDescription);
    assert(success);

    success = QObject::connect(
            &mGroupProxyModel, &GroupProxyModel::setGroupData,
            this, &HomePage::onSetGroupData);
    assert(success);

    success = QObject::connect(
            ui.testSelectScrollWidget, &ItemScrollWidget::descriptionClicked,
            this, &HomePage::onDescriptionClicked);
    assert(success);

	Q_UNUSED(success)

#if defined(GFXBENCH_GL) || defined(GFXBENCH_VULKAN) || defined(GFXBENCH_METAL) || defined(COMPUBENCH_CL) || defined(COMPUBENCH_CU)
		ui.apiSelectButton->setVisible(false);
#endif

    localize();
}


void HomePage::setApiSelector(ApiSelector *apiselector)
{
	ui.buttonsWidget->hide();
	m_apiselector = apiselector;
	m_apiselector->setParent(ui.apiSelectorContent);
	ui.apiSelectorContent->layout()->addWidget(m_apiselector);
	m_apiselector->setModel(&mConcatProxyModel);
	m_apiselector->show();
	connect(ui.apiSelectButton, SIGNAL(clicked()), this, SLOT(onApiSelectorClicked()));
}

void HomePage::setBenchmarkService(BenchmarkService *benchmarkService)
{
    mBenchmarkService = benchmarkService;
    localize();
}



void HomePage::localize()
{
    if (mBenchmarkService != 0) {
        if (mBenchmarkService->getConfig(BenchmarkService::CORPORATE_VERSION) == "true") {
            ui.versionLabel->setText(dict("AppVersionCorporate").arg(
                    QApplication::applicationVersion()));
        } else {
            ui.versionLabel->setText(QApplication::applicationVersion());
        }
    };
    ui.startButton->setText(dict("StartAll"));
    ui.selectButton->setText(dict("SelectTests"));
    ui.selectorCloseButton->setText(dict("Close"));
    ui.selectorStartButton->setText(dict("Start"));
    ui.selectorTitleLabel->setText(dict("SelectTests"));
    ui.descriptionPanel->localize();
}



void HomePage::setTestListModel(TestListModel *testListModel)
{
    if (mTestListModel != 0) {
        QObject::disconnect(mTestListModel);
    }
	
    mTestListModel = testListModel;
    mGroupProxyModel.setSourceModel(testListModel);
    mConcatProxyModel.addSourceModel(&mGroupProxyModel);
	
    if (mTestListModel == 0) {
        return;
    }

    bool success = QObject::connect(
            mTestListModel, &TestListModel::runAloneSelected,
            this, &HomePage::onRunAloneSelected, Qt::QueuedConnection);
    assert(success);
    /*
    success = QObject::connect(
            mTestListModel, &TestListModel::configSelected,
            this, &HomePage::sendSelectedDeviceIndex);
    assert(success);
    */
    Q_UNUSED(success)
}



void HomePage::setConfigurationListModel(ConfigurationListModel* configurationListModel)
{
    mConfigurationListModel = configurationListModel;
    mConcatProxyModel.addSourceModel(configurationListModel);
}



void HomePage::onStartButtonClicked()
{
    if (ui.testSelectionPanel->isVisible()) {
        mBenchmarkService->runSelectedTests();
    } else {
        mBenchmarkService->runAllTests();
    }
}



void HomePage::onTestSelectButtonClicked()
{
	if(!ui.testSelectionPanel->isVisible()) {
        ui.startButton->setEnabled(ui.selectorStartButton->isEnabled());
        ui.startButton->setText(dict("Start"));
		ui.selectButton->setText(dict("Close"));
        ui.testSelectionPanel->show();
        ui.testSelectScrollWidget->show();
	} else {
        ui.startButton->setEnabled(true);
		ui.startButton->setText(dict("StartAll"));
        ui.selectButton->setText(dict("SelectTests"));
		ui.testSelectionPanel->hide();
		if(ui.descriptionPanel->isVisible())
		{
			ui.descriptionPanel->hide();
			ui.controlPanel->show();
		}
	}
}



void HomePage::onRunAloneSelected(const std::string& testId)
{
    bool isAccepted = popupMsgBox("StabilityTestTitle", "StabilityTestBody", "OK", "Cancel", this);
    if (isAccepted) {
        mTestListModel->selectRunAlone(testId);
    }
}



void HomePage::onDescriptionClicked(const QModelIndex& modelIndex)
{
    if (modelIndex != mDescriptionIndex) {
        ui.descriptionPanel->setTitle(modelIndex.data(Qt::DisplayRole).toString());
        ui.descriptionPanel->setDescription(modelIndex.data(Qt::ToolTipRole).toString());
        ui.descriptionPanel->setImagePath(modelIndex.data(Kishonti::ImagePathRole).toString().
            replace(".png", "_full.png"));
        mDescriptionIndex = modelIndex;
        if (!ui.descriptionPanel->isVisible()) {
            ui.descriptionPanel->setVisible(true);
            ui.controlPanel->setVisible(false);
        }
    } else {
        ui.descriptionPanel->setVisible(!ui.descriptionPanel->isVisible());
        ui.controlPanel->setVisible(!ui.controlPanel->isVisible());
    }
}



void HomePage::onCloseDescription()
{
    ui.descriptionPanel->setVisible(false);
    ui.controlPanel->setVisible(true);
}

void HomePage::onApiSelectorClicked()
{
	if (m_apiSelectorShown)
	{
		m_apiselector->close();
		ui.apiSelectorContent->hide();
		ui.buttonsWidget->show();
		ui.apiSelectButton->setText("Select API and device");
		m_apiSelectorShown = false;
	}
	else
	{
		ui.buttonsWidget->hide();
		m_apiselector->onShow();
		ui.apiSelectButton->setText("Ok");
		ui.apiSelectorContent->show();
		m_apiSelectorShown = true;
	}
}



void HomePage::onSetGroupData(
        const QModelIndex& index,
        const QVariant& value,
        int role)
{
    QString groupId = index.data(Kishonti::GroupRole).toString();
    int checkState = value.toInt();
    mBenchmarkService->setGroupSelection(groupId.toUtf8(), checkState == Qt::Checked);
}



QString HomePage::getButtonCSS(const QString &buttonName)
{
    QString css = "\
    QPushButton#<buttonName>Button \
    { \
        color: <color>; \
        font-weight: normal; \
        font-size: <font-size>; \
        border-image: url(:/retina_<buttonName>.png) 0 0 0 0 stretch stretch; \
    } \
    \
    QPushButton#<buttonName>Button:hover \
    { \
        border-image: url(:/retina_<buttonName>_hover.png) 0 0 0 0 stretch stretch; \
    } \
    \
    QPushButton#<buttonName>Button:pressed \
    { \
        border-image: url(:/retina_<buttonName>_pressed.png) 0 0 0 0 stretch stretch; \
    }";
    css.replace("<buttonName>", buttonName);
    if(buttonName == "start")
    {
        css.replace("<color>", "#000");
        css.replace("<font-size>", "28px");
    }
    else if(buttonName == "select")
    {
        css.replace("<color>", "#fff");
        css.replace("<font-size>", "20px");
    }
    return css;
}
