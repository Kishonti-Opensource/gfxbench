/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ConcatProxyModel.h"

#include <QVector>

#include <cassert>



class ConcatProxyModel::Private
{
public:
    QVector<QAbstractItemModel*> sourceModels;
    int rowCount;
    int columnCount;
};



ConcatProxyModel::ConcatProxyModel(QObject* parent):
    QAbstractTableModel(parent),
    d(new Private)
{
}



ConcatProxyModel::~ConcatProxyModel()
{}

void ConcatProxyModel::addSourceModel(QAbstractItemModel* model)
{
    bool success = QObject::connect(model, &QAbstractItemModel::dataChanged,
            this, &ConcatProxyModel::sourceDataChanged);
    assert(success);

    success = QObject::connect(model, &QAbstractItemModel::modelReset,
            this, &ConcatProxyModel::sourceModelReset);
    assert(success);

    Q_UNUSED(success)
    
    beginResetModel();
    d->sourceModels.push_back(model);
    endResetModel();
}



QHash<int, QByteArray> ConcatProxyModel::roleNames() const
{
    if (d->sourceModels.empty()) {
        return QHash<int, QByteArray>();
    }
    return d->sourceModels.front()->roleNames();
}



int ConcatProxyModel::rowCount(const QModelIndex&) const
{
    int result = 0;
    foreach(QAbstractItemModel* sourceModel, d->sourceModels) {
        result += sourceModel->rowCount();
    }
    return result;
}



int ConcatProxyModel::columnCount(const QModelIndex&) const
{
    int result = 0;
    foreach(QAbstractItemModel* sourceModel, d->sourceModels) {
        result = std::max(sourceModel->rowCount(), result);
    }
    return result;
}



QVariant ConcatProxyModel::data(const QModelIndex& index, int role) const
{
    return sourceIndex(index).data(role);
}



bool ConcatProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    QModelIndex source = sourceIndex(index);
    return const_cast<QAbstractItemModel*>(source.model())->setData(source, value, role);
}



Qt::ItemFlags ConcatProxyModel::flags(const QModelIndex& index) const
{
    return sourceIndex(index).flags();
}



QModelIndex ConcatProxyModel::sourceIndex(const QModelIndex& wrappedIndex) const
{
    if (!wrappedIndex.isValid()) {
        return QModelIndex();
    }
    
    int i = 0;
    int row = wrappedIndex.row();
    while ((i < d->sourceModels.size()) && (row >= d->sourceModels.at(i)->rowCount())) {
        row -= d->sourceModels.at(i)->rowCount();
        ++i;
    }
    if (i >= d->sourceModels.size()) {
        return QModelIndex();
    }

    return d->sourceModels.at(i)->index(row, wrappedIndex.column());
}



void ConcatProxyModel::sourceDataChanged(
        const QModelIndex&,
        const QModelIndex&,
        const QVector<int>& roles)
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), roles);
}



void ConcatProxyModel::sourceModelReset()
{
    beginResetModel();
    endResetModel();
}
