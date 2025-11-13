/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPSettingsViewController.m
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import "APPSettingsViewController.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUIUtilities.h"
#import "APPRegisterPage.h"
#import "APPFindOutMorePage.h"
#import "APPEulaPage.h"
#import "APPTestInfoPage.h"
#import "APPLanguageSelectorPage.h"

NSString * const APP_UserPreference_CustomRes = @"CRes";
NSString * const APP_UserPreference_CustomResWidth = @"CResW";
NSString * const APP_UserPreference_CustomResHeight = @"CResH";
NSString * const APP_UserPreference_Brightness = @"CBright";
NSString * const APP_UserPreference_BrightnessValue = @"CBrightVal";
NSString * const APP_UserPreference_ShowDesktop = @"ShowDesktop";
NSString * const APP_UserPreference_Endless = @"Endless";

typedef enum appSettingsState
{
    APPSETTINGS_BASE
} AppSettingsState;

@interface APPSettingsViewController()

@property (strong, nonatomic) APPPageNavigator *navigator;
@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@property (copy, nonatomic) NSString *navigatorTitle;
@property (copy, nonatomic) NSString *loginTitle;
@property (copy, nonatomic) NSString *userName;
@property (assign, nonatomic) AppSettingsState state;

@end

@implementation APPSettingsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self.state = APPSETTINGS_BASE;
    self.navigatorTitle = @"TabSettings";
    self.loginTitle = @"Login";
    
    [self.greetingsLabel setAlpha:0.0];
    
    [self setupEdition];
    
    [self.brightnessSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:APP_UserPreference_Brightness]];
    [self.customResSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:APP_UserPreference_CustomRes]];
    [self.showDesktopSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:APP_UserPreference_ShowDesktop]];
    [self.endlessSwitch setOn:[[NSUserDefaults standardUserDefaults] boolForKey:APP_UserPreference_Endless]];
    
#if IS_CORPORATE
    [self.widthInput setText:[NSString stringWithFormat:@"%ld", (long)[[NSUserDefaults standardUserDefaults] integerForKey:APP_UserPreference_CustomResWidth]]];
    [self.heightInput setText:[NSString stringWithFormat:@"%ld", (long)[[NSUserDefaults standardUserDefaults] integerForKey:APP_UserPreference_CustomResHeight]]];
    
    if([self.widthInput.text isEqualToString:@"0"])
        self.widthInput.text = @"1024";
    if([self.heightInput.text isEqualToString:@"0"])
        self.heightInput.text = @"768";
    
    [self.brightnessSelector setSelectedSegmentIndex:[[NSUserDefaults standardUserDefaults] integerForKey:APP_UserPreference_BrightnessValue]];
#endif
    
    [self brightnessValueChanged:self];
    [self showDesktopValueChanged:self];
    [self customResValueChanged:self];
    [self endlessValueChanged:self];
    
    [self setupNavigator];
    [self setupBrightness];
    [self setupResolution];
    [self setupShowDesktop];
    
    [self applyTheme];
    [self applyLocalization];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLogIn:) name:NUI_Notification_LoggedIn object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLogOut) name:NUI_Notification_LoggedOut object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleDeletedUser) name:NUI_Notification_DeletedUser object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleSignUp) name:NUI_Notification_SignedUp object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
    
    
    if([NUIAppData sharedNUIAppData].loginName != nil) {
        [self handleLogIn:[NSNotification notificationWithName:NUI_Notification_LoggedIn object:self userInfo:@{@"username":[NUIAppData sharedNUIAppData].loginName}]];
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}


#pragma mark - Page Setup

- (void) setupNavigator {
    self.navigator = [APPPageNavigator addNavigatorIn:self.navigatorContainer];
    self.navigator.delegate = self;
}

- (void) setupEdition {
#if IS_CORPORATE
    [self.usernameInput setEnabled:NO];
    [self.passwirdInput setEnabled:NO];
    [self.loginBtn setEnabled:NO];
    [self.registerBtn setEnabled:NO];
    [self.eulaBtn setEnabled:NO];
    
    [self.customResSwitch setEnabled:YES];
    [self.brightnessSwitch setEnabled:YES];
    [self.endlessSwitch setEnabled:YES];
    [self.brightnessSelector setEnabled:YES];
    [self.widthInput setEnabled:YES];
    [self.heightInput setEnabled:YES];

    [self.informationContainer removeFromSuperview];
    self.informationContainer = nil;
    self.informationDesc = nil;
    self.informationTitle = nil;
    
    [self.commercialContainer removeFromSuperview];
    self.commercialContainer = nil;
    self.commercialDesc = nil;
    self.commercialTitle = nil;
    
#else
    [self.usernameInput setEnabled:YES];
    [self.passwirdInput setEnabled:YES];
    [self.loginBtn setEnabled:YES];
    [self.registerBtn setEnabled:YES];
    [self.eulaBtn setEnabled:YES];
    
    [self.customResSwitch setEnabled:NO];
    [self.brightnessSwitch setEnabled:NO];
    [self.endlessSwitch setEnabled:NO];
    [self.brightnessSelector setEnabled:NO];
    [self.widthInput setEnabled:NO];
    [self.heightInput setEnabled:NO];
    
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setBool:NO forKey:APP_UserPreference_CustomRes];
    [userDefaults setBool:NO forKey:APP_UserPreference_Brightness];
    [userDefaults setBool:NO forKey:APP_UserPreference_Endless];
    [userDefaults synchronize];
#endif
}

- (void) setupResolution {
    BOOL isSet = [self.customResSwitch isOn];
    
    [UIView animateWithDuration:0.2 animations:^{
        [self.widthInput setEnabled:isSet];
        [self.heightInput setEnabled:isSet];
        
        [self.widthLabel setAlpha:isSet ? 1.0 : 0.6];
        [self.heightLabel setAlpha:isSet ? 1.0 : 0.6];
        
        [self.widthInput setTextColor:isSet ?
         [THTheme getColorNamed:@"TextSettingsBaseColor"] : [THTheme getColorNamed:@"TextSettingsDisbaledColor"]];
        [self.heightInput setTextColor:isSet ?
         [THTheme getColorNamed:@"TextSettingsBaseColor"] : [THTheme getColorNamed:@"TextSettingsDisbaledColor"]];
    }];
    
}

- (void) setupBrightness {
    [UIView animateWithDuration:0.2 animations:^{
        [self.brightnessSelector setEnabled:[self.brightnessSwitch isOn]];
        
    }];
    
}

- (void) setupShowDesktop {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    BOOL showDesktop = [userDefaults boolForKey:APP_UserPreference_ShowDesktop];
    [self.showDesktopSwitch setOn:showDesktop];
}

- (void) setupAccount {

}


#pragma mark - Base Page Overrides

- (void) applyLocalization {
    if(self.navigator != nil) {
        self.navigator.title.text = [NUIAppData getLocalized:self.navigatorTitle];
        
        if(self.state == APPSETTINGS_BASE) {
            [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.rightBtn setUserInteractionEnabled:NO];
            [self.navigator.leftBtn setUserInteractionEnabled:NO];
            
        }
        
        self.navigator.subtitle.text = [NUIUtilities getVersionString];
    }
    
    [self applyLocalizationAccount];
    [self applyLocalizationCommercial];
    [self applyLocalizationCorporate];
    [self applyLocalizationInfo];
    [self applyLocalizationSettings];
}

- (void) applyLocalizationAccount {
    [self.accountTitle setText:[NUIAppData getLocalized:@"Login"]];
    [self.loginBtn setTitle:[NUIAppData getLocalized:self.loginTitle] forState:UIControlStateNormal];
    [self.registerBtn setTitle:[NUIAppData getLocalized:@"Register"] forState:UIControlStateNormal];
    
    [self.usernameInput setPlaceholder:[NUIAppData getLocalized:@"Username"]];
    [self.passwirdInput setPlaceholder:[NUIAppData getLocalized:@"Password"]];
    
    if(self.userName != nil && ![self.userName isEqualToString:@""])
        [self.greetingsLabel setText:[NSString stringWithFormat:@"%@ %@", [NUIAppData getLocalized:@"Welcome"], self.userName]];
}

- (void) applyLocalizationInfo {
    if(self.informationContainer != nil) {
        [self.informationTitle setText:[NUIAppData getLocalized:@"NetRequirementDialogTitle"]];
        [self.informationDesc setText:[NUIAppData getLocalized:@"NotRegisteredSectionBody"]];
    }
}

- (void) applyLocalizationSettings {
    [self.testInfoBtn setTitle:[NUIAppData getLocalized:@"TestList"] forState:UIControlStateNormal];
    [self.findOutMoreBtn setTitle:[NUIAppData getLocalized:@"FindOutMore"] forState:UIControlStateNormal];
    [self.eulaBtn setTitle:[NUIAppData getLocalized:@"ReadLicense"] forState:UIControlStateNormal];
    [self.clearHistoryBtn setTitle:[NUIAppData getLocalized:@"ClearHistoryDialogTitle"] forState:UIControlStateNormal];
    [self.languageSelectorBtn setTitle:[NUIAppData getLocalized:@"SelectLanguage"] forState:UIControlStateNormal];
    
    [self.showDesktopLabel setText:[NUIAppData getLocalized:@"ShowDesktop"]];
}

- (void) applyLocalizationCorporate {
    [self.corporateTitle setText:[NUIAppData getLocalized:@"CorporateFeatures"]];
    [self.customResLabel setText:[NUIAppData getLocalized:@"SetOnscrResolution"]];
    [self.endlessLabel setText:[NUIAppData getLocalized:@"EndlessTestRun"]];
    [self.widthLabel setText:[NUIAppData getLocalized:@"Width"]];
    [self.heightLabel setText:[NUIAppData getLocalized:@"Height"]];
    [self.brightnessLabel setText:[NUIAppData getLocalized:@"ForceDisplayBrightness"]];
}

- (void) applyLocalizationCommercial {
    if(self.commercialContainer != nil) {
        [self.commercialTitle setText:[NUIAppData getLocalized:@"CorporateCommercialTitle"]];
        [self.commercialDesc setText:[NUIAppData getLocalized:@"CorporateCommercial"]];
    }
}

- (void) applyTheme {
    self.view.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    
    [self.baseScroller setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
    for (UIView *separator in self.separators) {
        [separator setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    }
    
    if(self.navigator != nil) {
        [self.navigator setBackgroundColor:[THTheme getColorNamed:@"NavBarColor"]];
        [self.navigator.title setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self.navigator.subtitle setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self.navigator.leftBtn setTitleColor:[THTheme getColorNamed:@"TextHeaderColor"] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitleColor:[THTheme getColorNamed:@"TextHeaderColor"] forState:UIControlStateNormal];
        
        [self.navigator.title setFont:[THTheme getFontNamed:@"TitleSFont"]];
        [self.navigator.subtitle setFont:[THTheme getFontNamed:@"TextSFont"]];
        [self.navigator.leftBtn.titleLabel setFont:[THTheme getFontNamed:@"TextSFont"]];
        [self.navigator.rightBtn.titleLabel setFont:[THTheme getFontNamed:@"TextSFont"]];
    }
    
    [self applyThemeAccount];
    [self applyThemeCommercial];
    [self applyThemeCorporate];
    [self applyThemeInfo];
    [self applyThemeSettings];
}

- (void) applyThemeAccount {
    [self.accountTitle setTextColor:[THTheme getColorNamed:@"TextSettingsTitleColor"]];
    [self.accountTitle setFont:[THTheme getFontNamed:@"TextLFont"]];
    
    [self.loginBtn setTitleColor:[THTheme getColorNamed:@"TextBtnNormalColor"] forState:UIControlStateNormal];
    [self.loginBtn setTitleColor:[THTheme getColorNamed:@"TextBtnDisabledColor"] forState:UIControlStateDisabled];
    [self.loginBtn setBackgroundColor:[THTheme getColorNamed:[self.loginBtn isEnabled] ? @"BtnMainColor" : @"TextBoxDisabledBackColor"]];
    
    [self.registerBtn setBackgroundColor:[THTheme getColorNamed:[self.registerBtn isEnabled] ? @"BtnMainColor" : @"TextBoxDisabledBackColor"]];
    [self.registerBtn setTitleColor:[THTheme getColorNamed:@"TextBtnNormalColor"] forState:UIControlStateNormal];
    [self.registerBtn setTitleColor:[THTheme getColorNamed:@"TextBtnDisabledColor"] forState:UIControlStateDisabled];
    
    [self.registerBtn.layer setCornerRadius:4.0];
    [self.loginBtn.layer setCornerRadius:4.0];
}

- (void) applyThemeInfo {
    if(self.informationContainer != nil) {
        [self.informationTitle setTextColor:[THTheme getColorNamed:@"TextSettingsTitleColor"]];
        [self.informationTitle setFont:[THTheme getFontNamed:@"TextLFont"]];
    
        [self.informationDesc setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
        [self.informationDesc setFont:[THTheme getFontNamed:@"TextMFont"]];
    }
}

- (void) applyThemeSettings {
    [self.testInfoBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.testInfoBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.findOutMoreBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.findOutMoreBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.eulaBtn setBackgroundColor:[THTheme getColorNamed:[self.eulaBtn isEnabled] ? @"MainColor" : @"TextBoxDisabledBackColor"]];
    [self.eulaBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.eulaBtn setTitleColor:[THTheme getColorNamed:@"TextBtnDisabledColor"] forState:UIControlStateDisabled];
    [self.clearHistoryBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.clearHistoryBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.languageSelectorBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.languageSelectorBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    
    [self.showDesktopLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.showDesktopLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.showDesktopSwitch setOnTintColor:[THTheme getColorNamed:@"MainColor"]];
    
    
    [self.testInfoBtn.layer setCornerRadius:4.0];
    [self.findOutMoreBtn.layer setCornerRadius:4.0];
    [self.eulaBtn.layer setCornerRadius:4.0];
    [self.clearHistoryBtn.layer setCornerRadius:4.0];
    [self.languageSelectorBtn.layer setCornerRadius:4.0];
    
}

- (void) applyThemeCorporate {
    [self.corporateTitle setTextColor:[THTheme getColorNamed:[self.customResSwitch isEnabled] ? @"TextSettingsTitleColor" : @"TextSettingsDisbaledColor"]];
    [self.corporateTitle setFont:[THTheme getFontNamed:@"TextLFont"]];
    
    [self.customResLabel setTextColor:[THTheme getColorNamed:[self.customResSwitch isEnabled] ? @"TextSettingsBaseColor" : @"TextSettingsSecondaryDisbaledColor"]];
    [self.customResLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.widthLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.widthLabel setAlpha:[self.customResSwitch isOn] ? 1.0 : 0.6];
    [self.widthLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.heightLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.heightLabel setAlpha:[self.customResSwitch isOn] ? 1.0 : 0.6];
    [self.heightLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.brightnessLabel setTextColor:[THTheme getColorNamed:[self.customResSwitch isEnabled] ? @"TextSettingsBaseColor" : @"TextSettingsSecondaryDisbaledColor"]];
    [self.brightnessLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.widthInput setTextColor:[self.customResSwitch isOn] ?
     [THTheme getColorNamed:@"TextSettingsBaseColor"] : [THTheme getColorNamed:@"TextSettingsDisbaledColor"]];
    [self.heightInput setTextColor:[self.customResSwitch isOn] ?
     [THTheme getColorNamed:@"TextSettingsBaseColor"] : [THTheme getColorNamed:@"TextSettingsDisbaledColor"]];
    
    [self.endlessLabel setTextColor:[THTheme getColorNamed:[self.customResSwitch isEnabled] ? @"TextSettingsBaseColor" : @"TextSettingsSecondaryDisbaledColor"]];
    [self.endlessLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.customResSwitch setOnTintColor:[THTheme getColorNamed:@"MainColor"]];
    [self.brightnessSwitch setOnTintColor:[THTheme getColorNamed:@"MainColor"]];
    [self.brightnessSelector setTintColor:[THTheme getColorNamed:@"MainColor"]];
    [self.endlessSwitch setOnTintColor:[THTheme getColorNamed:@"MainColor"]];
    
}

- (void) applyThemeCommercial {
    if(self.commercialContainer != nil) {
        [self.commercialTitle setTextColor:[THTheme getColorNamed:@"TextSettingsTitleColor"]];
        [self.commercialTitle setFont:[THTheme getFontNamed:@"TextLFont"]];
    
        [self.commercialDesc setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
        [self.commercialDesc setFont:[THTheme getFontNamed:@"TextMFont"]];
    }
}



#pragma mark - User Interaction

- (void)navigationLeftPressed {
}


#pragma mark - Event Handling

- (void) handleLocalizationChanged {
    [self applyLocalization];
}

- (void) handleKeyboardWillShow:(NSNotification *) note {
    [self.view addGestureRecognizer:self.tapRecognizer];
    
    NSDictionary *info = [note userInfo];
    NSValue *kbFrame = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
    CGRect keyboardFrame = [kbFrame CGRectValue];
    CGFloat height = keyboardFrame.size.height;
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    
    [self respondToKeyboardHeight:height withDuration:animationDuration];
}

- (void) handleKeyboardWillHide:(NSNotification *) note
{
    [self.view removeGestureRecognizer:self.tapRecognizer];

    NSDictionary *info = [note userInfo];
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    
    [self respondToKeyboardHeight:0 withDuration:animationDuration];
}

- (void) respondToKeyboardHeight:(CGFloat)height withDuration:(NSTimeInterval) duration {
    if(height > 0) {
        
        UIView *focused = nil;
        if([self.heightInput isFirstResponder]) {
            focused = self.heightInput;
        }
        if([self.widthInput isFirstResponder]) {
            focused = self.widthInput;
        }
        
        if(focused != nil) {
            CGRect boundsInRootView = [self.view convertRect:self.view.bounds toView:[[self.view superview] superview]];
            UIView *rootView = [[self.view superview] superview];
            CGFloat offset = rootView.bounds.size.height - boundsInRootView.origin.y - boundsInRootView.size.height - 40;
            
            [self.baseScroller setContentInset:UIEdgeInsetsMake(0, 0, height - offset, 0)];
            [self.baseScroller setScrollIndicatorInsets:UIEdgeInsetsMake(0, 0, height - offset, 0)];
        }
        
    } else {
        [self.baseScroller setContentInset:UIEdgeInsetsMake(0, 0, 0, 0)];
        [self.baseScroller setScrollIndicatorInsets:UIEdgeInsetsMake(0, 0, 0, 0)];
    }
}

- (void) didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self.view endEditing:YES];
}

- (void) handleLogIn:(NSNotification *)notification {
    self.userName = notification.userInfo[@"username"];
    [UIView animateWithDuration:0.2 animations:^{
        [self.greetingsLabel setAlpha:1.0];
        [self.usernameInput setAlpha:0.0];
        [self.passwirdInput setAlpha:0.0];
        
        [self.usernameInput setUserInteractionEnabled:NO];
        [self.passwirdInput setUserInteractionEnabled:NO];
        
        self.loginTitle = @"Logout";
        [self.loginBtn setTitle:[NUIAppData getLocalized:self.loginTitle] forState:UIControlStateNormal];
        [self.registerBtn setTitle:[NUIAppData getLocalized: @"Delete account"] forState:UIControlStateNormal];
        
        [self.greetingsLabel setText:[NSString stringWithFormat:@"%@ %@", [NUIAppData getLocalized:@"Welcome"], self.userName]];
    }];
}

- (void) handleLogOut {
    self.userName = @"";
    [self.usernameInput setText:@""];
    [self.passwirdInput setText:@""];
    [UIView animateWithDuration:0.2 animations:^{
        [self.greetingsLabel setAlpha:0.0];
        [self.usernameInput setAlpha:1.0];
        [self.passwirdInput setAlpha:1.0];
        
        [self.usernameInput setUserInteractionEnabled:YES];
        [self.passwirdInput setUserInteractionEnabled:YES];
        
        self.loginTitle = @"Login";
        [self.loginBtn setTitle:[NUIAppData getLocalized:self.loginTitle] forState:UIControlStateNormal];
        [self.registerBtn setTitle:[NUIAppData getLocalized: @"Register"] forState:UIControlStateNormal];
        
    }];
}

- (void) handleDeletedUser {
    [ self handleLogOut ];
}


- (void) handleSignUp {
    
}

- (IBAction)loginPressed:(id)sender {
    
    if([self.loginTitle isEqualToString:@"Login"]) {
        [NUIAppData getService]->login([self.usernameInput.text UTF8String], [self.passwirdInput.text UTF8String]);
        
    } else {
        [NUIAppData getService]->logout();
    }
}

- (IBAction)registerPressed:(id)sender {
    
    if([self.loginTitle isEqualToString:@"Login"]) {
        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
            [APPRegisterPage addOverlayPageIn:self.parentViewController.parentViewController.view withSpecificClass:[APPRegisterPage class]];
            
        } else {
            [APPRegisterPage addOverlayPageIn:self.baseView withSpecificClass:[APPRegisterPage class]];
        }
    } else {
        APPModalMessageObject *messageObject = [APPModalMessageObject messageWithTitle:@"Delete my user account" message:@"This will erase your registered GFXBench account. Are you sure about this?" isFatal:false];
        [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"Yes" style:UIAlertActionStyleDefault handler:^(APPModalMessageAction *action) {
            [NUIAppData getService]->deleteUser();
        }]];
        [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"No" style:UIAlertActionStyleCancel handler:nil]];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                            object:[NUIAppData sharedNUIAppData]
                                                          userInfo:@{
                                                                     @"messageObj":messageObject
                                                                     }
         ];
    }
}

- (IBAction)testInfoPressed:(id)sender {
    BOOL needCloseBtn = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone;
    UIView *root = needCloseBtn ? self.parentViewController.parentViewController.view : self.baseView;
    [APPTestInfoPage addOverlayPageIn:root withSpecificClass:[APPTestInfoPage class] displayingCloseBtn:needCloseBtn];
}

- (IBAction)findOutMorePressed:(id)sender {
    
    BOOL needCloseBtn = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone;
    UIView *root = needCloseBtn ? self.parentViewController.parentViewController.view : self.baseView;
    [APPFindOutMorePage addOverlayPageIn:root withSpecificClass:[APPFindOutMorePage class] displayingCloseBtn:needCloseBtn];
}

- (IBAction)eulaPressed:(id)sender {
    [APPEulaPage addOverlayPageIn:self.parentViewController.parentViewController.view withSpecificClass:[APPEulaPage class]];
}

- (IBAction)clearHistoryPressed:(id)sender {
    APPModalMessageObject *messageObject = [APPModalMessageObject messageWithTitle:@"ClearHistoryDialogTitle" message:@"ClearHistoryDialogBody" isFatal:false];
    [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"ClearAll" style:UIAlertActionStyleDefault handler:^(APPModalMessageAction *action) {
        [NUIAppData getService]->clearResults();
    }]];
    [messageObject addChoice:[APPModalMessageAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_ShowModalMessage
                                                        object:[NUIAppData sharedNUIAppData]
                                                      userInfo:@{
                                                                 @"messageObj":messageObject
                                                                 }
     ];
}

- (IBAction)languageSelectorBtnPressed:(id)sender {
//    BOOL needCloseBtn = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone;
    UIView *root = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone ? self.parentViewController.parentViewController.view : self.baseView;
    [APPLanguageSelectorPage addOverlayPageIn:root withSpecificClass:[APPLanguageSelectorPage class] displayingCloseBtn:NO];
}

- (IBAction)brightnessSelectorValueChange:(id)sender {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setInteger:[self.brightnessSelector selectedSegmentIndex] forKey:APP_UserPreference_BrightnessValue];
    [userDefaults synchronize];
    [NUIAppData getService]->setCustomBrightness([self.brightnessSelector selectedSegmentIndex] / 4.0);
}

- (IBAction)showDesktopValueChanged:(id)sender {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setBool:[self.showDesktopSwitch isOn] forKey:APP_UserPreference_ShowDesktop];
    [userDefaults synchronize];
    
    [NUIAppData getService]->setHideDesktopDevices(![self.showDesktopSwitch isOn]);
}

- (IBAction)customResValueChanged:(id)sender {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setBool:[self.customResSwitch isOn] forKey:APP_UserPreference_CustomRes];
    
    if([self.customResSwitch isOn]) {
        [NUIAppData getService]->setCustomResolution([self.widthInput.text intValue], [self.heightInput.text intValue]);
        [userDefaults setInteger:[self.heightInput.text intValue] forKey:APP_UserPreference_CustomResHeight];
        [userDefaults setInteger:[self.widthInput.text intValue] forKey:APP_UserPreference_CustomResWidth];
        
    } else {
        [NUIAppData getService]->setCustomResolution(-1,-1);
    }
    [userDefaults synchronize];
    
    [self setupResolution];
}

- (IBAction)brightnessValueChanged:(id)sender {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setBool:[self.brightnessSwitch isOn] forKey:APP_UserPreference_Brightness];
    if([self.brightnessSwitch isOn]) {
        [NUIAppData getService]->setCustomBrightness([self.brightnessSelector selectedSegmentIndex] / 4.0);
        [userDefaults setInteger:[self.brightnessSelector selectedSegmentIndex] forKey:APP_UserPreference_BrightnessValue];
        
    } else {
        [NUIAppData getService]->setCustomBrightness(-1);
    }
    [userDefaults synchronize];
    
    [self setupBrightness];
}

- (IBAction)endlessValueChanged:(id)sender {
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    [userDefaults setBool:[self.brightnessSwitch isOn] forKey:APP_UserPreference_Endless];
    [NUIAppData getService]->setEndlessTestRun([self.brightnessSwitch isOn]);
    [userDefaults synchronize];
}

- (IBAction)usernameEditingEnded:(id)sender {
}

- (IBAction)passwordEditingEnded:(id)sender {
}

- (IBAction)widthEditingEnded:(id)sender {
    int newWidth = [self.widthInput.text intValue];
    if(newWidth >= 1) {
        [NUIAppData getService]->setCustomResolution([self.widthInput.text intValue], [self.heightInput.text intValue]);
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setInteger:[self.heightInput.text intValue] forKey:APP_UserPreference_CustomResHeight];
        [userDefaults setInteger:[self.widthInput.text intValue] forKey:APP_UserPreference_CustomResWidth];
        [userDefaults synchronize];
        
    } else {
        self.widthInput.text = @"1024";
    }
}

- (IBAction)heightEditingEnded:(id)sender {
    int newHeight = [self.heightInput.text intValue];
    if(newHeight >= 1) {
        [NUIAppData getService]->setCustomResolution([self.widthInput.text intValue], [self.heightInput.text intValue]);
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        [userDefaults setInteger:[self.heightInput.text intValue] forKey:APP_UserPreference_CustomResHeight];
        [userDefaults setInteger:[self.widthInput.text intValue] forKey:APP_UserPreference_CustomResWidth];
        [userDefaults synchronize];
        
    } else {
        self.widthInput.text = @"768";
    }
}


@end
