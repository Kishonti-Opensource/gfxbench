/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ScrollWidget.h"
#include "ui_ScrollWidget.h"

#include <QScrollBar>
#include <QDebug>
#include <QResizeEvent>

ScrollWidget::ScrollWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScrollWidget)
{
    ui->setupUi(this);
#ifdef Q_OS_MAC
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif
    m_index =  0;
    m_layout = (QBoxLayout*) ui->scrollAreaWidgetContents->layout();
    m_spacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_layout->insertSpacerItem(m_index, m_spacer);
    connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
}

ScrollWidget::~ScrollWidget()
{
    clear();
    
    delete ui;
}

void ScrollWidget::clear()
{
    foreach(QWidget* w, m_widgets)
    {
        if(w)
        {
            m_layout->removeWidget(w);
            delete w;
        }
    }
    m_widgets.clear();
    m_index = 0;
}

void ScrollWidget::addWidget(QWidget* widget)
{
    m_layout->insertWidget(m_index++, widget);
    m_widgets.append(widget);
}

void ScrollWidget::removeWidget(QWidget *widget)
{
    m_layout->removeWidget(widget);
    m_index--;
    m_widgets.removeOne(widget);
}

void ScrollWidget::removeAll()
{
    foreach(QWidget* w, m_widgets)
    {
        if(w)
        {
            removeWidget(w);
        }
    }
    m_widgets.clear();
    m_index = 0;
}

int ScrollWidget::count() const
{
    return m_widgets.count();
}

QScrollBar *ScrollWidget::verticalScrollBar() const
{
    return ui->scrollArea->verticalScrollBar();
}

QScrollBar *ScrollWidget::horizontalScrollBar() const
{
    return ui->scrollArea->horizontalScrollBar();
}

void ScrollWidget::showEvent(QShowEvent *event)
{
    event->accept();
}

void ScrollWidget::hideEvent(QHideEvent *event)
{
    event->accept();
}

void ScrollWidget::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
    event->accept();
}

void ScrollWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::ActivationChange)
    {
        event->accept();
    }
}

QWidget *ScrollWidget::widgetAt(int index) const
{
    QLayoutItem * item = ui->scrollAreaWidgetContents->layout()->itemAt(index);
	if (item)
		return item->widget();
	return nullptr;
}

void ScrollWidget::onSliderValueChanged(int value)
{
    QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
    emit scrolled((double)value/scrollBar->maximum());
}

int ScrollWidget::containedHeightSum()
{
	int sum = 0;
	foreach(QWidget *w, m_widgets)
	{
		sum += w->height();
	}
	return sum;
}