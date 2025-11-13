/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ChartPainter.h"

#include <cmath>



int LABEL_HEIGHT = 20;
int LABEL_WIDTH = 50;
int PADDING = 10;
int MINIMUM_HORIZONTAL_DISTANCE = 50;
int MINIMUM_VERTICAL_DISTANCE = 30;
double MAXIMUM_ZOOM = 100.0;



ChartPainter::ChartPainter(QPaintDevice* paintDevice):
    mPaintDevice(paintDevice)
{
}



void ChartPainter::clearDataSets()
{
    mDataSets.clear();
}



void ChartPainter::addDataset(const QPolygonF& data, const QString& name, const QColor& color)
{
    mDataSets.push_back(DataSet());
    mDataSets.back().data = data;
    mDataSets.back().name = name;
    mDataSets.back().color = color;
}



void ChartPainter::addDataset(const QPolygonF& data, const QString& name)
{
    QVector<QColor> colors;
    colors.append(0xff4699b8);
    colors.append(0xff8bab4d);
    colors.append(0xffe4902e);
    addDataset(data, name, colors.at(mDataSets.size() % colors.size()));
}



void ChartPainter::setHorizontalLabel(const QString& text)
{
    mHorizontalLabel = text;
}



void ChartPainter::setVerticalLabel(const QString& text)
{
    mVerticalLabel = text;
}



void ChartPainter::pan(qreal dx, qreal dy)
{
    mUserTransform *= QTransform::fromTranslate(dx, dy);
    correctUserTransform();
}



void ChartPainter::zoom(const QPointF& center, qreal sx, qreal sy)
{
    sx = std::max(1.0 / mUserTransform.m11(), std::min(sx, MAXIMUM_ZOOM / mUserTransform.m11()));
    sy = std::max(1.0 / mUserTransform.m22(), std::min(sy, MAXIMUM_ZOOM / mUserTransform.m22()));
    QTransform temp = QTransform::fromScale(sx, sy);
    mUserTransform *= temp;
    QPointF c = center - mInnerRect.bottomLeft();
    QPointF delta = c - temp.map(c);
    mUserTransform *= QTransform::fromTranslate(delta.x(), delta.y());
    correctUserTransform();
}



void ChartPainter::paint()
{
    QPainter painter(mPaintDevice);
    calculateInnerRect();
    calculateValueTransform();
    calculateGridResolution();
    paintHorizontalLabel(painter);
    paintVerticalLabel(painter);
    paintHorizontalLines(painter);
    paintVerticalLines(painter);
    paintValues(painter);
    paintBorder(painter);
    paintLegend(painter);
}



qreal ChartPainter::calculateResolution(qreal minimum)
{
    qreal resolution = std::pow(10.0, std::ceil(std::log10(minimum)));
    return (resolution / 2.0 > minimum) ? (resolution / 2.0) : resolution;
}



qreal ChartPainter::nearestMultiple(qreal dividend, qreal divisor)
{
    return std::ceil(dividend / divisor) * divisor;
}



void ChartPainter::calculateInnerRect()
{
    mInnerRect.setTop(PADDING);
    mInnerRect.setLeft(LABEL_HEIGHT + LABEL_WIDTH);
    mInnerRect.setBottom(mPaintDevice->height() - 2 * LABEL_HEIGHT);
    mInnerRect.setRight(mPaintDevice->width() - PADDING);
}



void ChartPainter::correctUserTransform()
{
    qreal sx = std::max(1.0, std::min(mUserTransform.m11(), MAXIMUM_ZOOM));
    qreal sy = std::max(1.0, std::min(mUserTransform.m22(), MAXIMUM_ZOOM));
    mUserTransform = QTransform(
            sx,
            0.0,
            0.0,
            sy,
            std::max((1.0 - sx) * mInnerRect.width(), std::min(mUserTransform.dx(), 0.0)),
            std::min((sy - 1.0) * mInnerRect.height(), std::max(mUserTransform.dy(), 0.0)));
}



void ChartPainter::calculateValueTransform()
{
    QRectF range(0, 0, 10, 10);
    if (!mDataSets.empty()) {
        range = mDataSets.front().data.boundingRect();
    }
    for (int i = 1; i < mDataSets.size(); ++i) {
        range |= mDataSets.at(i).data.boundingRect();
    }
    qreal height = range.height();
    range.setBottom(range.bottom() + 0.1*height);
    range.setTop(range.top() - 0.1*height);

    mValueTransform = QTransform::fromTranslate(-range.left(), -range.top());
    mValueTransform *= QTransform::fromScale(
            mInnerRect.width() / range.width(),
            -mInnerRect.height() / range.height());
    mValueTransform *= mUserTransform;
    mValueTransform *= QTransform::fromTranslate(mInnerRect.left(), mInnerRect.bottom());
}



void ChartPainter::calculateGridResolution()
{
    mGridResolution = QSizeF(
            calculateResolution(MINIMUM_HORIZONTAL_DISTANCE / mValueTransform.m11()) * mValueTransform.m11(),
            calculateResolution(MINIMUM_VERTICAL_DISTANCE / -mValueTransform.m22()) * -mValueTransform.m22());
    mGridOrigin = mValueTransform.map(QPointF(0.0, 0.0));
    qreal minDivision;
    minDivision = std::remainder(mGridOrigin.x(), mGridResolution.width());
    minDivision += nearestMultiple(mInnerRect.left() - minDivision, mGridResolution.width());
    mGridOrigin.setX(minDivision);
    minDivision = std::remainder(mGridOrigin.y(), mGridResolution.height());
    minDivision += nearestMultiple(mInnerRect.bottom() - minDivision, mGridResolution.height());
    mGridOrigin.setY(minDivision - mGridResolution.height());
}



void ChartPainter::paintHorizontalLabel(QPainter& painter)
{
    painter.drawText(mInnerRect.left(), mInnerRect.bottom() + LABEL_HEIGHT,
            mInnerRect.width(), LABEL_HEIGHT, Qt::AlignCenter, mHorizontalLabel);
}



void ChartPainter::paintVerticalLabel(QPainter& painter)
{
    painter.save();
    painter.rotate(270);
    painter.drawText(-mInnerRect.height(), 0, mInnerRect.height(), LABEL_HEIGHT,
            Qt::AlignCenter, mVerticalLabel);
    painter.restore();
}



void ChartPainter::paintHorizontalLines(QPainter& painter)
{
    QTransform inverse = mValueTransform.inverted();
    double resolution = mGridResolution.height() / -mValueTransform.m22();
    int decimals = static_cast<int>(std::max(0.0, 1.0 - std::log10(resolution)));
    painter.save();
    for (qreal y = mGridOrigin.y(); y > mInnerRect.top(); y -= mGridResolution.height()) {
        painter.setPen(Qt::lightGray);
        painter.drawLine(mInnerRect.left(), y, mInnerRect.right(), y);
        painter.setPen(Qt::black);
        qreal position = inverse.map(QPointF(0, y)).y();
        if (std::abs(position) < std::pow(10.0, -decimals)) {
            position = 0.0; // No negative zero
        }
        QString text = QString::number(position, 'f', decimals);
        painter.drawText(
                LABEL_HEIGHT, y - LABEL_HEIGHT / 2,
                LABEL_WIDTH - PADDING, LABEL_HEIGHT,
                Qt::AlignVCenter | Qt::AlignRight, text);
    }
    painter.restore();
}



void ChartPainter::paintVerticalLines(QPainter& painter)
{
    QTransform inverse = mValueTransform.inverted();
    double resolution = mGridResolution.width() / mValueTransform.m11();
    int decimals = static_cast<int>(std::max(0.0, 1.0 - std::log10(resolution)));
    painter.save();
    for (qreal x = mGridOrigin.x(); x <= mInnerRect.right(); x += mGridResolution.width()) {
        painter.setPen(Qt::lightGray);
        painter.drawLine(x, mInnerRect.top(), x, mInnerRect.bottom());
        painter.setPen(Qt::black);
        qreal position = inverse.map(QPointF(x, 0)).x();
        if (std::abs(position) < std::pow(10.0, -decimals)) {
            position = 0.0; // No negative zero
        }
        QString text = QString::number(position, 'f', decimals);
        painter.drawText(
                x - MINIMUM_HORIZONTAL_DISTANCE / 2, mInnerRect.bottom() + 5,
                MINIMUM_HORIZONTAL_DISTANCE, LABEL_HEIGHT,
                Qt::AlignHCenter | Qt::AlignTop, text);
    }
    painter.restore();
}



void ChartPainter::paintValues(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(mInnerRect);

    QPen pen;
    pen.setWidth(2);
    for (int i = 0; i < mDataSets.size(); ++i) {
        const DataSet& dataSet = mDataSets.at(i);
        pen.setColor(dataSet.color);
        painter.setPen(pen);
        painter.drawPolyline(mValueTransform.map(dataSet.data));
    }

    painter.restore();
}



void ChartPainter::paintBorder(QPainter& painter)
{
    painter.setPen(Qt::black);
    painter.drawRect(mInnerRect.x(), mInnerRect.y(), mInnerRect.width() - 1,
        mInnerRect.height() - 1);
}



void ChartPainter::paintLegend(QPainter& painter)
{
    painter.save();
    painter.setClipRect(mInnerRect);

    for (int i = 0; i < mDataSets.size(); ++i) {
        const DataSet& dataSet = mDataSets.at(i);
        QRect legendRect(
                mInnerRect.right() - 120,
                mInnerRect.top() + PADDING + i*LABEL_HEIGHT + 2,
                20,
                LABEL_HEIGHT - 4);
        QRect smallRect(legendRect.left() + 2, legendRect.top() + 2,
                legendRect.width() - 3, legendRect.height() - 3);

        painter.setPen(Qt::gray);
        painter.setBrush(Qt::white);
        painter.drawRect(legendRect);
        painter.setPen(Qt::NoPen);
        painter.setBrush(dataSet.color);
        painter.drawRect(smallRect);

        painter.setPen(dataSet.color);
        painter.drawText(
                legendRect.right() + PADDING, legendRect.top(), 120, legendRect.height(),
                Qt::AlignVCenter, dataSet.name);
    }

    painter.restore();
}
