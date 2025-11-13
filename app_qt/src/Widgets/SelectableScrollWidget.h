/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SELECTABLESCROLLWIDGET_H
#define SELECTABLESCROLLWIDGET_H

#include "ScrollWidget.h"
#include "SelectableWidget.h"

class QBoxLayout;
class QScrollBar;
class QSpacerItem;

class SelectableScrollWidget : public ScrollWidget
{
    Q_OBJECT

public:
    explicit SelectableScrollWidget(QWidget *parent = 0);
    ~SelectableScrollWidget();
    
    void addWidget(SelectableWidget* widget);
    void addWidget(QWidget* widget);
    void clear();
    
    void setCurrentItem(SelectableWidget* widget);
    void setCurrentRow(int row);
    SelectableWidget *currentItem();
    SelectableWidget *item(int row);

signals:
    void currentItemChanged(SelectableWidget *current, SelectableWidget *previous);
    void currentRowChanged(int currentRow);

private slots:
    void onSelect();
    
private:
    SelectableWidget *current_, *previous_;
    QVector<SelectableWidget*> selectableWidgets_;
};

#endif // SCROLLWIDGET_H
