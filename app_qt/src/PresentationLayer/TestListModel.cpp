/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "TestListModel.h"



TestListModel::TestListModel(QObject* parent):
    CursorListModel(parent),
    mBenchmarkService(nullptr)
{
}



bool TestListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (!index.isValid() || (role != Qt::CheckStateRole)) {
        return false;
    }
    QString testId = index.data(Qt::DisplayRole).toString();
    if (index.data(Kishonti::RunAloneRole).toBool() && (value.toInt() == Qt::Checked)) {
        emit runAloneSelected(testId.toStdString());
        return false;
    }
    mBenchmarkService->setTestSelection(testId.toUtf8(), value.toInt() == Qt::Checked);
    return true;
}



void TestListModel::setBenchmarkService(BenchmarkService *benchmarkService)
{
    mBenchmarkService = benchmarkService;
}



void TestListModel::selectRunAlone(const std::string& testId)
{
    assert(mBenchmarkService != nullptr);
    mBenchmarkService->setTestSelection(testId.c_str(), true);
}
