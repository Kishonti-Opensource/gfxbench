/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DEVICEI_SELECT_H
#define DEVICEI_SELECT_H

#include <QWidget>
#include <QModelIndex>
#include "ui_DeviceWidget.h"
#include <qradiobutton.h>

class DeviceWidget : public QWidget
{
	Q_OBJECT

public:
    DeviceWidget(QWidget *parent = 0);

    QModelIndex modelIndex() const;
    void setModelIndex(const QModelIndex& modelIndex);
    void setDeviceName(const std::string &name);
    void setDeviceType(const std::string &type);
    void setComment(const std::string &comment);
	QRadioButton *radioButton() { return ui.radioButton; }
    void setDisabled(bool);
    void setEnabled(bool);
    void setChecked(bool);
	bool isChecked();
	QString deviceText();

signals:
    void toggled(bool checked);
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
	void onRadioButtonToggled(bool checked);

private:
	Ui::DeviceWidget ui;

    QPoint oldPos;
	QWidget *m_pressed;
    double m_devicePixelRatio;
    QModelIndex m_modelIndex;
	QString m_deviceText;
};

#endif
