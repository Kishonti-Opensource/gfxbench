/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QWidget>

namespace Ui {
class ScrollWidget;
}

class QBoxLayout;
class QScrollBar;
class QSpacerItem;

class ScrollWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScrollWidget(QWidget *parent = 0);
    ~ScrollWidget();

    //remove and delete widgets
    void clear();
    void addWidget(QWidget* widget);
    //remove widgets
    void removeWidget(QWidget* widget);
    void removeAll();
    int count() const;
    QScrollBar *verticalScrollBar() const;
    QScrollBar *horizontalScrollBar() const;
    QBoxLayout *layout() const { return m_layout; }
    QWidget *widgetAt(int index) const;

	int containedHeightSum();
    
protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);
    
protected:
    Ui::ScrollWidget *ui;
    
    QBoxLayout *m_layout;
    QList<QWidget*> m_widgets;
    int m_index;
    QSpacerItem *m_spacer;

private slots:
    void onSliderValueChanged(int value);
    
signals:
    void scrolled(double value);
    void resized(QSize size);
};

#endif // SCROLLWIDGET_H
