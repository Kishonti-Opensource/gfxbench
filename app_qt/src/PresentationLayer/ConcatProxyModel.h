/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CONCATPROXYMODEL_H
#define CONCATPROXYMODEL_H

#include <QAbstractTableModel>

#include <memory>



class ConcatProxyModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    ConcatProxyModel(QObject* parent = 0);
    ~ConcatProxyModel();
    void addSourceModel(QAbstractItemModel* model);
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(
            const QModelIndex & index,
            const QVariant & value,
            int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex sourceIndex(const QModelIndex& wrappedIndex) const;
private slots:
    void sourceDataChanged(
            const QModelIndex& topLeft,
            const QModelIndex& bottomRight,
            const QVector<int>& roles);
    void sourceModelReset();
private:
    class Private;
    std::unique_ptr<Private> d;
};



#endif // CONCATPROXYMODEL_H
