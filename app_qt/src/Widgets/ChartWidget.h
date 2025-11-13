/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "ChartPainter.h"
#include "CursorListModel.h"

#include <QWidget>



class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    ChartWidget(QWidget* parent = 0);
    void addDataset(const QPolygonF& data, const QString& name, const QColor& color);
    void addDataset(const QPolygonF& data, const QString& name);
    void setHorizontalLabel(const QString& text);
    void setVerticalLabel(const QString& text);
    void setCursor(const std::shared_ptr<Cursor>& cursor);
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
private slots:
    void update();
private:
    QPoint mMouseStart;
    ChartPainter mPainter;
    CursorListModel mChartModel;
};



#endif // CHARTWIDGET_H
