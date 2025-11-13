/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPRegisterPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 10/09/2015.
//
//

#import "APPRegisterPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"


@interface APPRegisterPage ()

@property (assign, nonatomic) BOOL isCustomized;

@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end

@implementation APPRegisterPage

- (instancetype) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if(self) {
        [self setup];
    }
    return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if(self) {
        [self setup];
    }
    return self;
}


#pragma mark - Page Setup
- (void) setup {
    self.isCustomized = NO;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleSuccessfulSignup) name:NUI_Notification_SignedUp object:nil];
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
}

- (void) customize {
    [self.backView addGestureRecognizer:self.tapRecognizer];
    
    [self applyTheme];
    [self applyLocalization];
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Base Page Overrides

- (void)layoutSubviews {
    [super layoutSubviews];
    if(!self.isCustomized)
        [self customize];
}

- (void) applyLocalization {
    [super applyLocalization];
    self.registerTitle.text = [NUIAppData getLocalized:@"RegistrationForm"];
    self.userLabel.text = [NUIAppData getLocalized:@"Username"];
    self.passLabel.text = [NUIAppData getLocalized:@"Password"];
    self.passAgainLabel.text = [NUIAppData getLocalized:@"RegisterPasswordAgain"];
    self.emailLabel.text = [NUIAppData getLocalized:@"Email"];
    
    self.userInput.placeholder = [NUIAppData getLocalized:@"Username"];
    self.passInput.placeholder = [NUIAppData getLocalized:@"Password"];
    self.passAgainInput.placeholder = [NUIAppData getLocalized:@"RegisterPasswordAgain"];
    self.emailInput.placeholder = [NUIAppData getLocalized:@"Email"];
    
    [self.confirmBtn setTitle:[NUIAppData getLocalized:@"Confirm"] forState:UIControlStateNormal];
    [self.cancelBtn setTitle:[NUIAppData getLocalized:@"Cancel"] forState:UIControlStateNormal];
}

- (void) applyTheme {
    [super applyTheme];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        [self.backView setBackgroundColor:[UIColor clearColor]];
        if(self.blurView != nil) {
            [self.blurView setHidden:NO];
        }
    }
    [self.registerTitle setTextColor:[THTheme getColorNamed:@"MainColor"]];
    [self.userLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.passLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.passAgainLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.emailLabel setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    
    [self.registerTitle setFont:[THTheme getFontNamed:@"TitleLFont"]];
    [self.userLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.passLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.passAgainLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.emailLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.confirmBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.cancelBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.confirmBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.cancelBtn setBackgroundColor:[THTheme getColorNamed:@"BtnNegativeColor"]];
    
    [self.confirmBtn.layer setCornerRadius:4.0];
    [self.cancelBtn.layer setCornerRadius:4.0];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRoundedRect:self.sheet.bounds cornerRadius:8.0];
    self.sheet.layer.masksToBounds = NO;
    self.sheet.layer.cornerRadius = 8;
    self.sheet.layer.shadowColor = [UIColor blackColor].CGColor;
    self.sheet.layer.shadowRadius = 10;
    self.sheet.layer.shadowOpacity = 0.5f;
    self.sheet.layer.shadowOffset = CGSizeMake(0, 0);
    self.sheet.layer.shadowPath = shadowPath.CGPath;
}


#pragma mark - Animation Overrides

- (void) overlayWillAppear {
    [self setAlpha:0.0];
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
}

- (void) overlayAppearAnimation {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:8];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 1.0;
        [self layoutIfNeeded];
    }];
}

- (void) disappearWithCompletion:(void (^)(BOOL finished))completion; {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 0.0;
        [self layoutIfNeeded];
        
    } completion:^(BOOL finished) {
        if(finished) {
            [self removeFromSuperview];
            if(completion != nil)
                completion(finished);
        }
    }];
}


#pragma mark - Event Handling

- (void) handleKeyboardWillShow:(NSNotification *) note {
    NSDictionary *info = [note userInfo];
    NSValue *kbFrame = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    CGRect keyboardFrame = [kbFrame CGRectValue];
    
    CGFloat height = keyboardFrame.size.height;
    
    [self respondToKeyboardHeight:height];
    
    [UIView animateWithDuration:MAX(animationDuration,0.1) animations:^{
        [self layoutIfNeeded];
    }];
}

- (void) handleKeyboardWillHide:(NSNotification *) note {
    NSDictionary *info = [note userInfo];
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    
    [self respondToKeyboardHeight:0];
    [UIView animateWithDuration:MAX(animationDuration,0.1) animations:^{
        [self layoutIfNeeded];
    }];
}

- (void)handleLocalizationChanged {
    [self applyLocalization];
}

- (IBAction)confirmClicked:(id)sender {
    [NUIAppData getService]->signUp([self.emailInput.text UTF8String],
                                    [self.userInput.text UTF8String],
                                    [self.passInput.text UTF8String]);
}

- (IBAction)cancelClicked:(id)sender {
    [self disappearWithCompletion:nil];
}

- (IBAction)backTap:(id)sender {
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}

- (void)handleSuccessfulSignup {
    [self disappearWithCompletion:nil];
}

- (void) respondToKeyboardHeight:(CGFloat)height {
    CGFloat diff = (self.sheet.bounds.size.height/2.0 + 20) - (self.bounds.size.height/2.0 - height);
    if(diff > 0) {
        [self.centerYConstraint setConstant:-diff];
    } else {
        [self.centerYConstraint setConstant:0];
    }
}

@end
