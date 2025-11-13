/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QPushButton>

class PushButton : public QPushButton
{
    Q_OBJECT

public:
    PushButton(QWidget *parent = 0);
    ~PushButton();
    
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    
signals:


private slots:

    
private:
    QString color();
    QString font_weight();
    QString font_size();
    QString border_image();
    QString border_image_hover();
    QString border_image_pressed();
    
    bool pressed;
    bool hover;
};

#endif // PUSHBUTTON_H
