/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef EULADIALOG_H
#define EULADIALOG_H

#include <QDialog>
#include "ui_EULADialog.h"

namespace QtUI
{
    class EULADialog;
    typedef EULADialog PrivacyDialog;
    
	class EULADialog : public QDialog
	{
		Q_OBJECT

	public:
        static EULADialog *createEULADialog(QWidget *parent = 0, bool alreadyAccepted = false, Qt::WindowFlags flags = 0);
        static PrivacyDialog *createPrivacyDialog(QWidget *parent = 0, bool alreadyAccepted = false, Qt::WindowFlags flags = 0);
        
		~EULADialog();
        
        bool isEULA();
        bool isPrivacy();
        
    protected:
        enum DialogType
        {
            EULA_DIALOG = 0,
            PRIVACY_DIALOG =1
        };
        
        EULADialog(QWidget *parent = 0, bool alreadyAccepted = false, Qt::WindowFlags flags = 0, DialogType type = EULA_DIALOG);
        
    public slots:
        void localize();
        
	private:
		Ui::EULADialog ui;

        bool m_alreadyAccepted;
        DialogType m_type;
	};
}

#endif