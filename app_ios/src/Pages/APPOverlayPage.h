/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPOverlayPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import <UIKit/UIKit.h>
#import "NUIBasePage.h"

@interface APPOverlayPage : UIView<NUIBasePage>

@property (weak, nonatomic) IBOutlet UIView *backView;
@property (weak, nonatomic) IBOutlet UIView *blurView;
@property (weak, nonatomic) IBOutlet UIButton *closeBtn;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *closeHidingBottomConstraint;

+ (APPOverlayPage *)addOverlayPageIn:(UIView *)holder withSpecificClass:(Class)overlayClass;
+ (APPOverlayPage *)addOverlayPageIn:(UIView *)holder withSpecificClass:(Class)overlayClass displayingCloseBtn:(BOOL)addCloseBtn;

- (IBAction)closeBtnClick:(id)sender;

- (void) displayCloseBtn;
- (void) disappearWithCompletion:(void (^)(BOOL finished))completion;

@end
