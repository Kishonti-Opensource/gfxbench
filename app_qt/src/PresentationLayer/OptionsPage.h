/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include <QWidget>
#include "ui_OptionsPage.h"



class ApplicationConfig;
class BenchmarkService;



namespace QtUI
{
	class OptionsPage : public QWidget
	{
		Q_OBJECT
	public:
		OptionsPage(QWidget *parent = 0);
        void setBenchmarkService(BenchmarkService *benchmarkService);
        void onUserNameTaken();
        void onEmailTaken();
        void onInvalidEmail();
        void onInvalidCredentials();
        void onRegistrationSucceeded();
        void onLoggedIn(const std::string &username);
        void onLoggedOut();
        void onDeletedUser();
    public slots:
        void localize();
	private slots:
        void onDeleteUserClicked();
        void onButtonChecked(bool checked);
        void onFollowUsClicked();
        void onFollowUsCloseClicked();
        void onLoginClicked();
        void onLogoutClicked();
        void onRegisterClicked();
        void onRegisterCloseClicked();
        void onReadEULAClicked();
        void onReadPrivacyClicked();
        void onClearResultsClicked();
        void onExternalLinkClicked();
        void onBrightnessButtonClicked(bool checked);
		void forceBrightness(bool state);
        void onSubmitButtonClicked();
        void onRememberChecked(bool checked);
        void onCustomResChecked(bool enable);
        void onCustomResEdited();
        void onOutTrafficClicked();
	private:
        QString getSocialCSS(const QString &name);
        
		Ui::OptionsPage ui;
        BenchmarkService *m_benchmarkService;
        QString m_username;
        QString m_logButtonText;
        bool m_logged_user = false;
	};
}

#endif
