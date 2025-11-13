/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "CursorListModel.h"

#include <QIcon>



CursorListModel::CursorListModel(QObject* parent) :
    QAbstractListModel(parent)
{
}



CursorListModel::~CursorListModel()
{
    if (mCursor.get() != nullptr) {
        mCursor->setCallback(nullptr);
    }
}



void CursorListModel::setCursor(const std::shared_ptr<Cursor>& cursor)
{
    beginResetModel();
    if (mCursor.get() != nullptr) {
        mCursor->setCallback(nullptr);
    }
    mCursor = cursor;
    mRoleMap.clear();
    if (mCursor.get() != nullptr) {
        mCursor->setCallback(this);
        addRoles();
    }
    endResetModel();
}



QHash<int, QByteArray> CursorListModel::roleNames() const
{
    if (mCursor.get() == nullptr) {
        return QHash<int, QByteArray>();
    }

    QHash<int, QByteArray> roleNames;
    for (int i = 0; i < mCursor->getColumnCount(); ++i) {
        roleNames[i] = mCursor->getColumnName(i);
    }
    return roleNames;
}



int CursorListModel::rowCount(const QModelIndex&) const
{
    return (mCursor.get() != nullptr) ? mCursor->getCount() : 0;
}



QVariant CursorListModel::data(const QModelIndex& index, int role) const
{
    if ((mCursor.get() == nullptr) ||
        !index.isValid() ||
        (index.row() >= mCursor->getCount()))
    {
        return QVariant();
    }

    mCursor->moveToPosition(index.row());
    int column = mRoleMap.value(role, -1);
    if (column == -1) {
        return QVariant();
    }

    if (role == Qt::DecorationRole) {
        std::string image = mCursor->getString(column);
        return QIcon(QString::fromStdString(image).replace("asset:", ":"));
    }
    if (role == Qt::CheckStateRole) {
        return mCursor->getBoolean(column) ? Qt::Checked : Qt::Unchecked;
    }

    switch (mCursor->getType(column)) {
    case Cursor::FIELD_TYPE_NULL:
        return QVariant();
    case Cursor::FIELD_TYPE_INTEGER:
        return mCursor->getLong(column);
    case Cursor::FIELD_TYPE_FLOAT:
        return mCursor->getDouble(column);
    case Cursor::FIELD_TYPE_STRING:
        return QString::fromStdString(mCursor->getString(column));
    case Cursor::FIELD_TYPE_BLOB: {
        std::string blob = mCursor->getBlob(column);
        return QByteArray(&blob[0], static_cast<int>(blob.size()));
    }
    default:
        return QVariant();
    }
}



Qt::ItemFlags CursorListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags result;
    if ((mCursor.get() == nullptr) || !index.isValid() || (index.row() >= mCursor->getCount())) {
        return result;
    }

    mCursor->moveToPosition(index.row());

    int enabledColumn = mCursor->getColumnIndex("isEnabled");
    if (mCursor->isNull(enabledColumn) || mCursor->getBoolean(enabledColumn)) {
        result |= Qt::ItemIsEnabled;
    }
    if (!mCursor->isNull(mCursor->getColumnIndex("isChecked"))) {
        result |= Qt::ItemIsUserCheckable;
    }
    return result;
}



void BENCHMARK_SERVICE_API CursorListModel::dataSetChanged(int, int)
{
    emit dataChanged(index(0), index(rowCount() - 1));
}



void BENCHMARK_SERVICE_API CursorListModel::dataSetWillBeInvalidated()
{
    beginResetModel();
}



void BENCHMARK_SERVICE_API CursorListModel::dataSetInvalidated()
{
    endResetModel();
}



void CursorListModel::addRole(int role, const char* name)
{
    int column = mCursor->getColumnIndex(name);
    if (column != -1) {
        mRoleMap[role] = column;
    }
}



void CursorListModel::addRoles()
{
    addRole(Qt::DisplayRole, "title");
    addRole(Qt::ToolTipRole, "description");
    addRole(Qt::DecorationRole, "icon");
    addRole(Qt::CheckStateRole, "isChecked");
    addRole(Kishonti::RowIdRole, "_id");
    addRole(Kishonti::ImagePathRole, "icon");
    addRole(Kishonti::GroupRole, "group");
    addRole(Kishonti::ValueRole, "value");
    addRole(Kishonti::MajorRole, "major");
    addRole(Kishonti::MinorRole, "minor");
    addRole(Kishonti::AttributesRole, "attributesJson");
    addRole(Kishonti::RunAloneRole, "isRunalone");
    addRole(Kishonti::VariantOfRole, "variantOf");
    addRole(Kishonti::PrimaryScoreRole, "primaryScore");
    addRole(Kishonti::PrimaryUnitRole, "primaryUnit");
    addRole(Kishonti::SecondaryScoreRole, "secondaryScore");
    addRole(Kishonti::SecondaryUnitRole, "secondatyUnit");
	addRole(Kishonti::DeviceNameRole, "deviceName");
    addRole(Kishonti::ScoreARole, "scoreA");
    addRole(Kishonti::ScoreBRole, "scoreB");
    addRole(Kishonti::DataRole, "data");
    addRole(Kishonti::MetricRole, "metric");
    addRole(Kishonti::UnitRole, "unit");
    addRole(Kishonti::IncompatibilityRole, "incompatibilityText");
    addRole(Kishonti::ApiRole, "api");
}
