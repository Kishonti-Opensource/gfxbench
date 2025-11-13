/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "OptionsPage.h"
#include "Dictionary.h"
#include "EULADialog.h"
#include "MessageBox.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QFile>
#include <QIntValidator>

#include "benchmarkservice.h"



using namespace QtUI;



OptionsPage::OptionsPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    if (devicePixelRatio() > 1.0)
    {
        ui.twitterButton->setStyleSheet(getSocialCSS("twitter"));
        ui.facebookButton->setStyleSheet(getSocialCSS("facebook"));
        ui.linkedinButton->setStyleSheet(getSocialCSS("linkedin"));
        ui.youtubeButton->setStyleSheet(getSocialCSS("youtube"));
        ui.vimeoButton->setStyleSheet(getSocialCSS("vimeo"));
        ui.gplusButton->setStyleSheet(getSocialCSS("gplus"));
    }

	ui.customResWidthEdit->setValidator( new QIntValidator(0, INT_MAX, this) );
	ui.customResHeightEdit->setValidator( new QIntValidator(0, INT_MAX, this) );

	QPushButton* brightnessBtn[] = {ui.brightness0Button, ui.brightness25Button, ui.brightness50Button, ui.brightness75Button, ui.brightness100Button};
	const int brightnessInc = 25;
	int brightness = 0;

	for (size_t i=0; i<sizeof(brightnessBtn)/sizeof(brightnessBtn[0]); i++, brightness+=brightnessInc) {
		connect(brightnessBtn[i], SIGNAL(clicked(bool)), this, SLOT(onBrightnessButtonClicked(bool)));
		brightnessBtn[i]->setProperty("brightness", brightness);
        brightnessBtn[i]->setEnabled(false);
	}
    ui.followUsPanel->hide();
    ui.registerPanel->hide();

	connect(ui.forceBrightnessCheckButton, SIGNAL(toggled(bool)), this, SLOT(forceBrightness(bool)));
    connect(ui.followUsButton, SIGNAL(clicked()), this, SLOT(onFollowUsClicked()));
    connect(ui.followUsCloseButton, SIGNAL(clicked()), this, SLOT(onFollowUsCloseClicked()));
    connect(ui.registerButton, SIGNAL(clicked()), this, SLOT(onRegisterClicked()));
    connect(ui.registerCloseButton, SIGNAL(clicked()), this, SLOT(onRegisterCloseClicked()));
    connect(ui.readLicenseButton, SIGNAL(clicked()), this, SLOT(onReadEULAClicked()));
    connect(ui.readPrivacyButton, SIGNAL(clicked()), this, SLOT(onReadPrivacyClicked()));
    connect(ui.clearResultHistoryButton, SIGNAL(clicked()), this, SLOT(onClearResultsClicked()));
    connect(ui.submitButton, SIGNAL(clicked()), this, SLOT(onSubmitButtonClicked()));
    connect(ui.registerUserLineEdit, SIGNAL(returnPressed()), this, SLOT(onSubmitButtonClicked()));
    connect(ui.registerPassLineEdit1, SIGNAL(returnPressed()), this, SLOT(onSubmitButtonClicked()));
    connect(ui.registerPassLineEdit2, SIGNAL(returnPressed()), this, SLOT(onSubmitButtonClicked()));
    connect(ui.registerEmailLineEdit, SIGNAL(returnPressed()), this, SLOT(onSubmitButtonClicked()));
    connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(onLoginClicked()));
    connect(ui.userLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoginClicked()));
    connect(ui.passLineEdit, SIGNAL(returnPressed()), this, SLOT(onLoginClicked()));
    connect(ui.deleteUserButton, SIGNAL(clicked()), this, SLOT(onDeleteUserClicked()));
    connect(ui.customResCheckButton, SIGNAL(toggled(bool)), this, SLOT(onCustomResChecked(bool)));
    connect(ui.outTrafficCheckButton, SIGNAL(clicked()), this, SLOT(onOutTrafficClicked()));

    QLineEdit* resEdits[] = { ui.customResWidthEdit, ui.customResHeightEdit };
    for(size_t i = 0; i < sizeof(resEdits) / sizeof(resEdits[0]); i++)
    {
        connect(resEdits[i], SIGNAL(editingFinished()), this, SLOT(onCustomResEdited()));
        connect(resEdits[i], SIGNAL(textChanged(const QString&)), this, SLOT(onCustomResEdited()));
    }

    QPushButton* checkButtons[] = { ui.customResCheckButton,  ui.forceBrightnessCheckButton};
    for(size_t i = 0; i < sizeof(checkButtons) / sizeof(checkButtons[0]); i++)
    {
        connect(checkButtons[i], SIGNAL(toggled(bool)), this, SLOT(onButtonChecked(bool)));
    }

    QPushButton* linkButtons[] = { ui.twitterButton, ui.linkedinButton, ui.youtubeButton, ui.vimeoButton, ui.gplusButton, ui.facebookButton, ui.productLinkButton, ui.orgLinkButton, ui.contactusButton };
    for(size_t i=0; i<sizeof(linkButtons)/sizeof(linkButtons[0]); i++)
    {
        connect(linkButtons[i], SIGNAL(clicked()), this, SLOT(onExternalLinkClicked()));
    }

    ui.loggedInLabel->hide();
    m_logButtonText = "Login";

    if(QApplication::applicationName().toLower().contains("compubench"))
    {
        ui.followUsButton->click();
        ui.followUsButton->hide();
        ui.followUsCloseButton->setDisabled(true);
    }

    localize();
}



void OptionsPage::localize()
{
    ui.titleLabel->setText(dict("TabSettings"));
    ui.userLabel->setText(dict("User"));
    ui.userLineEdit->setPlaceholderText(dict("Username"));
    ui.passLineEdit->setPlaceholderText(dict("Password"));
    ui.loginButton->setText(dict(m_logButtonText));
    ui.registerButton->setText(dict("Register"));

    ui.registerCloseButton->setText(dict("Close"));
    ui.registerTitleLabel->setText(dict("RegistrationForm"));
    ui.registerLabel->setText(dict("FillTheForm"));
    ui.registerUserLineEdit->setPlaceholderText(dict("Username"));
    ui.registerPassLineEdit1->setPlaceholderText(dict("Password"));
    ui.registerPassLineEdit2->setPlaceholderText(dict("Password"));
    ui.registerEmailLineEdit->setPlaceholderText(dict("Email"));
    ui.submitButton->setText(dict("Register"));
    ui.corporateFeaturesLabel->setText(dict("CorporateFeatures"));

    ui.informationTitleLabel->setText(dict("NotRegisteredSectionTitle"));
    ui.informationLabel->setText(dict("NotRegisteredSectionBody"));
    ui.followUsButton->setText(dict("FollowUs"));
    ui.readLicenseButton->setText(dict("ReadLicense"));
    ui.readPrivacyButton->setText(dict("ReadPrivacy"));
    ui.clearResultHistoryButton->setText(dict("ClearHistoryDialogTitle"));

    ui.followUsTitleLabel->setText(dict("FollowUs"));
    ui.followUsCloseButton->setText(dict("Close"));
    ui.whatIsThisProductLabel->setText(dict("FollowUsSectionBody"));
    ui.orgLinkButton->setText("kishonti.net");
    ui.contactusButton->setText(dict("ContactUs"));

    ui.aboutLicensingTitleLabel->setText(dict("CorporateCommercialTitle"));
    ui.aboutLicensingLabel->setText(dict("CorporateCommercial") +
                                    "\n\nThe Qt Toolkit is Copyright (C) The Qt Company Ltd.\n" +
                                    "Qt is licensed under terms of the GNU LGPLv3, available at:\n\n" +
                                    "https://github.com/kishontikft/qtbase/blob/5.10/LICENSE.LGPLv3");

    ui.loggedInLabel->setText(dict("Welcome").append(" ").append(m_username));

    ui.customResLabel->setText(dict("SetOnscrResolution"));
    ui.customResHeightLabel->setText(dict("Height"));
    ui.customResWidthLabel->setText(dict("Width"));

    QPushButton* checkButtons[] = { ui.customResCheckButton,  ui.forceBrightnessCheckButton, ui.outTrafficCheckButton };
    for(size_t i = 0; i < sizeof(checkButtons) / sizeof(checkButtons[0]); i++)
    {
        checkButtons[i]->isChecked() ? checkButtons[i]->setText(dict("SwitchStateOn")) : checkButtons[i]->setText(dict("SwitchStateOff"));
    }

    ui.outTrafficLabel->hide();
    ui.outTrafficCheckButton->hide();
}



void OptionsPage::setBenchmarkService(BenchmarkService *benchmarkService)
{
    m_benchmarkService = benchmarkService;
    if (benchmarkService == 0) return;

    QString websiteUrl = QCoreApplication::organizationDomain();
    ui.productLinkButton->setText(websiteUrl);
    ui.productLinkButton->setProperty("link", "https://" + websiteUrl);
    QString contactEmail = "mailto:help@" + websiteUrl;
    ui.contactusButton->setProperty("link", contactEmail);

    bool isCorporateVersion =
            benchmarkService->getConfig(BenchmarkService::CORPORATE_VERSION) == "true";
    QWidget *communityWidgets[] = {
        ui.userLineEdit,
        ui.passLineEdit,
        ui.registerButton,
        ui.loginButton,
        ui.readLicenseButton,
        ui.deleteUserButton
    };
    for (size_t i = 0; i < sizeof(communityWidgets) / sizeof(communityWidgets[0]); i++) {
        communityWidgets[i]->setDisabled(isCorporateVersion);
    }

    ui.deleteUserButton->setEnabled(m_logged_user);
    ui.readPrivacyButton->setVisible(!isCorporateVersion);
    ui.aboutPanel->setVisible(!isCorporateVersion);
    ui.corporateFeaturesPanel->setVisible(isCorporateVersion);
}



void OptionsPage::onUserNameTaken()
{
    ui.registerUserLineEdit->setFocus();
    ui.registerUserLineEdit->selectAll();
}



void OptionsPage::onEmailTaken()
{
    ui.registerEmailLineEdit->setFocus();
    ui.registerEmailLineEdit->selectAll();
}



void OptionsPage::onInvalidEmail()
{
    ui.registerEmailLineEdit->setFocus();
    ui.registerEmailLineEdit->selectAll();
}



void OptionsPage::onInvalidCredentials()
{
    ui.passLineEdit->setFocus();
    ui.passLineEdit->selectAll();
}



void OptionsPage::onRegistrationSucceeded()
{
    popupMsgBox("RegistrationFeedbackDialogTitle", "RegistrationFeedbackDialogBody", "OK", this);
    onRegisterCloseClicked();
}



void OptionsPage::onLoggedIn(const std::string &username)
{
    m_logged_user = true;
    ui.loggedInLabel->show();
    ui.userLineEdit->hide();
    ui.userLineEdit->setText(QString::fromStdString(username));
    ui.passLineEdit->hide();
    ui.registerButton->hide();
    ui.deleteUserButton->setEnabled(true);
    m_username = QString::fromStdString(username);
    m_logButtonText = "Logout";
    localize();
    disconnect(ui.loginButton, SIGNAL(clicked()), this, SLOT(onLoginClicked()));
    connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(onLogoutClicked()));
}



void OptionsPage::onLoggedOut()
{
    m_logged_user = false;

    ui.loggedInLabel->hide();
    ui.userLineEdit->show();
    ui.passLineEdit->show();
    ui.registerButton->show();
    ui.deleteUserButton->setEnabled(false);
    m_logButtonText = "Login";
    localize();
    disconnect(ui.loginButton, SIGNAL(clicked()), this, SLOT(onLogoutClicked()));
    connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(onLoginClicked()));
}


void OptionsPage::onDeletedUser()
{
    onLoggedOut();
}

void OptionsPage::onDeleteUserClicked()
{
    if(popupMsgBox("DeleteUserDialogTitle", "DeleteUserDialogBody", "Yes", "No", this))
    {
        m_benchmarkService->deleteUser();
    }
}



void OptionsPage::onButtonChecked(bool checked)
{
    QPushButton *button = (QPushButton*)sender();
    button->setText(dict(checked ? "SwitchStateOn" : "SwitchStateOff"));
}



void OptionsPage::onReadEULAClicked()
{
    EULADialog *eula = EULADialog::createEULADialog(this, true);
    eula->exec();
    delete eula;
}



void OptionsPage::onReadPrivacyClicked()
{
    PrivacyDialog *privacy = PrivacyDialog::createPrivacyDialog(this, true);
    privacy->exec();
}



void OptionsPage::onClearResultsClicked()
{
    if(popupMsgBox("ClearHistoryDialogTitle", "ClearHistoryDialogBody", "OK", "Cancel", this))
    {
        m_benchmarkService->clearResults();
    }
}



void OptionsPage::onFollowUsClicked()
{
    if(ui.registerPanel->isVisible())
    {
        ui.registerPanel->hide();
    }
    ui.followUsPanel->show();
    ui.rightPanel->hide();
}



void OptionsPage::onFollowUsCloseClicked()
{
    ui.followUsPanel->hide();
    ui.rightPanel->show();
}



void OptionsPage::onLoginClicked()
{
    QLineEdit* requiredFields[] = { ui.userLineEdit, ui.passLineEdit };
    for(size_t i = 0; i < sizeof(requiredFields)/sizeof(requiredFields[0]); i++)
    {
        if(requiredFields[i]->text().isEmpty())
        {
            popupMsgBox("MissingParametersDialogTitle", "MissingParametersDialogBody", "OK", QString::null, this);
            requiredFields[i]->setFocus();
            return;
        }
    }
    m_username = ui.userLineEdit->text();
    m_benchmarkService->login(ui.userLineEdit->text().toUtf8(), ui.passLineEdit->text().toUtf8());
}



void OptionsPage::onLogoutClicked()
{
    m_benchmarkService->logout();
}



void OptionsPage::onRegisterClicked()
{
    ui.followUsPanel->hide();
    ui.rightPanel->hide();
    ui.registerPanel->show();
}



void OptionsPage::onRegisterCloseClicked()
{
    ui.registerPanel->hide();
    ui.rightPanel->show();
}



void OptionsPage::onExternalLinkClicked()
{
    QPushButton *button = (QPushButton*) sender();
    QString link = button->property("link").toString();
    QUrl url(link);
    QDesktopServices::openUrl(url);
}



void OptionsPage::onBrightnessButtonClicked(bool checked)
{
    if (!checked) {
        m_benchmarkService->setCustomBrightness(0.5);
        return;
    }

    QPushButton* brightnessBtn[] = {
        ui.brightness0Button,
        ui.brightness25Button,
        ui.brightness50Button,
        ui.brightness75Button,
        ui.brightness100Button
    };

    for (size_t i = 0; i < sizeof(brightnessBtn) / sizeof(brightnessBtn[0]); ++i) {
        if (brightnessBtn[i] == sender()) {
            m_benchmarkService->setCustomBrightness(i / 4.0);
        } else {
            brightnessBtn[i]->setChecked(false);
        }
    }
}



void OptionsPage::forceBrightness(bool state)
{
    if (!state && m_benchmarkService) {
        m_benchmarkService->setCustomBrightness(0.5);
    }

    QPushButton* brightnessBtn[] = {
        ui.brightness0Button,
        ui.brightness25Button,
        ui.brightness50Button,
        ui.brightness75Button,
        ui.brightness100Button
    };
    for (size_t i = 0; i < sizeof(brightnessBtn) / sizeof(brightnessBtn[0]); ++i) {
        brightnessBtn[i]->setEnabled(state);
        if (!state) {
            brightnessBtn[i]->setChecked(false);
        }
    }
}



void OptionsPage::onSubmitButtonClicked()
{
    QLineEdit* requiredFields[] = { ui.registerUserLineEdit, ui.registerPassLineEdit1, ui.registerPassLineEdit2, ui.registerEmailLineEdit };
    for(size_t i = 0; i < sizeof(requiredFields)/sizeof(requiredFields[0]); i++)
    {
        if(requiredFields[i]->text().isEmpty())
        {
            popupMsgBox("MissingParametersDialogTitle", "MissingParametersDialogBody", "OK", this);
            requiredFields[i]->setFocus();
            return;
        }
    }
    if(QString::compare(ui.registerPassLineEdit1->text(), ui.registerPassLineEdit2->text()))
    {
        popupMsgBox("PasswordMismatchDialogTitle", "PasswordMismatchDialogBody", "OK", this);
        ui.registerPassLineEdit1->setText("");
        ui.registerPassLineEdit2->setText("");
        ui.registerPassLineEdit1->setFocus();
        return;
    }
    m_benchmarkService->signUp(
            ui.registerEmailLineEdit->text().toUtf8(),
            ui.registerUserLineEdit->text().toUtf8(),
            ui.registerPassLineEdit1->text().toUtf8());

}



void OptionsPage::onRememberChecked(bool checked)
{
    m_benchmarkService->setLogoutOnClose(!checked);
}



void OptionsPage::onCustomResChecked(bool enable)
{
    ui.customResWidget->setEnabled(enable);
    ui.customResHeightEdit->setEnabled(enable);
    ui.customResWidthEdit->setEnabled(enable);
    if(enable) {
        onCustomResEdited();
    } else {
        m_benchmarkService->setCustomResolution(0, 0);
    }
}



void OptionsPage::onCustomResEdited()
{
    m_benchmarkService->setCustomResolution(
            ui.customResWidthEdit->text().toInt(), ui.customResHeightEdit->text().toInt());
}



void OptionsPage::onOutTrafficClicked()
{
    onReadPrivacyClicked();
}



QString OptionsPage::getSocialCSS(const QString &name)
{
    QString social =
    "QPushButton#<name>Button \
    { \
        border-image: url(:/retina_social_<name>.png) 0 0 0 0 stretch stretch; \
    }";
    social.replace("<name>", name);
    return social;
}
