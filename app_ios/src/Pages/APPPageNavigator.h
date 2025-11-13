/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPPageNavigator.h
//  app_ios
//
//  Created by Balazs Hajagos on 28/08/2015.
//
//

#import <UIKit/UIKit.h>

@protocol APPPageNavigatorDelegate <NSObject>

@optional
- (void)navigationLeftPressed;
- (void)navigationRightPressed;

@end

@interface APPPageNavigator : UIView
@property (weak, nonatomic) id<APPPageNavigatorDelegate> delegate;

@property (weak, nonatomic) IBOutlet UIButton *leftBtn;
@property (weak, nonatomic) IBOutlet UIButton *rightBtn;
@property (weak, nonatomic) IBOutlet UILabel *title;
@property (weak, nonatomic) IBOutlet UILabel *subtitle;

- (IBAction)leftBtnPressed:(id)sender;
- (IBAction)rightBtnPressed:(id)sender;

+ (APPPageNavigator *)addNavigatorIn:(UIView *)holder;

@end
