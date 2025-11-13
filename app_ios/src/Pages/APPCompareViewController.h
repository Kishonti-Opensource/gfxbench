/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPCompareViewContoller.h
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "NUIBasePage.h"
#import "APPPageNavigator.h"
#import "Controls/APPVerticalSelector.h"
#import "Utils/NUICursorDataSource.h"

@interface APPCompareViewController : UIViewController<NUIBasePage, APPPageNavigatorDelegate, NUICursorDataSourceDelegate, APPVerticalSelectorDelegate, UISearchBarDelegate, NUICursorCallback>

@property (weak, nonatomic) IBOutlet UIView *navigatorHolder;
@property (weak, nonatomic) IBOutlet UIView *pageBack;
@property (weak, nonatomic) IBOutlet UITableView *testlistTable;
@property (weak, nonatomic) IBOutlet UITableView *compareTable;
@property (weak, nonatomic) IBOutlet UIView *compareBack;
@property (weak, nonatomic) IBOutlet UIView *testSelectorHolder;
@property (weak, nonatomic) IBOutlet UIView *userResultHolder;
@property (weak, nonatomic) IBOutlet UIView *searchHolder;
@property (weak, nonatomic) IBOutlet UISearchBar *searchbar;

@property (weak, nonatomic) IBOutlet UIView *userProgress;
@property (weak, nonatomic) IBOutlet UILabel *userTitle;
@property (weak, nonatomic) IBOutlet UILabel *userSubtitle;
@property (weak, nonatomic) IBOutlet UIImageView *userIcon;
@property (weak, nonatomic) IBOutlet UIView *userScoreBack;
@property (weak, nonatomic) IBOutlet UILabel *userMainScore;
@property (weak, nonatomic) IBOutlet UILabel *userSecondaryScore;


@property (weak, nonatomic) IBOutlet NSLayoutConstraint *userSecondaryScoreHeightConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *userProgressConstraint;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *testSelectorHeightConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *searchholderBottomConstraint;

@end
