/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTSHISTORYITEMWIDGET_H
#define RESULTSHISTORYITEMWIDGET_H

#include "SelectableWidget.h"

#include <QModelIndex>



namespace Ui {
class ResultsHistoryItemWidget;
}

class ResultsHistoryItemWidget : public SelectableWidget
{
    Q_OBJECT

public:
	explicit ResultsHistoryItemWidget(const QString &majorText = "", const QString &minorText = "", QWidget *parent = 0);
    ~ResultsHistoryItemWidget();

    void setMajorBold(bool enable);
    QString majorText();
    QString minorText();
	void setSelectable(bool set);
	void showOpenArrow(bool show);

    qint64 sessionId() const { return m_sessionId; }
    void setSessionId(qint64 sessionId) { m_sessionId = sessionId; }

    const QModelIndex& modelIndex() const { return m_modelIndex; }
    void setModelIndex(const QModelIndex& index) { m_modelIndex = index; }
private slots:
    void onSelected();
    void onDeselected();
    
private:
    Ui::ResultsHistoryItemWidget *ui;
    qint64 m_sessionId;
    QModelIndex m_modelIndex;
};

#endif // RESULTSHISTORYITEMWIDGET_H
