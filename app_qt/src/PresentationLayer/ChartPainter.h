/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CHARTPAINTER_H
#define CHARTPAINTER_H

#include "schemas/chart.h"

#include <QPainter>



class ChartPainter
{
public:
    ChartPainter(QPaintDevice* paintDevice);
    void paint();
    void clearDataSets();
    void addDataset(const QPolygonF& data, const QString& name, const QColor& color);
    void addDataset(const QPolygonF& data, const QString& name);
    void setHorizontalLabel(const QString& text);
    void setVerticalLabel(const QString& text);
    void pan(qreal dx, qreal dy);
    void zoom(const QPointF& center, qreal sx, qreal sy);
private:
    struct DataSet {
        QPolygonF data;
        QString name;
        QColor color;
    };

    QPaintDevice* mPaintDevice;
    QRect mInnerRect;
    QPointF mGridOrigin;
    QSizeF mGridResolution;
    QString mHorizontalLabel;
    QString mVerticalLabel;
    QTransform mUserTransform;
    QTransform mValueTransform;
    QVector<DataSet> mDataSets;

    qreal calculateResolution(qreal minimum);
    qreal nearestMultiple(qreal dividend, qreal divisor);
    QString formatNumber(qreal number, qreal resolution);
    void calculateInnerRect();
    void correctUserTransform();
    void calculateValueTransform();
    void calculateGridResolution();
    void paintHorizontalLabel(QPainter& painter);
    void paintVerticalLabel(QPainter& painter);
    void paintHorizontalLines(QPainter& painter);
    void paintVerticalLines(QPainter& painter);
    void paintValues(QPainter& painter);
    void paintBorder(QPainter& painter);
    void paintLegend(QPainter& painter);
};


#endif // CHARTPAINTER_H
