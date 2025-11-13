/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "GroupProxyModel.h"

#include "CursorListModel.h"
#include "Dictionary.h"

#include <QVector>

#include <cassert>



struct Mapping
{
    Mapping()
    {
        headerCheckState = Qt::Unchecked;
    }

    int sourceRow;
    Qt::CheckState headerCheckState;
    bool isHeader;
};



class GroupProxyModel::Private
{
public:
    QAbstractListModel* sourceModel;
    QVector<QVector<Mapping> > mappings;
    bool isMultiColumn;
    bool hideNA;
    void refreshMappings();
    void createMappings();
    void updateGroupCheckStates();
    void removeNAs();
};



GroupProxyModel::GroupProxyModel(QObject* parent):
    QAbstractTableModel(parent),
    d(new Private)
{
    d->sourceModel = nullptr;
    d->isMultiColumn = false;
    d->hideNA = false;
}



GroupProxyModel::~GroupProxyModel()
{}



void GroupProxyModel::setSourceModel(QAbstractListModel* model)
{
    if (d->sourceModel != nullptr) {
        QObject::disconnect(d->sourceModel, nullptr, this, nullptr);
    }
    d->sourceModel = model;
    if (d->sourceModel != nullptr) {
        bool success;
        success = QObject::connect(d->sourceModel, &QAbstractItemModel::dataChanged,
                this, &GroupProxyModel::sourceDataChanged);
        assert(success);

        success = QObject::connect(d->sourceModel, &QAbstractItemModel::modelReset,
                this, &GroupProxyModel::sourceModelReset);
        assert(success);

        Q_UNUSED(success)
    }
}



QHash<int, QByteArray> GroupProxyModel::roleNames() const
{
    if (d->sourceModel == nullptr) {
        return QHash<int, QByteArray>();
    }
    return d->sourceModel->roleNames();
}



int GroupProxyModel::rowCount(const QModelIndex&) const
{
    if (d->sourceModel == nullptr) {
        return 0;
    }
    return d->mappings.size();
}



int GroupProxyModel::columnCount(const QModelIndex&) const
{
    return 2;// d->mappings.front().size();
}



QVariant GroupProxyModel::data(const QModelIndex& index, int role) const
{
    if (d->sourceModel == nullptr) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.column() >= d->mappings[index.row()].size()) {
        return QVariant();
    }

    const Mapping& mapping = d->mappings[index.row()][index.column()];
    if (!mapping.isHeader) {
        return d->sourceModel->index(mapping.sourceRow).data(role);
    }

    switch (role) {
    case Qt::DisplayRole:
        return QtUI::dict(d->sourceModel->index(mapping.sourceRow).data(
                Kishonti::GroupRole).toString());
    case Qt::CheckStateRole:
        return mapping.headerCheckState;
    case Kishonti::HeaderRole:
        return mapping.isHeader;
    case Kishonti::GroupRole:
        return d->sourceModel->index(mapping.sourceRow).data(Kishonti::GroupRole);
    default:
        return QVariant();
    }
}



bool GroupProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (d->sourceModel == nullptr) {
        return false;
    }
    if (!index.isValid()) {
        return false;
    }
    if (index.column() >= d->mappings[index.row()].size()) {
        return false;
    }

    const Mapping& mapping = d->mappings[index.row()][index.column()];
    if (mapping.isHeader) {
        emit setGroupData(index, value, role);
        return true;
    } else {
        return d->sourceModel->setData(d->sourceModel->index(mapping.sourceRow), value, role);
    }
}



Qt::ItemFlags GroupProxyModel::flags(const QModelIndex& index) const
{
    return sourceIndex(index).flags();
}



QModelIndex GroupProxyModel::sourceIndex(const QModelIndex& wrappedIndex) const
{
    if (!wrappedIndex.isValid()) {
        return QModelIndex();
    }
    if (wrappedIndex.column() >= d->mappings[wrappedIndex.row()].size()) {
        return QModelIndex();
    }
    const Mapping& mapping = d->mappings[wrappedIndex.row()][wrappedIndex.column()];
    return d->sourceModel->index(mapping.sourceRow);
}



void GroupProxyModel::setMultiColumn(bool isMultiColumn)
{
    d->isMultiColumn = isMultiColumn;
}



void GroupProxyModel::setHideNA(bool hideNA)
{
    d->hideNA = hideNA;
}



void GroupProxyModel::sourceDataChanged(
        const QModelIndex& topLeft,
        const QModelIndex&,
        const QVector<int>& roles)
{
    d->refreshMappings();
    emit dataChanged(topLeft, index(d->mappings.size() - 1, 0), roles);
}



void GroupProxyModel::sourceModelReset()
{
    beginResetModel();
    d->refreshMappings();
    endResetModel();
}



void GroupProxyModel::Private::refreshMappings()
{
    createMappings();
    updateGroupCheckStates();
    if (hideNA) {
        removeNAs();
    }
}



void GroupProxyModel::Private::createMappings()
{
    mappings.clear();
    QVariant lastGroupId = "XXX";
    QVariant lastVariantOf;
    for (int i = 0; i < sourceModel->rowCount(); ++i) {
        QVariant groupId = sourceModel->index(i).data(Kishonti::GroupRole);
        if (lastGroupId != groupId) {
            lastVariantOf.clear();
            lastGroupId = groupId;
            Mapping headerMapping;
            headerMapping.sourceRow = i;
            headerMapping.headerCheckState = Qt::Unchecked;
            headerMapping.isHeader = true;
            mappings.push_back(QVector<Mapping>());
            mappings.back().push_back(headerMapping);
        }
        Mapping mapping;
        mapping.sourceRow = i;
        mapping.isHeader = false;
        QVariant variantOf = sourceModel->index(i).data(Kishonti::VariantOfRole);
        if (!isMultiColumn || (lastVariantOf != variantOf)) {
            mappings.push_back(QVector<Mapping>());
            lastVariantOf = variantOf;
        }
        mappings.back().push_back(mapping);
    }
}



void GroupProxyModel::Private::updateGroupCheckStates()
{
    int totalCount = 0;
    int checkedCount = 0;
    for (int i = mappings.size() - 1; i >= 0; --i) {
        for (int j = mappings[i].size() - 1; j >= 0; --j) {
            Mapping& mapping = mappings[i][j];
            if (mapping.isHeader) {
                if (checkedCount == totalCount) {
                    mapping.headerCheckState = Qt::Checked;
                } else if (checkedCount == 0) {
                    mapping.headerCheckState = Qt::Unchecked;
                } else {
                    mapping.headerCheckState = Qt::PartiallyChecked;
                }
                totalCount = 0;
                checkedCount = 0;
            } else {
                ++totalCount;
                QModelIndex sourceIndex = sourceModel->index(mappings[i][j].sourceRow);
                if (sourceIndex.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
                    ++checkedCount;
                }
            }
        }
    }
}



void GroupProxyModel::Private::removeNAs()
{
    for (int i = mappings.size() - 1; i >= 0; --i) {
        int j;
        for (j = 0; j < mappings[i].size(); ++j) {
            if (sourceModel->index(mappings[i][j].sourceRow).data(
                    Kishonti::MajorRole).toString() != "Results_NA")
            {
                break;
            }
        }
        if (j >= mappings[i].size()) {
            mappings.removeAt(i);
        }
    }
}
