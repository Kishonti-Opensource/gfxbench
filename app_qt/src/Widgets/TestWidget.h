/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include "SelectableWidget.h"

#include <QWidget>
#include <QMap>
#include <QModelIndex>

namespace Ui {
class TestWidget;
}

class TestWidget : public SelectableWidget
{
    Q_OBJECT

public:
    
    static TestWidget *createSelectWidgetWithOffscreen(const QString &title, const QString &description, const QStringList args = QStringList(), QWidget *parent = 0);
    static TestWidget *createSelectWidget(const QString &title, const QString &description, const QStringList args = QStringList(), QWidget *parent = 0);
    static TestWidget *createResultWidgetWithOffscreen(const QString &title, const QString &description, const QStringList args = QStringList(), QWidget *parent = 0);
    static TestWidget *createResultWidget(const QString &title, const QString &description, const QStringList args = QStringList(), QWidget *parent = 0);
    static TestWidget *createSelectableWidget(const QString &title, const QString &description, const QStringList args = QStringList(), QWidget *parent = 0);
    
   	bool isOscreenChecked() { return m_checkedOnscreen; }
    void setOnscreenChecked(bool checked);
	bool isOffscreenChecked() { return m_checkedOffscreen; }
    void setOffscreenChecked(bool checked);
    void setTestId(const std::string &testId);
    std::string testId() const { return m_onscreen; }
    void setSubresultId(int id);
    int subresultId() { return m_subresultId; }
	void setOnscreenId(const std::string &testId);
    std::string onscreenId() { return m_onscreen; }
	void setOffscreenId(const std::string &testId);
    std::string offscreenId() { return m_offscreen; }
	std::vector<std::string> testIdList();
    void setTitle(const QString &title);
    void setRawTitle(const QString &title);
    QString title() const;
    void setDescription(const QString &description);
    void setRawDescription(const QString &description);
    QString description() const;
    void setIcon(const QIcon& icon);
    void setArgs(const QStringList &args);
    QStringList args() const;
    QString formattedDescription() const;
    void setIncompatibleText(const QString &text);
    QString incompatibleText() const;
    void setChecked(bool checked);
    bool isCheckedAny();
    bool isCheckedBoth();
    void showCheckOffscreenPanel(bool show);
    void showResultOffscreenPanel(bool show);
    bool isCheckOffscreenPanelShown() const;
    bool isResultOffscreenPanelShown() const;
    
    void showCheckPanel(bool show);
    void showResultPanel(bool show);
    void showDescription(bool show);
    
	void setOnscreenText(const QString &major, const QString &minor = QString());
	void setOffscreenText(const QString &major, const QString &minor = QString());
    QString onscreenMajor();
    QString onscreenMinor();
    QString offscreenMajor();
    QString offscreenMinor();
	void showOnscreenOpenArrow(bool show);
	void showOffscreenOpenArrow(bool show);
    bool isOnscreenOpenArrowShown();
    bool isOffscreenOpenArrowShown();
    
    void setSelectable(bool set);
    bool isSelectable() { return m_selectable; }
    
    const QModelIndex& modelIndex() const { return m_modelIndex; }
    void setModelIndex(const QModelIndex& index) { m_modelIndex = index; }
public slots:
    void localize();
	void clickCheckOnscreen();
	void clickCheckOffscreen();
    void clickResultOnscreen();
    void clickResultOffscreen();
    void clickDescription();
    
protected:
    TestWidget(const QString &title, const QString &description, const QStringList &args = QStringList(), QWidget *parent = 0);
    ~TestWidget();
    
	void mouseReleaseEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent * event);
    void showEvent(QShowEvent * event);
    void changeEvent(QEvent *event);

private slots:
    void onSelected();
    void onDeselected();

signals:
    void toggled(bool checked);
    void clicked();
    void descriptionClicked();
	void resultClicked(const QModelIndex &index);
    
private:
    Ui::TestWidget *ui;
    
    std::string m_onscreen, m_offscreen;
    bool m_checkedOnscreen, m_checkedOffscreen;
    QWidget *m_pressed;
    
    QString m_title, m_description, m_incompatible;
    QStringList m_args;
    
    bool m_selectable;
    int m_subresultId;
    
    double m_devicePixelRatio;
    QModelIndex m_modelIndex;
};

#endif // TESTWIDGET_H
