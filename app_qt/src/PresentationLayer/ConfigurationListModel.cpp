/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ConfigurationListModel.h"



ConfigurationListModel::ConfigurationListModel(QObject* parent):
    CursorListModel(parent),
    mBenchmarkService(nullptr)
{
}



QVariant ConfigurationListModel::data(const QModelIndex& index, int role) const
{
    if (role == Kishonti::DeviceRole) {
        return true;
    } else {
        return CursorListModel::data(index, role);
    }
}



bool ConfigurationListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (!index.isValid() || (role != Qt::CheckStateRole) || (value.toInt() != Qt::Checked)) {
        return false;
    }
    int configurationIndex = index.data(Kishonti::RowIdRole).toInt();
    mBenchmarkService->selectConfiguration(configurationIndex);
    return true;
}


Qt::ItemFlags ConfigurationListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags temp = CursorListModel::flags(index);
    if (rowCount() == 1) {
        temp &= ~Qt::ItemIsEnabled;
    }
    return temp;
}



void ConfigurationListModel::setBenchmarkService(BenchmarkService *benchmarkService)
{
    mBenchmarkService = benchmarkService;
}
