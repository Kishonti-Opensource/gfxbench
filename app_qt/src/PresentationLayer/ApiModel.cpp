/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ApiModel.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//							APIITEM CLASS
//
//////////////////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------------------
Constructor / destructor
------------------------------------------------------------------------------------------*/
ApiItem::ApiItem(const tfw::ApiDefinition::Type& api)
	:m_checked(false)
{
	m_api = api;
	switch (api)
	{
	case tfw::ApiDefinition::CL:		m_name = "OpenCL";	break;
	case tfw::ApiDefinition::CUDA:		m_name = "Cuda";	break;
	case tfw::ApiDefinition::GL:		m_name = "OpenGL";	break;
	case tfw::ApiDefinition::VULKAN:	m_name = "Vulkan";	break;
	case tfw::ApiDefinition::METAL:		m_name = "Metal";	break;
	case tfw::ApiDefinition::DX:		m_name = "DirectX 11"; break;
	case tfw::ApiDefinition::DX12:		m_name = "DirectX 12"; break;

	default:
		throw std::invalid_argument("invalid api type ApiItem::ApiItem(api)");
	}
}

/*------------------------------------------------------------------------------------------
Public functions
------------------------------------------------------------------------------------------*/
std::string ApiItem::getName()
{
	return m_name;
}

tfw::ApiDefinition::Type ApiItem::getApi()
{
	return m_api;
}

bool ApiItem::checked()
{
	return m_checked;
}

void ApiItem::setChecked(bool checkState)
{
	m_checked = checkState;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//							APIMODEL Class
//
//////////////////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------------------
Constructor / destructor
------------------------------------------------------------------------------------------*/

ApiModel::ApiModel(const std::vector<std::string>& data)
{
	for (auto it = data.begin(); it != data.end(); it++)
	{
		m_items.push_back(new ApiItem(tfw::ApiDefinition::typeFromString(*it)));
	}
}

ApiModel::~ApiModel()
{

}

/*------------------------------------------------------------------------------------------
Public functions
------------------------------------------------------------------------------------------*/
QVariant ApiModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Kishonti::DeviceRole)
	{
		return QVariant(true);
	}

	if (role == Qt::CheckStateRole)
	{
		ApiItem *item = static_cast<ApiItem*>(index.internalPointer());
		if (item->checked())
			return QVariant(Qt::Checked);
		return QVariant(Qt::Unchecked);
	}

	if (role == Qt::DisplayRole)
	{
		ApiItem *item = static_cast<ApiItem*>(index.internalPointer());
		return QVariant(item->getName().c_str());
	}

	return QVariant();
}

Qt::ItemFlags ApiModel::flags(const QModelIndex &index) const
{
	if (index.isValid())
		return 0;
	return QAbstractItemModel::flags(index);
}

QVariant ApiModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}

QModelIndex ApiModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	ApiItem *citem = m_items[row];
	return createIndex(row, column, citem);
}

QModelIndex ApiModel::parent(const QModelIndex &index) const
{
	return QModelIndex();
}

int ApiModel::rowCount(const QModelIndex &parent) const
{
	return (int)m_items.size();
}

int ApiModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

bool ApiModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
	for (auto it = m_items.begin(); it != m_items.end(); it++)
	{
		ApiItem *item = *it;
		item->setChecked(false);
	}

	ApiItem *item = m_items[index.row()];
	item->setChecked(data.toBool());

	m_selected = item->getApi();

	emit dataChanged(this->index(0, 0), this->index((int)m_items.size() - 1, 0));

	return true;
}

tfw::ApiDefinition::Type ApiModel::selectedApi()
{
	return m_selected;
}
