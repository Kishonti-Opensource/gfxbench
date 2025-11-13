/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "SelectableScrollWidget.h"
#include <QDebug>

SelectableScrollWidget::SelectableScrollWidget(QWidget *parent) :
    ScrollWidget(parent)
{
    current_ = 0;
    previous_ = 0;
}

SelectableScrollWidget::~SelectableScrollWidget()
{
}

void SelectableScrollWidget::addWidget(SelectableWidget* widget)
{
    ScrollWidget::addWidget(widget);
    selectableWidgets_.append(widget);
    connect(widget, SIGNAL(selected()), this, SLOT(onSelect()));
}

void SelectableScrollWidget::addWidget(QWidget* widget)
{
    ScrollWidget::addWidget(widget);
}

void SelectableScrollWidget::clear()
{
    ScrollWidget::clear();
    selectableWidgets_.clear();
}

void SelectableScrollWidget::setCurrentItem(SelectableWidget* widget)
{
    if(selectableWidgets_.contains(widget))
    {
        widget->select();
    }
}

void SelectableScrollWidget::setCurrentRow(int row)
{
    if(row >= 0 && row < selectableWidgets_.count())
    {
        selectableWidgets_[row]->select();
    }
    else if(selectableWidgets_.count() >= 1)
    {
        selectableWidgets_[0]->select();
    }
}

SelectableWidget *SelectableScrollWidget::currentItem()
{
    return current_;
}

SelectableWidget *SelectableScrollWidget::item(int row)
{
    if(row >= 0 && row < selectableWidgets_.count())
    {
        return selectableWidgets_[row];
    }
    return 0;
}

void SelectableScrollWidget::onSelect()
{
    previous_ = current_;
    current_ = (SelectableWidget*) sender();
    emit currentItemChanged(current_, previous_);
    int row = -1;
    for(int i(0); i < selectableWidgets_.count(); i++)
    {
        if(selectableWidgets_[i] == current_)
        {
            row = i;
        }
    }
    emit currentRowChanged(row);
    foreach(SelectableWidget *sw, selectableWidgets_)
    {
        if(sw != current_)
        {
            sw->deselect();
        }
    }
}