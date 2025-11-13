/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ApiSelector.h"
#include "ApiModel.h"
#include "ui_ApiSelector.h"
#include "DeviceWidget.h"

#include <MainWindow.h>

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//							APISELECTOR CLASS
//
//////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------------
	Constructor / destructor
---------------------------------------------------------------------------------------*/

ApiSelector::ApiSelector(BenchmarkService* benchMarkService, QtUI::MainWindow *mainWindow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApiSelector),
	m_BenchMarkService(benchMarkService),
	m_MainWindow(mainWindow)
{
    ui->setupUi(this);

	// setup layout
	QLabel *apiLabel = new QLabel("Select API");
	QLabel *deviceLabel = new QLabel("Select Device");

	apiLabel->setStyleSheet("color: #727272; font-size: 15px; font-weight: bold;");
	deviceLabel->setStyleSheet("color: #727272; font-size: 15px; font-weight: bold;");

	m_scrollWidget = new DeviceSelectorScrollWidget(this);
	m_apiscrollWidget = new DeviceSelectorScrollWidget(this);

	layout()->addWidget(apiLabel);
	layout()->addWidget(m_apiscrollWidget);
	layout()->addWidget(deviceLabel);
	layout()->addWidget(m_scrollWidget);

	setFixedWidth(700);
    
    m_fixRenderApi = tfw::ApiDefinition::NOT_DEFINED;
}

ApiSelector::~ApiSelector()
{
    delete ui;
}

/*--------------------------------------------------------------------
	Public functions
--------------------------------------------------------------------*/

void ApiSelector::onShow()
{
	QWidget *parent = dynamic_cast<QWidget*>(this->parent());
	if (parent)
	{
		setFixedWidth(700);
		parent->setFixedWidth(700);
	}
	this->show();
}

void ApiSelector::setModel(QAbstractItemModel *model)
{
	m_scrollWidget->setModel(model);

	QWidget *parent = dynamic_cast<QWidget*>(this->parent());
	if (parent)
	{
		m_origParentGeometry = parent->geometry();
		parent->setFixedWidth(700);
	}
	
	m_scrollWidget->setApiCollectedCallback(this);
	connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
	connect(model, SIGNAL(modelReset()), this, SLOT(modelReset()));
}

QString ApiSelector::getApiAndDevice()
{
	return "";
}

/*--------------------------------------------------------------------
Public functions :: QWidget overrides 
--------------------------------------------------------------------*/
void ApiSelector::closeEvent(QCloseEvent* event)
{
	setFixedWidth(10);
	this->hide();
	QWidget *parent = dynamic_cast<QWidget*>(this->parent());
	if (parent)
	{
		parent->setFixedWidth(m_origParentGeometry.width());
	}
	QWidget::closeEvent(event);
}

void ApiSelector::dataChanged(const QModelIndex &topleft, const QModelIndex &bottomRight)
{
	
}

/*------------------------------------------------------------------------------------
	Public slots
------------------------------------------------------------------------------------*/

void ApiSelector::modelReset()
{
	
}

void ApiSelector::setFixRenderApi(tfw::ApiDefinition::Type renderApi)
{
	m_fixRenderApi = renderApi;
}

void ApiSelector::onApiSelected(const QModelIndex&, const QModelIndex&)
{
	tfw::ApiDefinition::Type selectedApi =  m_apimodel->selectedApi();
	selectApi(selectedApi);
}

void ApiSelector::delayedRestart()
{
	m_timer.stop();
	disconnect(&m_timer);
	selectApi(m_apimodel->selectedApi());
}

/*---------------------------------------------------------------------------------------
	Private functions
---------------------------------------------------------------------------------------*/

void ApiSelector::onApiCollected()
{
#ifdef COMPUBENCH
	auto apis = m_scrollWidget->allComputeApi();
	m_apimodel = new ApiModel(apis);
#else
	auto apis = m_scrollWidget->allRenderApi();
	m_apimodel = new ApiModel(apis);
#endif
	
	auto index = m_apimodel->index(0, 0);
	m_apimodel->setData(index, true, Qt::CheckStateRole);

	m_apiscrollWidget->setModel(m_apimodel);
	m_apiscrollWidget->modelReset();
	connect(m_apimodel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onApiSelected(const QModelIndex&, const QModelIndex&)));
	
	// delayed refresh
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(delayedRestart()));
	m_timer.start(20);
}


void ApiSelector::selectApi(tfw::ApiDefinition::Type api)
{
	m_BenchMarkService->stop();

	try
	{
#ifdef COMPUBENCH
	m_BenchMarkService->start(m_fixRenderApi, api);
#else
	m_BenchMarkService->start(api);
#endif
	}
	catch (...)
	{ }

	m_MainWindow->setBenchmarkService(m_BenchMarkService);
}
