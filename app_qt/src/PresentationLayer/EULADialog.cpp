/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "EULADialog.h"
#include "Dictionary.h"
#include <QDebug>

using namespace QtUI;

EULADialog::EULADialog(QWidget *parent, bool alreadyAccepted, Qt::WindowFlags flags, DialogType type)
    : QDialog(parent), m_alreadyAccepted(alreadyAccepted), m_type(type)
{
	ui.setupUi(this);
	if(flags)
		setWindowFlags(flags);
	connect(ui.acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.rejectButton, SIGNAL(clicked()), this, SLOT(reject()));
    localize();
}

EULADialog::~EULADialog()
{
}

EULADialog *EULADialog::createEULADialog(QWidget *parent, bool alreadyAccepted, Qt::WindowFlags flags)
{
    EULADialog *dg;
    dg = new EULADialog(parent, alreadyAccepted, flags, EULA_DIALOG);
    return dg;
}

PrivacyDialog *EULADialog::createPrivacyDialog(QWidget *parent, bool alreadyAccepted, Qt::WindowFlags flags)
{
    PrivacyDialog *dg;
    dg = new EULADialog(parent, alreadyAccepted, flags, PRIVACY_DIALOG);
    return dg;
}

bool EULADialog::isEULA()
{
    return (m_type == EULA_DIALOG);
}

bool EULADialog::isPrivacy()
{
    return (m_type == PRIVACY_DIALOG);
}

void EULADialog::localize()
{
#ifdef OS_MAC
    ui.textBrowser->setStyleSheet("");
#endif
    QString title, text;
    if(isEULA())
    {
        title = dict("LicenseDialogTitle");
        text = dict("LicenseDialogBody");
    }
    else if(isPrivacy())
    {
        title = dict("PrivacyDialogTitle");
        text = dict("PrivacyDialogBody");
    }
    setWindowTitle(title);
    ui.textBrowser->setText(text);
    if(!m_alreadyAccepted)
    {
        if(isEULA())
        {
            ui.acceptButton->setText(dict("Accept"));
        }
        else if(isPrivacy())
        {
            ui.acceptButton->setText(dict("Yes"));
        }
        ui.rejectButton->show();
    }
    else
    {
        ui.acceptButton->setText(dict("OK"));
        ui.rejectButton->hide();
    }
    if(isEULA())
    {
        ui.rejectButton->setText(dict("Decline"));
    }
    else if(isPrivacy())
    {
        ui.rejectButton->setText(dict("No"));
    }
}