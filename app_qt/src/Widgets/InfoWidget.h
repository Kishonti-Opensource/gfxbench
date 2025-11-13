/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include "CursorListModel.h"

#include <QModelIndex>
#include <QWidget>

#include "ui_InfoWidget.h"

typedef QMap<QString, QVariant> Gadgets;

class InfoWidget : public QWidget
{
	Q_OBJECT

public:
	InfoWidget(const QString &title, const QIcon& icon, const QString &majorText = "", const QString &minorText= "", QWidget *parent = 0);

    QModelIndex modelIndex() const { return m_modelIndex; }
    void setModelIndex(const QModelIndex& modelIndex) { m_modelIndex = modelIndex; }
    void setMajorText(const QString &major);
    QString majorText();
    void setMinorText(const QString &minor);
    QString minorText();

    void setGadgets(const std::shared_ptr<Cursor>& cursor);
    void showOpenArrow(bool show);
    bool isOpenArrowShown() const;
    void setPrefix(const std::string &prefix);
    std::string prefix() const;
public slots:
    void localize();

protected:
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

signals:
    void clicked();

private slots:
    void click();
    void updateGadgets();

private:
	Ui::InfoWidget ui;

    CursorListModel m_model;

    QWidget *m_pressed;
    std::string m_prefix;

    QString m_title;
    QString m_majorText;
    QString m_minorText;

    QList<QLabel*> m_gadgetLabels;
    QStringList m_gadgetText;
    QModelIndex m_modelIndex;
};

#endif // INFOWIDGET_H
