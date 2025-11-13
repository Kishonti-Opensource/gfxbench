/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "TestWidget.h"
#include "ui_TestWidget.h"

#include "Dictionary.h"

#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QPainter>
#include <QIcon>

#include <iostream>

TestWidget::TestWidget(const QString &title, const QString &description, const QStringList &args, QWidget *parent) :
    SelectableWidget(parent),
    ui(new Ui::TestWidget),
    m_checkedOnscreen(true),
    m_checkedOffscreen(true),
    m_title(title),
    m_description(description),
    m_incompatible(""),
    m_args(args),
    m_selectable(false),
    m_subresultId(-1)
{
    ui->setupUi(this);

    m_devicePixelRatio = 1.0;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_devicePixelRatio = devicePixelRatio();
#endif

    if(m_devicePixelRatio > 1.0)
    {
        QString arrowCss =
        "QLabel { border-image: url(:/retina_arrow.png) 0 0 0 0 stretch stretch; }";
        ui->openArrowLabel->setStyleSheet(arrowCss);
        ui->onscreenArrowLabel->setStyleSheet(arrowCss);
        ui->offscreenArrowLabel->setStyleSheet(arrowCss);
    }

    ui->incompatibleLabel->hide();

    m_pressed = 0;

    setOnscreenChecked(m_checkedOnscreen);
    setOffscreenChecked(m_checkedOffscreen);

    localize();
}

TestWidget *TestWidget::createSelectWidgetWithOffscreen(const QString &title, const QString &description, const QStringList args, QWidget *parent)
{
    TestWidget* w = new TestWidget(title, description, args, parent);
    w->showCheckOffscreenPanel(true);
    w->showResultPanel(false);
    w->showCheckPanel(true);
    return w;
}

TestWidget *TestWidget::createSelectWidget(const QString &title, const QString &description, const QStringList args, QWidget *parent)
{
    TestWidget* w = new TestWidget(title, description, args, parent);
    w->showCheckOffscreenPanel(false);
    w->showResultPanel(false);
    w->showCheckPanel(true);
    return w;
}

TestWidget *TestWidget::createResultWidgetWithOffscreen(const QString &title, const QString &description, const QStringList args, QWidget *parent)
{
    TestWidget* w = new TestWidget(title, description, args, parent);
    w->showCheckPanel(false);
    w->showResultPanel(true);
    w->showResultOffscreenPanel(true);
    return w;
}

TestWidget *TestWidget::createResultWidget(const QString &title, const QString &description, const QStringList args, QWidget *parent)
{
    TestWidget* w = new TestWidget(title, description, args, parent);
    w->showCheckPanel(false);
    w->showResultPanel(true);
    w->showResultOffscreenPanel(false);
    return w;
}

TestWidget *TestWidget::createSelectableWidget(const QString &title, const QString &description, const QStringList args, QWidget *parent)
{
    TestWidget* w = new TestWidget(title, description, args, parent);
    w->setSelectable(true);
    w->showCheckPanel(false);
    w->showResultPanel(false);
    w->ui->verticalLine0->setVisible(false);
    return w;
}

TestWidget::~TestWidget()
{
    delete ui;
}

void TestWidget::localize()
{
    setTitle(m_title);
    setDescription(m_description);
    setIncompatibleText(m_incompatible);
}

std::vector<std::string> TestWidget::testIdList()
{
    std::vector<std::string> ds;
    if(isEnabled())
    {
        if(m_checkedOnscreen && !m_onscreen.empty())
        {
            ds.push_back(m_onscreen);
        }
        if(m_checkedOffscreen && !m_offscreen.empty())
        {
            ds.push_back(m_offscreen);
        }
    }
	return ds;
}

void TestWidget::setTitle(const QString &title)
{
    m_title = title;
    QString titleText = ui->titleLabel->fontMetrics().elidedText(QtUI::dict(m_title), Qt::ElideRight, qRound(1.0*ui->titleLabel->width()));
    ui->titleLabel->setText(titleText);
}

void TestWidget::setRawTitle(const QString& title)
{
    ui->titleLabel->setText(title);
}

QString TestWidget::title() const
{
    return m_title;
}

void TestWidget::setDescription(const QString &description)
{
    m_description = description;
    QString descText = ui->descriptionLabel->fontMetrics().elidedText(QtUI::dict(m_description, m_args), Qt::ElideRight, qRound(1.75*ui->descriptionLabel->width()));
    ui->descriptionLabel->setText(descText);
}

void TestWidget::setRawDescription(const QString &description)
{
    ui->descriptionLabel->setText(description);
}

QString TestWidget::description() const
{
    return m_description;
}

void TestWidget::setIcon(const QIcon& icon)
{
    ui->testIconLabel->setPixmap(icon.pixmap(ui->testIconLabel->size()));
}

void TestWidget::setArgs(const QStringList &args)
{
    m_args = args;
}

QStringList TestWidget::args() const
{
    return m_args;
}

QString TestWidget::formattedDescription() const
{
    return QtUI::dict(m_description, m_args);
}

void TestWidget::setIncompatibleText(const QString &text)
{
    m_incompatible = text;
    ui->incompatibleLabel->setText(QtUI::dict(m_incompatible));
}

QString TestWidget::incompatibleText() const
{
    return m_incompatible;
}

void TestWidget::setTestId(const std::string &testId)
{
    setOnscreenId(testId);
}

void TestWidget::setSubresultId(int id)
{
    m_subresultId = id;
}

void TestWidget::setOnscreenId(const std::string &testId)
{
    m_onscreen = testId;
    QString pic = QString(testId.c_str());
}

void TestWidget::setOffscreenId(const std::string &testId)
{
    m_offscreen = testId;
    QString pic = QString(testId.c_str()).remove("_off");
}

void TestWidget::setOnscreenChecked(bool checked)
{
    if (!isEnabled()) {
        return;
    }
    m_checkedOnscreen = checked;

    QString ss = QString("QWidget#onscreenCheckWidget { background-color: %1 } QLabel#onscreenCheckLabel { border-image: url(:/%2) 0 0 0 0 stretch stretch; }");
    QString color, checkedPic;
    if(checked) {
        color = property("selectColor").toString();
        checkedPic = property("checkedPic").toString();
    } else {
        color = property("unselectColor").toString();
        checkedPic = property("uncheckedPic").toString();
    }
    if(m_devicePixelRatio > 1.0)
    {
        checkedPic.prepend("retina_");
    }
    ss = ss.arg(color, checkedPic);
    ui->onscreenCheckWidget->setStyleSheet(ss);
}

void TestWidget::setOffscreenChecked(bool checked)
{
    if (!isEnabled()) {
        return;
    }
    m_checkedOffscreen = checked;

    QString ss = QString("QWidget#offscreenCheckWidget { background-color: %1 } QLabel#offscreenCheckLabel { border-image: url(:/%2) 0 0 0 0 stretch stretch; }");

    QString color, checkedPic;
    if(checked)
    {
        color = property("selectColor").toString();
        checkedPic = property("checkedPic").toString();
    }
    else
    {
        color = property("unselectColor").toString();
        checkedPic = property("uncheckedPic").toString();
    }
    if(m_devicePixelRatio > 1.0)
    {
        checkedPic.prepend("retina_");
    }
    ss = ss.arg(color, checkedPic);
    ui->offscreenCheckWidget->setStyleSheet(ss);
}

void TestWidget::setChecked(bool checked)
{
    if(isEnabled())
	{
        if(isCheckOffscreenPanelShown())
        {
            setOnscreenChecked(checked);
            setOffscreenChecked(checked);
        }
        else
        {
            setOnscreenChecked(checked);
        }
	}
}

bool TestWidget::isCheckedAny()
{
    if(isCheckOffscreenPanelShown())
    {
        return (ui->onscreenCheckLabel->isEnabled() && m_checkedOnscreen) || (ui->offscreenCheckLabel->isEnabled() && m_checkedOffscreen);
    }
    else
    {
        return (ui->onscreenCheckLabel->isEnabled() && m_checkedOnscreen);
    }
}

bool TestWidget::isCheckedBoth()
{
    if(isCheckOffscreenPanelShown())
    {
        return (ui->onscreenCheckLabel->isEnabled() && m_checkedOnscreen) && (ui->offscreenCheckLabel->isEnabled() && m_checkedOffscreen);
    }
    else
    {
        return (ui->onscreenCheckLabel->isEnabled() && m_checkedOnscreen);
    }
}

void TestWidget::clickCheckOnscreen()
{
    setOnscreenChecked(!m_checkedOnscreen);
    emit toggled(true);
	emit clicked();
}

void TestWidget::clickCheckOffscreen()
{
    setOffscreenChecked(!m_checkedOffscreen);
    emit toggled(false);
	emit clicked();
}

void TestWidget::clickResultOnscreen()
{
    emit resultClicked(m_modelIndex.sibling(m_modelIndex.row(), 0));
}

void TestWidget::clickResultOffscreen()
{
    emit resultClicked(m_modelIndex.sibling(m_modelIndex.row(), 1));
}

void TestWidget::clickDescription()
{
	emit descriptionClicked();
    if(m_selectable)
    {
        emit select();
    }
}

void TestWidget::showCheckOffscreenPanel(bool show)
{
    ui->offscreenCheckWidget->setVisible(show);
    ui->verticalLine1->setVisible(show);
}

void TestWidget::showResultOffscreenPanel(bool show)
{
    ui->offscreenResultWidget->setVisible(show);
    ui->verticalLine3->setVisible(show);
}

bool TestWidget::isCheckOffscreenPanelShown() const
{
    return ui->offscreenCheckWidget->isVisible();
}

bool TestWidget::isResultOffscreenPanelShown() const
{
    return ui->offscreenResultWidget->isVisible();
}

void TestWidget::showCheckPanel(bool show)
{
    ui->checkPanel->setVisible(show);
}

void TestWidget::showResultPanel(bool show)
{
    ui->resultPanel->setVisible(show);
}
void TestWidget::showDescription(bool show)
{
    ui->descriptionLabel->setVisible(show);
}

void TestWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui->verticalLine0->width();
        QRect descRect = ui->descriptionPanel->rect();
        QRect onRect, offRect;
        if(ui->checkPanel->isVisible())
        {
            onRect = QRect(QPoint(ui->descriptionPanel->width() + line, 0), ui->onscreenCheckWidget->size());
            offRect = QRect(QPoint(onRect.x() + ui->onscreenCheckWidget->width() + line, 0), ui->offscreenCheckWidget->size());
        }
        else if(ui->resultPanel->isVisible())
        {
            onRect = QRect(QPoint(ui->descriptionPanel->width() + line, 0), ui->onscreenResultWidget->size());
            offRect = QRect(QPoint(onRect.x() + ui->onscreenResultWidget->width() + line, 0), ui->offscreenResultWidget->size());
        }

        if(onRect.contains(event->pos()))
        {
            if(ui->checkPanel->isVisible())
                m_pressed = ui->onscreenCheckWidget;
            else
                m_pressed = ui->onscreenResultWidget;
        }
        else if(offRect.contains(event->pos()))
        {
            if(ui->checkPanel->isVisible())
                m_pressed = ui->offscreenCheckWidget;
            else
                m_pressed = ui->offscreenResultWidget;
        }
        else if(descRect.contains(event->pos()))
        {
            m_pressed = ui->descriptionPanel;
        }
    }
}

void TestWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui->verticalLine0->width();
        QRect descRect = ui->descriptionPanel->rect();
        QRect onRect, offRect;
        if(ui->checkPanel->isVisible())
        {
            onRect = QRect(QPoint(ui->descriptionPanel->width() + line, 0), ui->onscreenCheckWidget->size());
            offRect = QRect(QPoint(onRect.x() + ui->onscreenCheckWidget->width() + line, 0), ui->offscreenCheckWidget->size());
        }
        else if(ui->resultPanel->isVisible())
        {
            onRect = QRect(QPoint(ui->descriptionPanel->width() + line, 0), ui->onscreenResultWidget->size());
            offRect = QRect(QPoint(onRect.x() + ui->onscreenResultWidget->width() + line, 0), ui->offscreenResultWidget->size());
        }
        if(descRect.contains(event->pos()) && m_pressed == ui->descriptionPanel)
        {
            this->clickDescription();
        }
        else if(onRect.contains(event->pos()) && m_pressed == ui->onscreenCheckWidget)
        {
            this->clickCheckOnscreen();
        }
        else if(offRect.contains(event->pos()) && m_pressed == ui->offscreenCheckWidget)
        {
            this->clickCheckOffscreen();
        }
        else if(onRect.contains(event->pos()) && m_pressed == ui->onscreenResultWidget)
        {
            this->clickResultOnscreen();
        }
        else if(offRect.contains(event->pos()) && m_pressed == ui->offscreenResultWidget)
        {
            this->clickResultOffscreen();
        }
        m_pressed = 0;
    }
}

void TestWidget::showEvent(QShowEvent*)
{
    setTitle(m_title);
    setDescription(m_description);
}

void TestWidget::resizeEvent(QResizeEvent*)
{
    setTitle(m_title);
    setDescription(m_description);
}

void TestWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::EnabledChange)
    {
        ui->incompatibleLabel->setVisible(!isEnabled());
        showCheckOffscreenPanel(isEnabled() && ui->offscreenCheckLabel->isVisible());
        ui->onscreenCheckLabel->setVisible(isEnabled() && ui->onscreenCheckLabel->isEnabled());
        ui->offscreenCheckLabel->setVisible(isEnabled() && ui->offscreenCheckLabel->isEnabled());
    }
}

void TestWidget::onSelected()
{
    QString ss = QString("QWidget { background-color: %1; }").arg(property("selectColor").toString());
    ui->descriptionPanel->setStyleSheet(ss);
}

void TestWidget::onDeselected()
{
    QString ss = QString("QWidget { background-color: %1; }").arg(property("unselectColor").toString());
    ui->descriptionPanel->setStyleSheet(ss);
}


void TestWidget::setOnscreenText(const QString &major, const QString &minor)
{
    ui->onscreenMajorLabel->setText(QtUI::dict(major));
    ui->onscreenMinorLabel->setText(QtUI::dict(minor));
	if(!minor.isEmpty())
	{
		ui->onscreenMajorLabel->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
		ui->onscreenMinorLabel->show();
	}
	else
	{
		ui->onscreenMajorLabel->setAlignment(Qt::AlignCenter);
		ui->onscreenMinorLabel->hide();
	}
}

void TestWidget::setOffscreenText(const QString &major, const QString &minor)
{
    ui->offscreenMajorLabel->setText(QtUI::dict(major));
    ui->offscreenMinorLabel->setText(QtUI::dict(minor));
	if(!minor.isEmpty())
	{
		ui->offscreenMajorLabel->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
		ui->offscreenMinorLabel->show();
	}
	else
	{
		ui->offscreenMajorLabel->setAlignment(Qt::AlignCenter);
		ui->offscreenMinorLabel->hide();
	}
}

QString TestWidget::onscreenMajor()
{
	return ui->onscreenMajorLabel->text();
}

QString TestWidget::onscreenMinor()
{
	return ui->onscreenMinorLabel->text();
}

QString TestWidget::offscreenMajor()
{
	return ui->offscreenMajorLabel->text();
}

QString TestWidget::offscreenMinor()
{
	return ui->offscreenMinorLabel->text();
}

void TestWidget::showOnscreenOpenArrow(bool show)
{
	ui->onscreenArrowLabel->setVisible(show);
}

void TestWidget::showOffscreenOpenArrow(bool show)
{
	ui->offscreenArrowLabel->setVisible(show);
}

bool TestWidget::isOnscreenOpenArrowShown()
{
    return ui->onscreenArrowLabel->isVisible();
}

bool TestWidget::isOffscreenOpenArrowShown()
{
    return ui->offscreenArrowLabel->isVisible();
}

void TestWidget::setSelectable(bool set)
{
	if(set)
	{
        m_selectable = true;
	    connect(this, SIGNAL(selected()), this, SLOT(onSelected()));
		connect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
	}
	else
	{
        m_selectable = false;
		disconnect(this, SIGNAL(selected()), this, SLOT(onSelected()));
		disconnect(this, SIGNAL(deselected()), this, SLOT(onDeselected()));
	}
}

