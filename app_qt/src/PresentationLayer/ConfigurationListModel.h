/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CONFIGURATIONLISTMODEL_H
#define CONFIGURATIONLISTMODEL_H

#include "CursorListModel.h"

#include "benchmarkservice.h"

#include <memory>



class ConfigurationListModel: public CursorListModel
{
    Q_OBJECT
public:
    ConfigurationListModel(QObject* parent = 0);
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex & index,
            const QVariant & value,
            int role = Qt::EditRole) override;
    void setBenchmarkService(BenchmarkService *benchmarkService);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
private:
    BenchmarkService *mBenchmarkService;
};



#endif
