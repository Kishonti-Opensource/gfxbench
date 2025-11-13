/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultViewController.h
//  app_ios
//
//  Created by Balazs Hajagos on 31/08/2015.
//
//

#import <UIKit/UIKit.h>
#import "APPPageNavigator.h"
#import "NUIBasePage.h"
#import "Utils/NUICursorDataSource.h"

@interface APPResultViewController : UIViewController<NUIBasePage, APPPageNavigatorDelegate, NUICursorDataSourceDelegate>

@property (weak, nonatomic) IBOutlet UIView *navigatorHolder;
@property (weak, nonatomic) IBOutlet UIView *tablesHolder;
@property (weak, nonatomic) IBOutlet UITableView *resultTable;
@property (weak, nonatomic) IBOutlet UITableView *historyTable;

@end
