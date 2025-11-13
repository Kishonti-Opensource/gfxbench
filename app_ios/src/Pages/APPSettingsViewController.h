/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPSettingsViewController.h
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "NUIBasePage.h"
#import "APPPageNavigator.h"
#import "Utils/NUICursor.h"

@interface APPSettingsViewController : UIViewController<NUIBasePage, APPPageNavigatorDelegate>

@property (weak, nonatomic) IBOutlet UIView *navigatorContainer;

@property (weak, nonatomic) IBOutlet UIView *baseView;
@property (weak, nonatomic) IBOutlet UIScrollView *baseScroller;

@property (weak, nonatomic) IBOutlet UIView *accountContainer;
@property (weak, nonatomic) IBOutlet UILabel *accountTitle;
@property (weak, nonatomic) IBOutlet UILabel *greetingsLabel;
@property (weak, nonatomic) IBOutlet UITextField *usernameInput;
@property (weak, nonatomic) IBOutlet UITextField *passwirdInput;
@property (weak, nonatomic) IBOutlet UIButton *loginBtn;
@property (weak, nonatomic) IBOutlet UIButton *registerBtn;

@property (weak, nonatomic) IBOutlet UIView *informationContainer;
@property (weak, nonatomic) IBOutlet UILabel *informationTitle;
@property (weak, nonatomic) IBOutlet UILabel *informationDesc;

@property (weak, nonatomic) IBOutlet UIView *settingsContainer;
@property (weak, nonatomic) IBOutlet UIButton *testInfoBtn;
@property (weak, nonatomic) IBOutlet UIButton *findOutMoreBtn;
@property (weak, nonatomic) IBOutlet UIButton *eulaBtn;
@property (weak, nonatomic) IBOutlet UIButton *clearHistoryBtn;
@property (weak, nonatomic) IBOutlet UIButton *languageSelectorBtn;
@property (weak, nonatomic) IBOutlet UILabel *showDesktopLabel;
@property (weak, nonatomic) IBOutlet UISwitch *showDesktopSwitch;

@property (weak, nonatomic) IBOutlet UIView *corporateContainer;
@property (weak, nonatomic) IBOutlet UILabel *corporateTitle;
@property (weak, nonatomic) IBOutlet UILabel *endlessLabel;
@property (weak, nonatomic) IBOutlet UISwitch *endlessSwitch;
@property (weak, nonatomic) IBOutlet UILabel *customResLabel;
@property (weak, nonatomic) IBOutlet UISwitch *customResSwitch;
@property (weak, nonatomic) IBOutlet UILabel *widthLabel;
@property (weak, nonatomic) IBOutlet UITextField *widthInput;
@property (weak, nonatomic) IBOutlet UILabel *heightLabel;
@property (weak, nonatomic) IBOutlet UITextField *heightInput;
@property (weak, nonatomic) IBOutlet UILabel *brightnessLabel;
@property (weak, nonatomic) IBOutlet UISwitch *brightnessSwitch;
@property (weak, nonatomic) IBOutlet UISegmentedControl *brightnessSelector;

@property (weak, nonatomic) IBOutlet UIView *commercialContainer;
@property (weak, nonatomic) IBOutlet UILabel *commercialTitle;
@property (weak, nonatomic) IBOutlet UILabel *commercialDesc;

@property (strong, nonatomic) IBOutletCollection(UIView) NSArray *separators;

- (IBAction)loginPressed:(id)sender;
- (IBAction)registerPressed:(id)sender;
- (IBAction)testInfoPressed:(id)sender;
- (IBAction)findOutMorePressed:(id)sender;
- (IBAction)eulaPressed:(id)sender;
- (IBAction)clearHistoryPressed:(id)sender;
- (IBAction)languageSelectorBtnPressed:(id)sender;

- (IBAction)brightnessSelectorValueChange:(id)sender;
- (IBAction)showDesktopValueChanged:(id)sender;
- (IBAction)customResValueChanged:(id)sender;
- (IBAction)brightnessValueChanged:(id)sender;
- (IBAction)endlessValueChanged:(id)sender;

- (IBAction)usernameEditingEnded:(id)sender;
- (IBAction)passwordEditingEnded:(id)sender;
- (IBAction)widthEditingEnded:(id)sender;
- (IBAction)heightEditingEnded:(id)sender;

@end
