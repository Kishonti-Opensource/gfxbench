/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDeviceinfoViewController.h
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "NUIBasePage.h"
#import "APPPageNavigator.h"
#import "Utils/NUICursorDataSource.h"

@interface APPDeviceinfoViewController : UIViewController<NUIBasePage, APPPageNavigatorDelegate, NUICursorDataSourceDelegate>

@property (weak, nonatomic) IBOutlet UIView *navigationHolder;
@property (weak, nonatomic) IBOutlet UIView *page;
@property (weak, nonatomic) IBOutlet UITableView *deviceinfoTable;

@end
