/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPEulaPage.h
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import "APPOverlayPage.h"
#import "NUIBasePage.h"

@interface APPEulaPage : APPOverlayPage<NUIBasePage>

@property (weak, nonatomic) IBOutlet UIView *scrollerBack;
@property (weak, nonatomic) IBOutlet UIScrollView *scroller;
@property (weak, nonatomic) IBOutlet UILabel *title;
@property (weak, nonatomic) IBOutlet UILabel *desc;
@property (weak, nonatomic) IBOutlet UIButton *confirmBtn;

- (IBAction)ConfirmClick:(id)sender;

@end
