/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "InfoWidget.h"

#include "Dictionary.h"

#include <QMouseEvent>
#include <QDebug>
#include <QIcon>


InfoWidget::InfoWidget(const QString &title, const QIcon& icon, const QString &majorText, const QString &minorText, QWidget *parent)
	: QWidget(parent), m_title(title), m_majorText(majorText), m_minorText(minorText)
{
	ui.setupUi(this);

    if (devicePixelRatio() > 1.0)
    {
        QString arrow = "QLabel { border-image: url(:/retina_arrow.png) 0 0 0 0 stretch stretch; }";
        ui.openArrowLabel->setStyleSheet(arrow);
    }
    ui.titleIcon->setPixmap(icon.pixmap(ui.titleIcon->size()));
	minorText.isEmpty() ? ui.minorLabel->hide() : ui.minorLabel->show();
    majorText.isEmpty() ? ui.majorLabel->hide() : ui.majorLabel->show();
    if(majorText.isEmpty() && minorText.isEmpty())
    {
        ui.gadgetWidget->show();
    }
    else
    {
        ui.gadgetWidget->hide();
    }
    ui.openArrowLabel->hide();
    m_pressed = 0;

    localize();

    QObject::connect(&m_model, &QAbstractListModel::dataChanged, this, &InfoWidget::updateGadgets);
    QObject::connect(&m_model, &QAbstractListModel::modelReset, this, &InfoWidget::updateGadgets);
}

void InfoWidget::localize()
{
    ui.titleLabel->setText(QtUI::dict(m_title));
    setMajorText(m_majorText);
	setMinorText(m_minorText);
    for(int i(0); i < m_gadgetText.count(); i++)
    {
        m_gadgetLabels[i]->setText(QtUI::dict(m_gadgetText[i]));
    }
}

void InfoWidget::setMajorText(const QString &major)
{
    m_majorText = major;
    QString translated = QtUI::dict(m_majorText);
    ui.majorLabel->setText(translated);
}

QString InfoWidget::majorText()
{
    return m_majorText;
}

void InfoWidget::setMinorText(const QString &minor)
{
    m_minorText = minor;
    QString translated = QtUI::dict(m_minorText);
    ui.minorLabel->setText(translated);
}

QString InfoWidget::minorText()
{
    return m_minorText;
}

void InfoWidget::setGadgets(const std::shared_ptr<Cursor>& cursor)
{
    m_model.setCursor(cursor);
}

void InfoWidget::showOpenArrow(bool show)
{
    ui.openArrowLabel->setVisible(show);
}

bool InfoWidget::isOpenArrowShown() const
{
    return ui.openArrowLabel->isVisible();
}

void InfoWidget::setPrefix(const std::string &prefix)
{
    m_prefix = prefix;
}

std::string InfoWidget::prefix() const
{
    return m_prefix;
}

void InfoWidget::click()
{
    if(isOpenArrowShown())
    {
        emit clicked();
    }
}

void InfoWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui.verticalLine->width();
        QRect infoRect = QRect(QPoint(ui.iconPanel->width() + line, 0), ui.infoPanel->size());

        if(infoRect.contains(event->pos()))
        {
            m_pressed = ui.infoPanel;
        }
    }
}

void InfoWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
    {
        int line = ui.verticalLine->width();
        QRect infoRect = QRect(QPoint(ui.iconPanel->width() + line, 0), ui.infoPanel->size());

        if(infoRect.contains(event->pos()) && m_pressed == ui.infoPanel)
        {
            this->click();
        }
        m_pressed = 0;
    }
}

void InfoWidget::updateGadgets()
{
    int maxColumn = 4;
    if (m_model.rowCount() > 0)
    {
        ui.gadgetWidget->show();
    }
    int currColCount = 0;
    int widgetCount = 0;
    QHBoxLayout* layout = 0;
    QVBoxLayout *vLayout = (QVBoxLayout*)ui.gadgetWidget->layout();

    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        QModelIndex modelIndex = m_model.index(i);
        if (!layout)
        {
            layout = new QHBoxLayout();
            vLayout->insertLayout(0, layout);
            layout->setSpacing(5);
        }
        QLabel *keyLabel = new QLabel(QtUI::dict(modelIndex.data(Qt::DisplayRole).toString()) + ":", ui.gadgetWidget);
        m_gadgetLabels.append(keyLabel);
        m_gadgetText.append(modelIndex.data(Qt::DisplayRole).toString());
        keyLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        keyLabel->setStyleSheet("QLabel { color : #444; }");
        layout->addWidget(keyLabel);
        QVariant val = modelIndex.data(Kishonti::ValueRole);
        QLabel* valueLabel = new QLabel(ui.gadgetWidget);
        valueLabel->setMinimumSize(14, 14);
        valueLabel->setMaximumSize(14, 14);
        valueLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        valueLabel->setAlignment(Qt::AlignCenter);

        QString ss;
        if (val.toBool())
        {
            ss = QString("QLabel { border-image: url(:/%1) 0 0 0 0 stretch stretch; }").arg("support.png");
        }
        else
        {
            ss = QString("QLabel { border-image: url(:/%1) 0 0 0 0 stretch stretch; }").arg("notsupport.png");
        }
        valueLabel->setStyleSheet(ss);

        layout->addWidget(valueLabel);
        widgetCount++;
        currColCount++;

        if (widgetCount == m_model.rowCount() || currColCount == maxColumn)
        {
            layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            if (currColCount == maxColumn)
            {
                currColCount = 0;
                layout = 0;
            }
        }
    }
}
