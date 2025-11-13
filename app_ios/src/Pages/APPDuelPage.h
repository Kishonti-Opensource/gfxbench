/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDuelPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 17/11/15.
//
//

#import <UIKit/UIKit.h>
#import "APPPageNavigator.h"
#import "APPOverlayPage.h"
#import "NUIBasePage.h"
#import "Utils/NUICursorDataSource.h"

@interface APPDuelPage : APPOverlayPage<NUIBasePage, NUICursorDataSourceDelegate>

@property (weak, nonatomic) IBOutlet UIView *page;
@property (weak, nonatomic) IBOutlet UITableView *table;
@property (weak, nonatomic) IBOutlet UIView *duelHeader;
@property (weak, nonatomic) IBOutlet UIImageView *leftImage;
@property (weak, nonatomic) IBOutlet UIImageView *rightImage;
@property (weak, nonatomic) IBOutlet UILabel *leftName;
@property (weak, nonatomic) IBOutlet UILabel *rightName;
@property (weak, nonatomic) IBOutlet UILabel *leftSubtitle;
@property (weak, nonatomic) IBOutlet UILabel *rightSubtitle;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *centerYConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

+ (APPDuelPage *)addDuelPageIn:(UIView *)holder
                   deviceAName:(NSString *)nameA
               deviceASubtitle:(NSString *)subtitleA
                  deviceAImage:(NSString *)imageA
                       forAApi:(NSString *)apiA
                   deviceBName:(NSString *)nameB
               deviceBSubtitle:(NSString *)subtitleB
                  deviceBImage:(NSString *)imageB
                       forBApi:(NSString *)apiB
                   hasCloseBtn:(BOOL)hasCloseBtn;

+ (APPDuelPage *)addDuelPageIn:(UIView *)holder
                   deviceAName:(NSString *)nameA
               deviceASubtitle:(NSString *)subtitleA
                  deviceAImage:(NSString *)imageA
                       forAApi:(NSString *)apiA
                   deviceBName:(NSString *)nameB
               deviceBSubtitle:(NSString *)subtitleB
                  deviceBImage:(NSString *)imageB
                       forBApi:(NSString *)apiB;

@end
