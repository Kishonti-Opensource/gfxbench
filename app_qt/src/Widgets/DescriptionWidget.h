/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DESCRIPTIONWIDGET_H
#define DESCRIPTIONWIDGET_H

#include <QWidget>

namespace Ui {
class DescriptionWidget;
}

class DescriptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DescriptionWidget(QWidget *parent = 0);
    ~DescriptionWidget();

    void setPixmap(const QPixmap &pixmap);
    void setTitle(const QString &title);
    void setImagePath(const QString& imagePath);
    void setDescription(const QString &description, const QStringList &args = QStringList());

public slots:
    void localize();
    
signals:
    void closeClicked();

private:
    Ui::DescriptionWidget *ui;
    
    static QMap<QString, QString> m_shotPics;
    QString m_title;
    QString m_description;
    QStringList m_args;
};

#endif // DESCRIPTIONWIDGET_H
