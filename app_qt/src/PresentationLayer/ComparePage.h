/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once


#include "ui_ComparePage.h"
#include "CursorListModel.h"

#include <QWidget>



class ApplicationConfig;
class BenchmarkService;
class CompareWidget;



namespace QtUI
{
	class ComparePage : public QWidget
	{
		Q_OBJECT
	public:
		ComparePage(QWidget *parent = 0);
        void setBenchmarkService(BenchmarkService* benchmarkService);
        void setResultListModel(CursorListModel* resultListModel);
        void setCompareListModel(CursorListModel* compareListModel);
        void setDuelListModel(CursorListModel* duelListModel);
	public slots:
        void localize();
	private slots:
        void onResultListUpdated();
        void onCompareListUpdated();
        void onDuelListUpdated();
        void onDeviceClicked();
		void onTestItemChanged(SelectableWidget *current, SelectableWidget *previous);
        void onSearchFilter(const QString &filter);
        void onDuelCloseClicked();
        void onScrolled(double value);
        void onCompareScrollWidgetResized(QSize size);
	private:
        void showTestList();
        void showDuel();
        void addWidgetsBack();

		Ui::ComparePage ui;

        BenchmarkService* mBenchmarkService = nullptr;
        CursorListModel* mResultListModel = nullptr;
        CursorListModel* mCompareListModel = nullptr;
        CursorListModel* mDuelListModel = nullptr;

        QString mActiveDevice;
        QString mUnit;
        std::string mSelectedResult;
        std::string mFilter;
        CompareWidget *mYourDeviceWidget = nullptr;
	};
}
