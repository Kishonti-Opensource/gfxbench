/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTDETAILPAGE_H
#define RESULTDETAILPAGE_H

#include <QWidget>

#include "CursorListModel.h"

#include <memory>



namespace Ui {
    class ResultDetailPage;
}
class ChartWidget;



class ResultDetailPage : public QWidget
{
    Q_OBJECT
public:
    explicit ResultDetailPage(QWidget *parent = 0);
    ~ResultDetailPage();
    void setBenchmarkService(BenchmarkService* benchmarkService);
    void setTitle(const QString& title);
    void setResultRowId(long long rowId);
    void setModel(CursorListModel* model);
public slots:
    void localize();
    void update();
signals:
    void closeClicked();
private:
    Ui::ResultDetailPage *ui;
    BenchmarkService* mBenchmarkService;
    CursorListModel* mModel;
    long long mResultRowId;
};



#endif // RESULTDETAILPAGE_H
