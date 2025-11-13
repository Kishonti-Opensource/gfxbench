/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPEulaPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import "APPEulaPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"


@interface APPEulaPage ()

@property (assign, nonatomic) BOOL isCustomized;

@end

@implementation APPEulaPage

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
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) customize {
    [self applyTheme];
    [self applyLocalization];
}

#pragma mark - Base Page Overrides

- (void)layoutSubviews {
    if(!self.isCustomized)
        [self customize];
}

- (void) applyLocalization {
    self.title.text = [NUIAppData getLocalized:@"ReadPrivacy"];
    self.desc.text = [NUIAppData getLocalized:@"LicenseDialogBody"];
    
    [self.confirmBtn setTitle:[NUIAppData getLocalized:@"Ok"] forState:UIControlStateNormal];
}

- (void) applyTheme {
    [self.title setTextColor:[THTheme getColorNamed:@"MainColor"]];
    [self.desc setTextColor:[THTheme getColorNamed:@"TextSettingsBaseColor"]];
    
    [self.title setFont:[THTheme getFontNamed:@"TitleLFont"]];
    [self.desc setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.confirmBtn setTitleColor:[THTheme getColorNamed:@"BackColor"] forState:UIControlStateNormal];
    [self.confirmBtn setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    
    [self.confirmBtn.layer setCornerRadius:4.0];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRoundedRect:self.scrollerBack.bounds cornerRadius:8.0];
    self.scrollerBack.layer.masksToBounds = NO;
    self.scrollerBack.layer.cornerRadius = 8;
    self.scrollerBack.layer.shadowColor = [UIColor blackColor].CGColor;
    self.scrollerBack.layer.shadowRadius = 10;
    self.scrollerBack.layer.shadowOpacity = 0.5f;
    self.scrollerBack.layer.shadowOffset = CGSizeMake(0, 0);
    self.scrollerBack.layer.shadowPath = shadowPath.CGPath;
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
}

- (IBAction)ConfirmClick:(id)sender {
    [super disappearWithCompletion:nil];
}

@end
