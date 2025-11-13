/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INFODETAILWIDGET_H
#define INFODETAILWIDGET_H

#include <QWidget>

namespace Ui {
class InfoDetailWidget;
}

class InfoDetailWidget : public QWidget
{
    Q_OBJECT

public:
    InfoDetailWidget(QWidget *parent = 0);
    ~InfoDetailWidget();

    static InfoDetailWidget *createDetail(const QString &key, const QString &value, QWidget *parent = 0);

    void setKey(const QString &text);
    QString key() const;
    void setValue(const QString &text);
    QString value() const;
    void setStretch(int left, int right);

private:
    Ui::InfoDetailWidget *ui;
};

#endif // INFODETAILWIDGET_H
