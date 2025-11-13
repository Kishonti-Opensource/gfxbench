/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>
#include <QString>
#include <QMessageBox>

namespace QtUI
{
    void setTopWindow(QWidget *widget, bool set);
    bool popupMsgBox(const QString &title, const QString &msg, const QString &acceptText, QWidget* parent = 0, QMessageBox::Icon icon = QMessageBox::NoIcon);
	bool popupMsgBox(const QString &title, const QString &msg, const QString &acceptText, const QString &rejectText, QWidget* parent = 0, QMessageBox::Icon icon = QMessageBox::NoIcon);
}

#endif // MESSAGEBOX_H
