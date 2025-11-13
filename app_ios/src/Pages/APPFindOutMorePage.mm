/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPFindOutMorePage.m
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import "APPFindOutMorePage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"


@interface APPFindOutMorePage ()

@property (assign, nonatomic) BOOL isCustomized;
@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end

@implementation APPFindOutMorePage

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
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) customize {
    [self.backView addGestureRecognizer:self.tapRecognizer];
    
    [self applyTheme];
    [self applyLocalization];
}

#pragma mark - Base Page Overrides

- (void)layoutSubviews {
    if(!self.isCustomized)
        [self customize];
}

- (void) applyLocalization {
    [super applyLocalization];
    self.desc.text = [NUIAppData getLocalized:@"FollowUsSectionBody"];
    [self.gfxBtn setTitle:@"gfxbench.com" forState:UIControlStateNormal];
    [self.kishontiBtn setTitle:@"kishonti.net" forState:UIControlStateNormal];
    [self.contactBtn setTitle:[NUIAppData getLocalized:@"ContactUs"] forState:UIControlStateNormal];
}

- (void) applyTheme {
    [super applyTheme];
    [self.desc setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    [self.desc setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.gfxBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.kishontiBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.contactBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.gfxBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.kishontiBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.contactBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    
    
    [self.gfxBtn.layer setCornerRadius:4.0];
    [self.kishontiBtn.layer setCornerRadius:4.0];
    [self.contactBtn.layer setCornerRadius:4.0];
    
    [self.sheet setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.scroller setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
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


#pragma mark - btns

- (IBAction)linkedInClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://www.linkedin.com/company/kishonti-informatics"] options:@{} completionHandler:nil];
}

- (IBAction)facebookClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://www.facebook.com/KishontiLtd"] options:@{} completionHandler:nil];
}

- (IBAction)twitterClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://twitter.com/KishontiI"]options:@{} completionHandler:nil];
}

- (IBAction)vimeoClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://vimeo.com/user11934111"] options:@{} completionHandler:nil];
}

- (IBAction)gplusClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://plus.google.com/u/0/b/113561619535028544544/113561619535028544544"] options:@{} completionHandler:nil];
}

- (IBAction)tubeClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://www.youtube.com/user/KishontiLtd"] options:@{} completionHandler:nil];
}

- (IBAction)productClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://gfxbench.com"] options:@{} completionHandler:nil];
}

- (IBAction)kishontiClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://kishonti.net"] options:@{} completionHandler:nil];
    }

- (IBAction)contactClick:(id)sender {
    [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"mailto:help@gfxbench.com"] options:@{} completionHandler:nil];
}


-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}

@end
