/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPLanguageSelectorPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 11/01/16.
//
//

#import "APPLanguageSelectorPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"


NSString * const APP_UserPreference_Locale = @"Locale";

@interface APPLanguageSelectorPage()

@property (assign, nonatomic) BOOL isCustomized;
@property (strong, nonatomic) NSArray *localizations;
@property (strong, nonatomic) NUICursor *localizationCursor;

@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end

@implementation APPLanguageSelectorPage

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
    
    self.localizationCursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getAvailableLanguages()];
    [self.localizationCursor setCallback:self];
}

- (void) customize {
    [self.backView addGestureRecognizer:self.tapRecognizer];
    [self.languagePicker setDataSource:self];
    [self.languagePicker setDelegate:self];
    
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
    self.pageTitle.text = [NUIAppData getLocalized:@"SelectLanguage"];
}

- (void) applyTheme {
    [super applyTheme];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        [self.backView setBackgroundColor:[UIColor clearColor]];
        if(self.blurView != nil) {
            [self.blurView setHidden:NO];
        }
    }
    [self.pageTitle setTextColor:[THTheme getColorNamed:@"MainColor"]];
    [self.pageTitle setFont:[THTheme getFontNamed:@"TitleLFont"]];
    
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

- (void)handleLocalizationChanged {
    [self applyLocalization];
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}


#pragma mark - NUICursorCallback
- (void) dataSetChangedFrom:(NSInteger)from to:(NSInteger)to {
    [self updateLocalizationArray];
}

- (void) dataSetInvalidated {
    [self updateLocalizationArray];
}

- (void) updateLocalizationArray {
    NSLocale *locale = [NSLocale autoupdatingCurrentLocale];
    
    NSInteger selectedIndex = -1;
    NSInteger defaultIndex = -1;
    NSString *selectedLocale = [NSString stringWithUTF8String:[NUIAppData getService]->getConfig(BenchmarkService::ConfigKey::LOCALE).c_str()];
    NSMutableArray *newLocArray = [[NSMutableArray alloc] init];
    for(NSInteger i = 0; i<self.localizationCursor.count; ++i) {
        [self.localizationCursor moveToPosition:i];
        NSString *addLocale = [self.localizationCursor getStringInColumn:@"title"];
        
        if([selectedLocale isEqualToString:addLocale])
            selectedIndex = i;
        if([addLocale isEqualToString:@"en"])
            defaultIndex = i;
        
        [newLocArray addObject:[locale displayNameForKey:NSLocaleIdentifier value:addLocale]];
    }
    self.localizations = newLocArray;
    
    [self.languagePicker reloadAllComponents];
    
    if(selectedIndex < 0) {
        selectedIndex = defaultIndex >= 0 ? defaultIndex : 0;
    }
    
    [self.languagePicker selectRow:selectedIndex inComponent:0 animated:NO];
}


#pragma mark - UIPickerViewDataSource/Delegate
- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component {
    if(self.localizations != nil)
        return [self.localizations count];
    return 0;
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView {
    return 1;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component {
    return self.localizations[row];
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
    NSString *selectedLocalization = @"";
    if(self.localizationCursor.count > 0) {
        [self.localizationCursor moveToPosition:row];
        selectedLocalization = [self.localizationCursor getStringInColumn:@"title"];
        
        [self disappearWithCompletion:^(BOOL finished){
            if(finished) {
                [NUIAppData getService]->setConfig(BenchmarkService::ConfigKey::LOCALE, [selectedLocalization UTF8String]);
                NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
                [userDefaults setObject:selectedLocalization forKey:APP_UserPreference_Locale];
                [userDefaults synchronize];
            }
        }];
    }
}

@end
