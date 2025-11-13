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
#import "Utils/NUICursor.h"

@interface APPResultDetailPage : APPOverlayPage<NUIBasePage, NUICursorCallback, UITableViewDataSource, UITableViewDelegate>

@property (weak, nonatomic) IBOutlet UIView *page;
@property (weak, nonatomic) IBOutlet UITableView *table;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

+ (APPResultDetailPage *)addResultDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId;
+ (APPResultDetailPage *)addResultDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId haseCloseBtn:(BOOL)hasCloseBtn;

@end
