/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDeviceDetailPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 27/10/2015.
//
//

#import <UIKit/UIKit.h>
#import "APPOverlayPage.h"
#import "NUIBasePage.h"
#import "Utils/NUICursor.h"

@interface APPDeviceDetailPage : APPOverlayPage<NUIBasePage, NUICursorCallback, UITableViewDataSource, UITableViewDelegate>

@property (weak, nonatomic) IBOutlet UIView *page;
@property (weak, nonatomic) IBOutlet UITableView *table;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *centerYConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

+ (APPDeviceDetailPage *)addDeviceDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId;
+ (APPDeviceDetailPage *)addDeviceDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId hasCloseBtn:(BOOL)hasCloseBtn;

@end
