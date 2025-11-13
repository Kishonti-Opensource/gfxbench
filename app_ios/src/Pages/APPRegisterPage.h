/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPRegisterPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 10/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "APPPageNavigator.h"
#import "APPOverlayPage.h"
#import "NUIBasePage.h"

@interface APPRegisterPage : APPOverlayPage<NUIBasePage>

@property (weak, nonatomic) IBOutlet UIView *sheet;

@property (weak, nonatomic) IBOutlet UILabel *registerTitle;
@property (weak, nonatomic) IBOutlet UILabel *userLabel;
@property (weak, nonatomic) IBOutlet UITextField *userInput;
@property (weak, nonatomic) IBOutlet UILabel *passLabel;
@property (weak, nonatomic) IBOutlet UITextField *passInput;
@property (weak, nonatomic) IBOutlet UILabel *passAgainLabel;
@property (weak, nonatomic) IBOutlet UITextField *passAgainInput;
@property (weak, nonatomic) IBOutlet UILabel *emailLabel;
@property (weak, nonatomic) IBOutlet UITextField *emailInput;
@property (weak, nonatomic) IBOutlet UIButton *confirmBtn;
@property (weak, nonatomic) IBOutlet UIButton *cancelBtn;

- (IBAction)confirmClicked:(id)sender;
- (IBAction)cancelClicked:(id)sender;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *centerYConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

- (void) respondToKeyboardHeight:(CGFloat)height;

@end
