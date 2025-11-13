/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTSPAGE_H
#define RESULTSPAGE_H

#include "ui_ResultsPage.h"

#include "CursorListModel.h"
#include "GroupProxyModel.h"

#include <QModelIndex>
#include <QWidget>



class BenchmarkService;



namespace QtUI
{
	class ResultsPage : public QWidget
	{
		Q_OBJECT
	public:
		ResultsPage(QWidget* parent = 0);
        void setBenchmarkService(BenchmarkService* benchmarkService);
        void setSessionListModel(CursorListModel* sessionListModel);
        void setResultListModel(CursorListModel* resultListModel);
        void setResultDetailListModel(CursorListModel* resultDetailListModel);
        void showLatest();
    public slots:
		void localize();
        void onResultListCurrentItemChanged(
                SelectableWidget* current,
                SelectableWidget* previous);
        void onResultClicked(const QModelIndex& modelIndex);
        void onDescriptionClicked(const QModelIndex& modelIndex);
        void resetPanels();
        void updateSessions();
	protected:
        void showEvent(QShowEvent *event);
	private:
        void showPanel(QWidget* panel);
        
		Ui::ResultsPage ui;
        
        GroupProxyModel mGroupProxyModel;
        BenchmarkService* mBenchmarkService;
        SelectableWidget* mBestResultsItem;
        CursorListModel* mSessionListModel;
        CursorListModel* mResultListModel;
        CursorListModel* mResultDetailListModel;
        QModelIndex mDescriptionIndex;
        bool mShowLatest;
	};
}

#endif // RESULTSPAGE_H
