/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "PushButton.h"
#include <QPainter>
#include <QDebug>

PushButton::PushButton(QWidget *parent) :
    QPushButton(parent), pressed(false), hover(false)
{
}

PushButton::~PushButton()
{
}

void PushButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QRect drawRect = rect();
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    if(pressed && hover)
    {
        painter.drawPixmap(drawRect, QPixmap(border_image_pressed()));
    }
    else if(!pressed && hover)
    {
        painter.drawPixmap(drawRect, QPixmap(border_image_hover()));
    }
    else
    {
        painter.drawPixmap(drawRect, QPixmap(border_image()));
    }
    QPen pen;
    pen.setColor(QColor(color()));
    pen.setWidth(font_size().toInt());
    painter.setPen(pen);
    painter.drawText(drawRect, Qt::AlignCenter | Qt::TextWordWrap, text());
}

void PushButton::mousePressEvent(QMouseEvent *event)
{
    pressed = true;
    QPushButton::mousePressEvent(event);
}

void PushButton::mouseReleaseEvent(QMouseEvent *event)
{
    pressed = false;
    QPushButton::mouseReleaseEvent(event);
}

void PushButton::enterEvent(QEvent *event)
{
    hover = true;
    QPushButton::enterEvent(event);
}

void PushButton::leaveEvent(QEvent *event)
{
    hover = false;
    pressed = false;
    QPushButton::leaveEvent(event);
}

QString PushButton::color()
{
    const QString &css = styleSheet();
    QString tag = "color: ";
    int start = css.indexOf(tag) + tag.length();
    int end = css.indexOf(";", start);
    return css.mid(start, end-start);
}

QString PushButton::font_weight()
{
    const QString &css = styleSheet();
    QString tag = "font-weight: ";
    int start = css.indexOf(tag) + tag.length();
    int end = css.indexOf(";", start);
    return css.mid(start, end-start);
}

QString PushButton::font_size()
{
    const QString &css = styleSheet();
    QString tag = "font-size: ";
    int start = css.indexOf(tag) + tag.length();
    int end = css.indexOf("px", start);
    return css.mid(start, end-start);
}

QString PushButton::border_image()
{
    const QString &css = styleSheet();
    QString tag = "border-image: url(";
    int start = css.indexOf(tag) + tag.length();
    int end = css.indexOf(")", start);
    return css.mid(start, end-start);
}

QString PushButton::border_image_hover()
{
    const QString &css = styleSheet();
    QString tag = "border-image: url(";
    int from = css.indexOf("hover");
    int start = css.indexOf(tag, from) + tag.length();
    int end = css.indexOf(")", start);
    return css.mid(start, end-start);
}

QString PushButton::border_image_pressed()
{
    const QString &css = styleSheet();
    QString tag = "border-image: url(";
    int from = css.indexOf("pressed");
    int start = css.indexOf(tag, from) + tag.length();
    int end = css.indexOf(")", start);
    return css.mid(start, end-start);
}