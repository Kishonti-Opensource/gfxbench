/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUIBaseCell.h
//  app_ios
//
//  Created by Balazs Hajagos on 26/08/2015.
//
//

#import <UIKit/UIKit.h>
#import "Utils/NUICursor.h"

@protocol NUICellInterface <NSObject>

+ (NSString *)getCellId;
+ (NSString *)getNibName;
+ (BOOL)needsDynamicLayout;

- (void)applyTheme;
- (void)displayAsLastInGroup;

- (void)setupWithCursor:(NUICursor *)cursor;
- (void)setupIcon:(NUICursor *)cursor;
- (void)setupTitle:(NUICursor *)cursor;
- (void)setupSubtitle:(NUICursor *)cursor;

@end

@interface NUIBaseCell : UITableViewCell<NUICellInterface>

@property (weak, nonatomic) IBOutlet UIView *cellBack;
@property (weak, nonatomic) IBOutlet UIView *lastLineBack;
@property (weak, nonatomic) IBOutlet UIView *separatorBack;

@property (weak, nonatomic) IBOutlet UILabel *titleLabel;
@property (weak, nonatomic) IBOutlet UILabel *subtitleLabel;
@property (weak, nonatomic) IBOutlet UIImageView *icon;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *separatorHeightConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *lastLineHeightConstraint;

- (void) setupWithCursor:(NUICursor *)cursor;

@end
