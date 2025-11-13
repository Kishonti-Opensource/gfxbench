/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QModelIndex>
#include <QWidget>

namespace Ui {
class GroupWidget;
}

class TestWidget;

class GroupWidget : public QWidget
{
    Q_OBJECT

public:
	static GroupWidget *createSelectWidgetWithOnOffscreen(const QString &title, QWidget *parent = 0);
    static GroupWidget *createSelectWidget(const QString &title, QWidget *parent = 0);
    static GroupWidget *createResultWidgetWithOnOffscreen(const QString &title, QWidget *parent = 0);
    static GroupWidget *createResultWidget(const QString &title, QWidget *parent = 0);
    
	void showOnscreenOffscreen(bool show);
	void showCheckBox(bool show);
    void setChecked(bool checked);
    void setEnabled(bool);
    void setDisabled(bool);
    bool isChecked();
    QString title();

    const QModelIndex& modelIndex() const { return m_modelIndex; }
    void setModelIndex(const QModelIndex& index) { m_modelIndex = index; }
public slots:
    void localize();
    
public:
    void click();
    
protected:
    GroupWidget(const QString &text, QWidget *parent = 0);
    ~GroupWidget();
    
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
    
signals:
    void toggled(bool checked);
    void clicked();
    
private:
    QString getCheckCSS(bool checked);
    Ui::GroupWidget *ui;
    
    bool checked_;
    QWidget *pressed_;
    QList<TestWidget*> testWidgets_;
    QString m_title;
    
    double m_devicePixelRatio;
    QModelIndex m_modelIndex;
};

#endif // GroupWidget_H
