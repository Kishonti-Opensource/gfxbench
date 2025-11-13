/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CURSORLISTMODEL_H
#define CURSORLISTMODEL_H

#include "cursor.h"

#include <QAbstractListModel>

#include <memory>



class BenchmarkService;



namespace Kishonti
{
    enum Role {
        RowIdRole = Qt::UserRole,
        ImagePathRole,
        GroupRole,
        ValueRole,
        MajorRole,
        MinorRole,
        AttributesRole,
        DeviceRole,
        HeaderRole,
        RunAloneRole,
        VariantOfRole,
        PrimaryScoreRole,
        PrimaryUnitRole,
        SecondaryScoreRole,
        SecondaryUnitRole,
		DeviceNameRole,
        ScoreARole,
        ScoreBRole,
        DataRole,
        MetricRole,
        UnitRole,
        IncompatibilityRole,
        ApiRole
    };
}



class CursorListModel : public QAbstractListModel, public CursorCallback
{
    Q_OBJECT
public:
    CursorListModel(QObject* parent = 0);
    ~CursorListModel();
    void setCursor(const std::shared_ptr<Cursor>& cursor);
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void BENCHMARK_SERVICE_API dataSetWillChange(int first, int last) override {}
    void BENCHMARK_SERVICE_API dataSetChanged(int first, int last) override;
    void BENCHMARK_SERVICE_API dataSetWillBeInvalidated() override;
    void BENCHMARK_SERVICE_API dataSetInvalidated() override;
private:
    std::shared_ptr<Cursor> mCursor;
    QHash<int, int> mRoleMap;

    void addRole(int role, const char* name);
    void addRoles();
};



#endif // CURSORLISTMODEL_H
