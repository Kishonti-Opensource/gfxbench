/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ChartWidget.h"

#include "ChartPainter.h"

#include <QMouseEvent>
#include <QWheelEvent>

#include <cmath>



ChartWidget::ChartWidget(QWidget* parent) :
    QWidget(parent),
    mPainter(this)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    bool success = QObject::connect(&mChartModel, &CursorListModel::modelReset,
            this, &ChartWidget::update);
    assert(success);

    success = QObject::connect(&mChartModel, &CursorListModel::dataChanged,
            this, &ChartWidget::update);
    assert(success);

    Q_UNUSED(success)
}



void ChartWidget::setHorizontalLabel(const QString& text)
{
    mPainter.setHorizontalLabel(text);
}



void ChartWidget::setVerticalLabel(const QString& text)
{
    mPainter.setVerticalLabel(text);
}



void ChartWidget::setCursor(const std::shared_ptr<Cursor>& cursor)
{
    mChartModel.setCursor(cursor);
}



void ChartWidget::addDataset(const QPolygonF& data, const QString& name)
{
    mPainter.addDataset(data, name);
}



void ChartWidget::paintEvent(QPaintEvent* event)
{
    mPainter.paint();
    event->accept();
}



void ChartWidget::mousePressEvent(QMouseEvent* event)
{
    mMouseStart = event->pos();
    event->accept();
}



void ChartWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPointF delta = event->pos() - mMouseStart;
    mPainter.pan(delta.x(), delta.y());
    mMouseStart = event->pos();
    event->accept();
    repaint();
}



void ChartWidget::wheelEvent(QWheelEvent* event)
{
    double scale = std::pow(2.0, event->angleDelta().y() * 0.001);
    if (event->modifiers() == Qt::ShiftModifier) {
        mPainter.zoom(event->posF(), 1.0, scale);
    } else if (event->modifiers() == Qt::ControlModifier) {
        mPainter.zoom(event->posF(), scale, 1.0);
    } else {
        mPainter.zoom(event->posF(), scale, scale);
    }
    event->accept();
    repaint();
}



void ChartWidget::update()
{
    QByteArray domainBlob;
    if (mChartModel.rowCount() >= 1) {
        QModelIndex modelIndex = mChartModel.index(0);
        mPainter.setHorizontalLabel(modelIndex.data(Kishonti::MetricRole).toString());
        domainBlob = modelIndex.data(Kishonti::DataRole).toByteArray();
    }
    QDataStream domainStream(domainBlob);
    domainStream.setByteOrder(QDataStream::LittleEndian);
    for (int i = 1; i < mChartModel.rowCount(); ++i) {
        QModelIndex modelIndex = mChartModel.index(i);
        mPainter.setVerticalLabel(modelIndex.data(Kishonti::MetricRole).toString());

        QString name = modelIndex.data(Qt::DisplayRole).toString();
        QPolygonF data;
        QDataStream sampleStream(modelIndex.data(Kishonti::DataRole).toByteArray());
        sampleStream.setByteOrder(QDataStream::LittleEndian);
        domainStream.device()->seek(0);
        while (!domainStream.atEnd()) {
            double x;
            domainStream >> x;
            double y;
            sampleStream >> y;
            data.append(QPointF(x, y));
        }
        addDataset(data, name);
    }
}
