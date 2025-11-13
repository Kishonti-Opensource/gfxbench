/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ItemScrollWidget.h"

#include "CursorListModel.h"
#include "GroupWidget.h"
#include "TestWidget.h"
#include "DeviceWidget.h"

#include <QAbstractItemModel>
#include <QDateTime>
#include <QIcon>

#include <cassert>
#include <iostream>

#include "ui_ScrollWidget.h"
#include "ng/log.h"

ItemScrollWidget::ItemScrollWidget(QWidget* parent):
    ScrollWidget(parent)
{}



void ItemScrollWidget::setModel(QAbstractItemModel* model)
{
    bool success;
    m_model = model;
    modelReset();

    success = connect(
            model, &QAbstractItemModel::dataChanged,
            this, &ItemScrollWidget::onDataChanged);
    assert(success);

    success = connect(
            model, &QAbstractItemModel::rowsInserted,
            this, &ItemScrollWidget::rowsInserted);
    assert(success);

    success = connect(
            model, &QAbstractItemModel::rowsInserted,
            this, &ItemScrollWidget::rowsRemoved);
    assert(success);

    success = connect(
            model, &QAbstractItemModel::modelReset,
            this, &ItemScrollWidget::modelReset);
    assert(success);

    Q_UNUSED(success)
}



void ItemScrollWidget::rowsInserted(
        const QModelIndex& /*parent*/,
        int /*first*/,
        int /*last*/)
{
    // TODO
}



void ItemScrollWidget::rowsRemoved(
        const QModelIndex& /*parent*/,
        int /*first*/,
        int /*last*/)
{
    // TODO
}

void ItemScrollWidget::modelReset()
{
    if (m_model == NULL) return;
    clear();
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        if (index.data(Kishonti::DeviceRole).toBool()) {
            addWidget(createDeviceWidget(index));
        } else if (index.data(Kishonti::HeaderRole).toBool()) {
            addWidget(createHeader(index));
        } else {
            addWidget(createItemWidget(index));
        }
    }
}



void ItemScrollWidget::onDataChanged(
        const QModelIndex& topLeft,
        const QModelIndex& bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid()) {
        return;
    }
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
        QModelIndex index = m_model->index(row, 0, QModelIndex());
        QWidget* widget = widgetAt(row);
        GroupWidget* groupWidget = dynamic_cast<GroupWidget*>(widget);
        if (groupWidget != NULL) {
            updateHeader(groupWidget, index);
        }
        TestWidget* testWidget = dynamic_cast<TestWidget*>(widget);
        if (testWidget != NULL) {
            updateItemWidget(testWidget, index);
        }
        DeviceWidget* deviceWidget = dynamic_cast<DeviceWidget*>(widget);
        if (deviceWidget != NULL) {
            updateDeviceWidget(deviceWidget, index);
        }
    }
}



void ItemScrollWidget::onTestToggled(bool onscreen)
{
    if (m_model == NULL) return;
    TestWidget* itemWidget = dynamic_cast<TestWidget*>(sender());
    assert(itemWidget != NULL);

    QModelIndex index = itemWidget->modelIndex();
    if (onscreen) {
        bool onscreenChecked = itemWidget->isOscreenChecked();
        bool success = m_model->setData(
            index,
            onscreenChecked ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole);
        if (!success) {
            itemWidget->setOnscreenChecked(!onscreenChecked);
        }
    } else {
        bool offscreenChecked = itemWidget->isOffscreenChecked();
        bool success = m_model->setData(
            index.sibling(index.row(), 1),
            offscreenChecked ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole);
        if (!success) {
            itemWidget->setOffscreenChecked(!offscreenChecked);
        }
    }
}



void ItemScrollWidget::onGroupToggled(bool)
{
    if (m_model == NULL) return;
    GroupWidget* itemWidget = dynamic_cast<GroupWidget*>(sender());
    assert(itemWidget != NULL);

    bool success = m_model->setData(
            itemWidget->modelIndex(),
            itemWidget->isChecked() ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole);
    if (!success) {
        itemWidget->setChecked(false);
    }
}



void ItemScrollWidget::onDeviceToggled(bool checked)
{
    if (m_model == NULL) return;
    DeviceWidget* itemWidget = dynamic_cast<DeviceWidget*>(sender());
    assert(itemWidget != NULL);

    m_model->setData(
            itemWidget->modelIndex(),
            checked ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole);
}



void ItemScrollWidget::onClicked()
{
    if (m_model == NULL) return;
    TestWidget* itemWidget = dynamic_cast<TestWidget*>(sender());
    assert(itemWidget != NULL);

    emit clicked(itemWidget->modelIndex());
}



void ItemScrollWidget::onDescriptionClicked()
{
    if (m_model == NULL) return;
    TestWidget* itemWidget = dynamic_cast<TestWidget*>(sender());
    assert(itemWidget != NULL);

    emit descriptionClicked(itemWidget->modelIndex());
}



GroupWidget* ItemScrollWidget::createHeader(const QModelIndex& index)
{
    GroupWidget* header;
    bool withCheckbox = index.flags().testFlag(Qt::ItemIsUserCheckable);
    bool withOffscreen = index.sibling(index.row() + 1, 1).data(Qt::DisplayRole).isValid();
    if (withCheckbox) {
        if (withOffscreen) {
            header = GroupWidget::createSelectWidgetWithOnOffscreen(
                    index.data(Qt::DisplayRole).toString(), this);
        } else {
            header = GroupWidget::createSelectWidget(
                    index.data(Qt::DisplayRole).toString(), this);
        }
    } else {
        if (withOffscreen) {
            header = GroupWidget::createResultWidgetWithOnOffscreen(
                    index.data(Qt::DisplayRole).toString(), this);
        } else {
            header = GroupWidget::createResultWidget(
                    index.data(Qt::DisplayRole).toString(), this);
        }
    }

    bool success = connect(header, &GroupWidget::toggled,
            this, &ItemScrollWidget::onGroupToggled, Qt::QueuedConnection);
    assert(success);
    Q_UNUSED(success)

    updateHeader(header, index);
    return header;
}



void ItemScrollWidget::updateHeader(
        GroupWidget* header,
        const QModelIndex& index)
{
    header->setModelIndex(index);
    header->setChecked(index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
}



TestWidget* ItemScrollWidget::createItemWidget(const QModelIndex& index)
{
    bool success;
    TestWidget* itemWidget = TestWidget::createSelectWidgetWithOffscreen(
            index.data(Qt::DisplayRole).toString(),
            index.data(Qt::ToolTipRole).toString(),
            QStringList(),
            this);
    itemWidget->showOnscreenOpenArrow(false);
    itemWidget->showOffscreenOpenArrow(false);
    updateItemWidget(itemWidget, index);

    success = connect(itemWidget, &TestWidget::toggled,
            this, &ItemScrollWidget::onTestToggled, Qt::QueuedConnection);
    assert(success);

    success = connect(itemWidget, &TestWidget::descriptionClicked,
            this, &ItemScrollWidget::onDescriptionClicked,
            Qt::QueuedConnection);
    assert(success);

    success = connect(itemWidget, &TestWidget::resultClicked,
            this, &ItemScrollWidget::resultClicked, Qt::QueuedConnection);
    assert(success);

    Q_UNUSED(success)

    return itemWidget;
}



void ItemScrollWidget::updateItemWidget(
        TestWidget* itemWidget,
        const QModelIndex& index)
{
    QModelIndex onscreen = index.sibling(index.row(), 0);
    QModelIndex offscreen = index.sibling(index.row(), 1);

    itemWidget->setModelIndex(onscreen);
    
    itemWidget->setEnabled(onscreen.flags().testFlag(Qt::ItemIsEnabled));
    itemWidget->setTitle(onscreen.data(Qt::DisplayRole).toString());
    itemWidget->setDescription(onscreen.data(Qt::ToolTipRole).toString());
    itemWidget->showCheckPanel(onscreen.flags().testFlag(Qt::ItemIsUserCheckable));
    itemWidget->setOnscreenChecked(onscreen.data(Qt::CheckStateRole).toInt() == Qt::Checked);
    itemWidget->showResultPanel(!onscreen.flags().testFlag(Qt::ItemIsUserCheckable));
    itemWidget->setOnscreenText(
            onscreen.data(Kishonti::MajorRole).toString(),
            onscreen.data(Kishonti::MinorRole).toString());
    itemWidget->showCheckOffscreenPanel(offscreen.flags().testFlag(Qt::ItemIsUserCheckable));
    itemWidget->setOffscreenChecked(offscreen.data(Qt::CheckStateRole).toInt() == Qt::Checked);
    itemWidget->showResultOffscreenPanel(
            !offscreen.flags().testFlag(Qt::ItemIsUserCheckable) &&
            offscreen.data(Qt::DisplayRole).isValid());
    itemWidget->setOffscreenText(
            offscreen.data(Kishonti::MajorRole).toString(),
            offscreen.data(Kishonti::MinorRole).toString());
    itemWidget->setIcon(onscreen.data(Qt::DecorationRole).value<QIcon>());
    itemWidget->setIncompatibleText(onscreen.data(Kishonti::IncompatibilityRole).toString());
    /*itemWidget->showOnscreenOpenArrow(
            onscreen.data(QtUI::OpenableRole).toBool());*/
}



DeviceWidget* ItemScrollWidget::createDeviceWidget(const QModelIndex& index)
{
    DeviceWidget* itemWidget = new DeviceWidget(this);
    updateDeviceWidget(itemWidget, index);
    
    bool success = connect(itemWidget, &DeviceWidget::toggled,
        this, &ItemScrollWidget::onDeviceToggled, Qt::QueuedConnection);
    assert(success);
    Q_UNUSED(success)

    return itemWidget;
}



void ItemScrollWidget::updateDeviceWidget(
    DeviceWidget* itemWidget,
    const QModelIndex& index)
{
    itemWidget->setModelIndex(index);

	const bool checked = index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
	const bool enabled = index.flags().testFlag(Qt::ItemIsEnabled);
#if defined(COMPUBENCH)

    itemWidget->setDeviceName(index.data(Qt::DisplayRole).toString().toStdString());
#else
	const QString api = index.data(Kishonti::ApiRole).toString();
	itemWidget->setDeviceName(QString(index.data(Qt::DisplayRole).toString() + " (" + api + ")").toStdString());
#endif

    itemWidget->setDeviceType(index.data(Qt::ToolTipRole).toString().toStdString());
    itemWidget->setComment(index.data(Kishonti::MajorRole).toString().toStdString());

    itemWidget->setChecked(checked);
    itemWidget->setEnabled(enabled);

#if defined(COMPUBENCH) || defined(GFXBENCH) || defined(GFXBENCH_DX)
	itemWidget->setVisible(checked && enabled);
	itemWidget->setEnabled(false);
#else
	itemWidget->setEnabled(enabled);
#endif
}


/*---------------------------------------------------------------
DeviceSelectorScrollWidget inhertited from ItemScroll widget
---------------------------------------------------------------*/

DeviceSelectorScrollWidget::DeviceSelectorScrollWidget(QWidget *parent)
	: ItemScrollWidget(parent), m_callback(nullptr)
{
	ui->scrollAreaWidgetContents->setStyleSheet("border-width: 1px;");
	ui->scrollAreaWidgetContents->layout()->setMargin(2);
}

DeviceSelectorScrollWidget::~DeviceSelectorScrollWidget()
{

}

/*-------------------------------------------------------------------
Overrides
-------------------------------------------------------------------*/
void DeviceSelectorScrollWidget::modelReset()
{
	if (m_model == NULL)
		return;
	clear();
	int rowIndex = 0;
	for (int i = 0; i < m_model->rowCount(); ++i) {
		QModelIndex index = m_model->index(i, 0);
		if (index.data(Kishonti::DeviceRole).toBool()) {

			std::string apilist = index.data(Kishonti::ApiRole).toString().toStdString();
			QString s(apilist.c_str());
			auto list = s.split("|");
			
			for (auto it = list.begin(); it != list.end(); it++)
			{
				std::string api = it->simplified().toStdString();

				auto found = std::find(m_apis.begin(), m_apis.end(), api);
				if (found == m_apis.end())
					m_apis.push_back(api);
			}

			DeviceWidget *widget = createDeviceWidget(index);
			addWidget(widget);
			rowIndex++;
		}
	}
	if (m_callback && !m_apis.empty())
	{
		m_callback->onApiCollected();
		m_callback = nullptr; // call only once
	}
}

std::vector<std::string> DeviceSelectorScrollWidget::allRenderApi()
{
	std::vector<std::string> renderApis;
	for (auto it = m_apis.begin(); it != m_apis.end(); it++)
	{
		if (*it != "CUDA" && *it != "CL")
			renderApis.push_back(*it);
	}
	return renderApis;
}

std::vector<std::string> DeviceSelectorScrollWidget::allComputeApi()
{
	std::vector<std::string> computeApis;
	for (auto it = m_apis.begin(); it != m_apis.end(); it++)
	{
		if (*it == "CUDA" || *it == "CL" || *it == "METAL")
			computeApis.push_back(*it);
	}
	return computeApis;
}

void DeviceSelectorScrollWidget::updateDeviceWidget(
	DeviceWidget* itemWidget,
	const QModelIndex& index)
{
	itemWidget->setModelIndex(index);
	itemWidget->setDeviceName(index.data(Qt::DisplayRole).toString().toStdString());
	itemWidget->setDeviceType(index.data(Qt::ToolTipRole).toString().toStdString());
	itemWidget->setComment(index.data(Kishonti::MajorRole).toString().toStdString());
	itemWidget->setChecked(index.data(Qt::CheckStateRole).toInt() == Qt::Checked);
}