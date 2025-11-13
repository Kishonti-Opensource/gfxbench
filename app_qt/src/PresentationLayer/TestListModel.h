/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTLISTMODEL_H
#define TESTLISTMODEL_H

#include "CursorListModel.h"

#include "benchmarkservice.h"

#include <memory>



class TestListModel: public CursorListModel
{
    Q_OBJECT
public:
    TestListModel(QObject* parent = 0);
    bool setData(
            const QModelIndex & index,
            const QVariant & value,
            int role = Qt::EditRole) override;
    void setBenchmarkService(BenchmarkService *benchmarkService);
    void selectRunAlone(const std::string& testId);
signals:
    void runAloneSelected(const std::string &testId);
private:
    BenchmarkService *mBenchmarkService;
};



#endif
