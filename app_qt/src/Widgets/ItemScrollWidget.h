/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef ITEMSCROLLWIDGET_H
#define ITEMSCROLLWIDGET_H

#include "ScrollWidget.h"



class QAbstractItemModel;
class GroupWidget;
class TestWidget;
class DeviceWidget;



class ItemScrollWidget: public ScrollWidget
{
    Q_OBJECT
public:
    ItemScrollWidget(QWidget* parent = 0);
    void setModel(QAbstractItemModel* model);
signals:
    void clicked(const QModelIndex& modelIndex);
    void descriptionClicked(const QModelIndex& modelIndex);
    void resultClicked(const QModelIndex& modelIndex);
private slots:
    void rowsInserted(const QModelIndex& parent, int first, int last);
    void rowsRemoved(const QModelIndex& parent, int first, int last);
    virtual void modelReset();
    void onDataChanged(
            const QModelIndex& topLeft,
            const QModelIndex& bottomRight);

    void onTestToggled(bool checked);
    void onGroupToggled(bool checked);
    void onDeviceToggled(bool checked);
    void onClicked();
    void onDescriptionClicked();
protected:
    QAbstractItemModel* m_model;

    GroupWidget* createHeader(const QModelIndex& index);
    void updateHeader(GroupWidget* header, const QModelIndex& index);
    TestWidget* createItemWidget(const QModelIndex& index);
    void updateItemWidget(TestWidget* itemWidget, const QModelIndex& index);
    DeviceWidget* createDeviceWidget(const QModelIndex& index);
    virtual void updateDeviceWidget(DeviceWidget* itemWidget, const QModelIndex& index);
};


#include <testfw.h>
#include <testinfo.h>
#include <functional>

class IDeviceSelectorCallBack
{
public:
	virtual void onApiCollected() = 0;
};


class DeviceSelectorScrollWidget : public ItemScrollWidget
{
public:
	DeviceSelectorScrollWidget(QWidget *parent = nullptr);
	virtual ~DeviceSelectorScrollWidget();

	void modelReset() override;
	std::vector<std::string> allRenderApi();
	std::vector<std::string> allComputeApi();
	void setApiCollectedCallback(IDeviceSelectorCallBack* callback) { m_callback = callback; }

private:
	void updateDeviceWidget( DeviceWidget* itemWidget, const QModelIndex& index) override;
	std::vector<std::string> m_apis;

	QString m_selectedDevice;
	IDeviceSelectorCallBack* m_callback;
};


#endif
