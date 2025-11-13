/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SELECTABLEWIDGET_H
#define SELECTABLEWIDGET_H

#include <QWidget>

class SelectableWidget : public QWidget
{
    Q_OBJECT
    
public:
    SelectableWidget(QWidget *parent = 0);
    
    void setSelected(bool set);
    void setPressed(bool set);
    bool isSelected();
    bool isPressed();
    
public slots:
    void select();
    void deselect();
    
protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    
signals:
    void selected();
    void deselected();
    
private:
    bool selected_;
    bool pressed_;
};

#endif // SCROLLWIDGET_H
