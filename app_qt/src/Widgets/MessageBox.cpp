/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "MessageBox.h"
#include "Dictionary.h"

#include <QFile>
#include <QPushButton>

namespace QtUI
{
    QString processLinks(const QString &message)
    {
        QString result;
        QRegExp regexp("https?://\\S*");
        int lastIndex = 0;
        int index = message.indexOf(regexp);
        while (index > 0) {
            result += message.mid(lastIndex, index - lastIndex);
            QString link = message.mid(index, regexp.matchedLength());
            result += QString("<A href=\"%1\">%1</A>").arg(link);
            lastIndex = index + link.length();
            index = message.indexOf(regexp, lastIndex);
        }
        result += message.mid(lastIndex);
        return result;
    }

    void setTopWindow(QWidget *widget, bool set)
    {
        Qt::WindowFlags flags = widget->windowFlags();
        if(set)
            widget->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
        else
            widget->setWindowFlags(flags & ~Qt::WindowStaysOnTopHint);
    }

    bool popupMsgBox(const QString &title, const QString &msg, const QString &acceptText, QWidget* parent, QMessageBox::Icon icon)
    {
        return popupMsgBox(title, msg, acceptText, QString::null, parent, icon);
    }

	bool popupMsgBox(const QString &title, const QString &msg, const QString &acceptText, const QString &rejectText, QWidget* /*parent*/, QMessageBox::Icon icon)
	{
		QMessageBox msgBox;
        msgBox.setTextFormat(Qt::RichText);
		msgBox.setWindowTitle(dict(title));
        msgBox.setModal(true);
		msgBox.setIcon(icon);
        msgBox.setText(dict(processLinks(msg)));
		QPushButton *accept = msgBox.addButton(dict(acceptText), QMessageBox::AcceptRole);
		accept->setObjectName("acceptButton");
		QPushButton *reject = 0;
		if(!rejectText.isNull())
		{
			reject = msgBox.addButton(dict(rejectText), QMessageBox::RejectRole);
			reject->setObjectName("rejectButton");
		}
        QFile cssFile(":/QMessageBox.css");
        cssFile.open(QFile::ReadOnly);
        QString css = cssFile.readAll();
    	msgBox.setStyleSheet(css);
		setTopWindow(&msgBox, true);
		msgBox.exec();
		setTopWindow(&msgBox, false);
		QPushButton *clicked = (QPushButton*) msgBox.clickedButton();
        return (clicked == accept);
	}
}

