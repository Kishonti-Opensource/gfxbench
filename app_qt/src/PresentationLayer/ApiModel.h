/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __APIMODEL_H__
#define __APIMODEL_H__

#include <QAbstractItemModel>
#include "testfw.h"
#include <benchmarkservice.h>
#include "CursorListModel.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  APIItem

class ApiItem
{
public:
	ApiItem(const tfw::ApiDefinition::Type& api);
	std::string getName();
	tfw::ApiDefinition::Type getApi();
	bool checked();
	void setChecked(bool checkState);

private:
	tfw::ApiDefinition::Type m_api;
	std::string m_name;
	std::string m_id;
	bool m_checked;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  APIModel

class ApiModel : public QAbstractItemModel
{
public:
	ApiModel(const std::vector<std::string>& data);
	virtual ~ApiModel();

	QVariant data(const QModelIndex &index, int role) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	bool setData(const QModelIndex &index, const QVariant &data, int role) override;

	tfw::ApiDefinition::Type selectedApi();

private:

	std::vector<ApiItem*> m_items;
	tfw::ApiDefinition::Type m_selected;
};

#endif //__APIMODEL_H__