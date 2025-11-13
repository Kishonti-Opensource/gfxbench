/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPTestInfoPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 05/10/2015.
//
//

#import "APPOverlayPage.h"
#import "Utils/NUICursorDataSource.h"
#import "Utils/NUIFadeCursorDataSource.h"

@interface APPTestInfoPage : APPOverlayPage


@property (weak, nonatomic) IBOutlet UIView *sheet;
@property (weak, nonatomic) IBOutlet UITableView *infoTable;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *bottomConstraint;

@end
