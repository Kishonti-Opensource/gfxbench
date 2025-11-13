/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INFOPAGE_H
#define INFOPAGE_H

#include "ui_InfoPage.h"

#include "CursorListModel.h"

#include "benchmarkservice.h"

#include <QWidget>



class InfoWidget;
class InfoDetailWidget;



namespace QtUI
{

class InfoPage : public QWidget
{
	Q_OBJECT
public:
	InfoPage(QWidget *parent = 0);
    void setBenchmarkService(BenchmarkService* benchmarkService);
    void setSystemInfoListModel(CursorListModel* systemInfoListModel);
public slots:
    void localize();
    void onInfoClicked();
    void onDetailCloseButtonClicked();
    void updateSystemInfo();
    void updateAttributes();
private:
    QString glValue(const QString &value);

	Ui::InfoPage ui;
    InfoWidget* mInfoShown;
    BenchmarkService* mBenchmarkService;
    CursorListModel* mSystemInfoListModel;
    CursorListModel mAttributeListModel;
};

}

#endif
