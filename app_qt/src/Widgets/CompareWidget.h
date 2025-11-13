/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPAREWIDGET_H
#define COMPAREWIDGET_H

#include <QModelIndex>
#include <QWidget>



namespace Ui {
class CompareWidget;
}

class CompareWidget : public QWidget
{
    Q_OBJECT

public:
    CompareWidget(QWidget *parent = 0);
    ~CompareWidget();
    static CompareWidget *createCompareWidget(
            const QString &deviceName,
            const QString &manufacturer,
            double score,
            double maxScore,
            double fps,
            const QString &metric,
            const QIcon &icon,
            QWidget *parent = 0);

    void setDevice(const QString &deviceName, const QString &manufacturer);
    void setScore(double points, double fps);
    void setMetric(const QString &metric);
    void setMaxScore(double maxScore);
    void setDeviceId(int deviceId);
    void setToNull();
    void setWidgetIndex(int index);

    QModelIndex modelIndex() const { return m_modelIndex; }
    void setModelIndex(const QModelIndex& modelIndex) { m_modelIndex = modelIndex; }

    static void setDecimals(int decimals);

    double score() const { return m_score; }
    QString deviceName() const { return m_deviceName; }
    QString manufacturer() const { return m_manufacturer; }
    QString fullName() const { return m_manufacturer + " " + m_deviceName; }
    int deviceId() const { return m_deviceId; }

public slots:
    void localize();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);

signals:
    void clicked();

private:
    Ui::CompareWidget *ui;

    static int s_decimals;
    static double s_devicePixelRatio;

    QString m_deviceName;
    QString m_manufacturer;
	double m_score;
    double m_maxScore;
    double m_fps;
	QString m_metric;
    QString m_pixUrl;
	double m_percent;
    int m_deviceId;
    int m_widgetIndex;
    QString m_barColor;
    QString m_scoreString;
    QModelIndex m_modelIndex;

    bool m_pressed;
};

#endif // COMPAREWIDGET_H
