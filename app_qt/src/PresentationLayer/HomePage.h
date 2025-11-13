/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include "ui_HomePage.h"

#include "CursorListModel.h"
#include "GroupProxyModel.h"
#include "ConcatProxyModel.h"
#include "ApiSelector.h"


class ApplicationConfig;
class BenchmarkService;
class ConfigurationListModel;
class TestWidget;
class TestListModel;



namespace QtUI
{	

	class HomePage : public QWidget
	{
		Q_OBJECT
	public:
		HomePage(QWidget *parent = 0);
        void setBenchmarkService(BenchmarkService *benchmarkService);
        void setTestListModel(TestListModel *testListModel);
        void setConfigurationListModel(ConfigurationListModel *configListModel);
		void setApiSelector(ApiSelector *apiselector);
	public slots:
        void localize();
        void onRunAloneSelected(const std::string &testId);
	signals:
		void sendSelectedDeviceIndex(int index);
        void startButtonClicked();
    private slots:
		void onStartButtonClicked();
		void onTestSelectButtonClicked();
        void onDescriptionClicked(const QModelIndex &modelIndex);
        void onCloseDescription();
        void onSetGroupData(
                const QModelIndex & index,
                const QVariant & value,
                int role = Qt::EditRole);
		void onApiSelectorClicked();
	private:
        QString getButtonCSS(const QString &buttonName);

		Ui::HomePage ui;
		ApiSelector *m_apiselector;

        BenchmarkService* mBenchmarkService;
        ConfigurationListModel* mConfigurationListModel;
        TestListModel* mTestListModel;
        ConcatProxyModel mConcatProxyModel;
        GroupProxyModel mGroupProxyModel;
        QModelIndex mDescriptionIndex;

		bool m_apiSelectorShown;
	};
}

#endif // HOMEPAGE_H
