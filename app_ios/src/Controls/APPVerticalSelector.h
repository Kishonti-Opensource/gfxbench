/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPVerticalSelector.h
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "Cells/APPTestCell.h"

@class APPVerticalSelector;

@protocol APPVerticalSelectorDelegate <NSObject>

@optional
- (void)leftBtnPressedForSelector:(APPVerticalSelector *)selector;
- (void)rightBtnPressedForSelector:(APPVerticalSelector *)selector;

@end

@interface APPVerticalSelector : UIView
@property (weak, nonatomic) id<APPVerticalSelectorDelegate> delegate;

@property (weak, nonatomic) IBOutlet UIButton *leftBtn;
@property (weak, nonatomic) IBOutlet UIButton *rightBtn;
@property (weak, nonatomic) IBOutlet UIView *cellBack;
@property (weak, nonatomic) IBOutlet UILabel *title;
@property (weak, nonatomic) IBOutlet UIImageView *icon;

- (IBAction)leftBtnPressed:(id)sender;
- (IBAction)rightBtnPressed:(id)sender;

- (void)updateFromCursor:(NUICursor *)cursor;
- (void)applyTheme;

+ (APPVerticalSelector *)addSelectorIn:(UIView *)holder;

@end
