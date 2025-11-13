/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "DeviceWidget.h"
#include <QMouseEvent>
#include <QDebug>
#include "Dictionary.h"

using namespace QtUI;

DeviceWidget::DeviceWidget(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
    
    m_devicePixelRatio = 1.0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_devicePixelRatio = devicePixelRatio();
#endif
    
    if(m_devicePixelRatio > 1.0)
    {
        QString chip =
        "QLabel#imageLabel { \
            border-image: url(:/retina_cpu.png) 0 0 0 0 stretch stretch; \
        }";
        ui.imageLabel->setStyleSheet(chip);
        QString radio =
        "QRadioButton::indicator:unchecked { \
            border-image: url(:/retina_checkbox_uncheck.png) 0 0 0 0 stretch stretch; \
        } \
        \
        QRadioButton::indicator:checked { \
        \
            border-image: url(:/retina_checkbox_check.png) 0 0 0 0 stretch stretch; \
        }";
        ui.radioButton->setStyleSheet(radio);

    }

	connect(ui.radioButton, SIGNAL(toggled(bool)), this, SLOT(onRadioButtonToggled(bool)));
}

QModelIndex DeviceWidget::modelIndex() const
{
    return m_modelIndex;
}

void DeviceWidget::setModelIndex(const QModelIndex& modelIndex)
{
    m_modelIndex = modelIndex;
}

void DeviceWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui.verticalSpace->width();
        QRect checkRect = QRect(QPoint(ui.titleWidget->width() + line, 0), ui.checkWidget->size());

        if(checkRect.contains(event->pos()))
        {
            m_pressed = ui.checkWidget;
        }
    }
}

void DeviceWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui.verticalSpace->width();
        QRect checkRect = QRect(QPoint(ui.titleWidget->width() + line, 0), ui.checkWidget->size());

        if(checkRect.contains(event->pos()) && m_pressed == ui.checkWidget)
        {
            this->ui.radioButton->click();
        }
    }
}

void DeviceWidget::onRadioButtonToggled(bool checked)
{
    if (!checked) {
        ui.radioButton->blockSignals(true);
        ui.radioButton->setChecked(true);
        ui.radioButton->blockSignals(false);
        return;
    }
    setChecked(checked);
    emit toggled(checked);
}

void DeviceWidget::setDisabled(bool set)
{
    this->setEnabled(!set);
}

QString DeviceWidget::deviceText()
{
	return m_modelIndex.data(Qt::DisplayRole).toString();
}

void DeviceWidget::setEnabled(bool set)
{
    QWidget::setEnabled(set);
    if(set)
    {
        ui.radioButton->show();
        ui.commentLabel->hide();
    }
    else
    {
        ui.radioButton->hide();
        ui.commentLabel->show();
    }
}

void DeviceWidget::setChecked(bool set)
{
    ui.radioButton->blockSignals(true);
    ui.radioButton->setChecked(set);
    ui.radioButton->blockSignals(false);
    QString widget = QString("QWidget#checkWidget { background-color: %1 }");
    QString radio = " \
    QRadioButton::indicator:unchecked { \
        background-color: %1; \
        border-image: url(:/retina_checkbox_uncheck.png) 0 0 0 0 stretch stretch; \
    } \
    \
    QRadioButton::indicator:checked { \
        background-color: %1; \
        border-image: url(:/retina_checkbox_check.png) 0 0 0 0 stretch stretch; \
    }";
	if(set)
    {
		ui.checkWidget->setStyleSheet(widget.arg(property("selectColor").toString()));
        ui.radioButton->setStyleSheet(radio.arg(property("selectColor").toString()));
    } else {
        ui.checkWidget->setStyleSheet(widget.arg(property("unselectColor").toString()));
        ui.radioButton->setStyleSheet(radio.arg(property("unselectColor").toString()));
    }
}

bool DeviceWidget::isChecked()
{
	return ui.radioButton->isChecked();
}

void DeviceWidget::setDeviceType(const std::string &type)
{
    QString chip =
    "QLabel#imageLabel { \
    border-image: url(:/%1) 0 0 0 0 stretch stretch; \
    }";
    QString pic;

    if(m_devicePixelRatio > 1.0)
    {
        pic = "retina_";
    }
    
    QString qtype = QString::fromStdString(type);
    if(qtype == "ACC")
    {
        pic.append("config_acc");
    }
    else if(qtype == "CPU")
    {
        pic.append("config_cpu");
    }
    else if(qtype == "dGPU")
    {
        pic.append("config_dgpu");
    }
    else if(qtype == "iGPU")
    {
        pic.append("config_igpu");
    }
    else if(qtype == "mGPU")
    {
        pic.append("config_mgpu");
    }
    else if(qtype == "CPU-iGPU")
    {
        pic.append("config_soc");
    }
    pic.append(".png");
    QString css = chip.arg(pic);
    ui.imageLabel->setStyleSheet(css);
}

void DeviceWidget::setDeviceName(const std::string &name)
{
    ui.deviceNameLabel->setText(name.c_str());
	m_deviceText.fromStdString(name);
}

void DeviceWidget::setComment(const std::string &comment)
{
    ui.commentLabel->setText(dict(comment.c_str()));
}
