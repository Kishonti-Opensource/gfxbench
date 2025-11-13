/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPHomeViewController.h
//  app_ios
//
//  Created by Balazs Hajagos on 26/08/2015.
//
//

#import <UIKit/UIKit.h>
#import "Pages/NUIBasePage.h"
#import "APPPageNavigator.h"
#import "Controls/APPMainControl.h"
#import "Utils/NUICursorDataSource.h"

@interface APPHomeViewController : UIViewController<NUIBasePage, APPPageNavigatorDelegate, NUICursorDataSourceDelegate, APPMainControlHandler>

@property (weak, nonatomic) IBOutlet UITableView *testTable;
@property (weak, nonatomic) IBOutlet UIView *navigatorHolder;
@property (weak, nonatomic) IBOutlet UIView *startView;
@property (weak, nonatomic) IBOutlet APPMainControl *mainControl;


@property (weak, nonatomic) IBOutlet NSLayoutConstraint *navToFullConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *startToFullConstraintBase;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *startToFullConstraintSecondary;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *tableToFullConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *VerticalToStartConstraintBase;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *VerticalToStartConstraintSecondary;

@end
