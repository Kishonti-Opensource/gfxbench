/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef APISELECTOR_H
#define APISELECTOR_H

#include <QWidget>
#include <QTimer>
#include <QAbstractItemModel>

#include <benchmarkservice.h>
#include "ItemScrollWidget.h"
#include "testfw.h"


namespace QtUI
{
	class MainWindow;
}

namespace Ui {
class ApiSelector;
}

class ApiModel;

class ApiSelector : public QWidget , public IDeviceSelectorCallBack
{
    Q_OBJECT

public:
    explicit ApiSelector(BenchmarkService* benchMarkService, QtUI::MainWindow *mainWindow, QWidget *parent = 0);
    ~ApiSelector();

	void setModel(QAbstractItemModel *model);
	void closeEvent(QCloseEvent* event) override;
	void setFixRenderApi(tfw::ApiDefinition::Type renderApi);
	void onShow();
	void onApiCollected() override;
	QString getApiAndDevice();

private slots:
	void dataChanged(const QModelIndex &topleft, const QModelIndex &bottomRight);
	void onApiSelected(const QModelIndex&, const QModelIndex&);
	void modelReset();
	void delayedRestart();

private:
	void selectApi(tfw::ApiDefinition::Type api);

	Ui::ApiSelector *ui;
	BenchmarkService *m_BenchMarkService;
	QtUI::MainWindow *m_MainWindow;
	DeviceSelectorScrollWidget *m_scrollWidget;
	DeviceSelectorScrollWidget *m_apiscrollWidget;
	ApiModel *m_apimodel;
	QRect m_origParentGeometry;
	QTimer m_timer;
	tfw::ApiDefinition::Type m_fixRenderApi;
};

#endif // APISELECTOR_H
