/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "SelectableWidget.h"
#include <QDebug>
#include <QMouseEvent>

SelectableWidget::SelectableWidget(QWidget *parent)
: QWidget(parent), selected_(false), pressed_(false)
{
    setMouseTracking(true);
}

void SelectableWidget::setSelected(bool set)
{
    selected_ = set;
}

void SelectableWidget::setPressed(bool set)
{
    pressed_ = set;
}

bool SelectableWidget::isSelected()
{
    return selected_;
}

bool SelectableWidget::isPressed()
{
    return pressed_;
}

void SelectableWidget::select()
{
    selected_ = true;
    emit selected();
}

void SelectableWidget::deselect()
{
    selected_ = false;
    emit deselected();
}

void SelectableWidget::mousePressEvent(QMouseEvent *event)
{
    if(this->rect().contains(event->pos()))
    {
        pressed_ = true;
    }
}

void SelectableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(pressed_ && this->rect().contains(event->pos()))
    {
        select();
    }
    pressed_ = false;
}
