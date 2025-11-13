/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPTestInfoPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 05/10/2015.
//
//

#import "APPTestInfoPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUICursor.h"
#import "Cells/APPTestInfoCell.h"


@interface APPTestInfoPage ()

@property (assign, nonatomic) BOOL isCustomized;

@property (strong, nonatomic) NUICursorDataSource *datasource;
@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end

@implementation APPTestInfoPage

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

- (void)setupTable {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getTests()];
    self.datasource = [[NUIFadeCursorDataSource alloc] initWithCursor:cursor
                                                            Table:self.infoTable
                                                         CellType:[APPTestInfoCell class]
                                                       HeaderType:nil];
    
    self.infoTable.dataSource = self.datasource;
    self.infoTable.delegate = self.datasource;
}

- (void) customize {
    [self.backView addGestureRecognizer:self.tapRecognizer];
    
    [self applyTheme];
    [self applyLocalization];
    [self setupTable];
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
}

- (void) applyTheme {
    [super applyTheme];
    
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



- (void)handleLocalizationChanged {
    [self applyLocalization];
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}

@end
