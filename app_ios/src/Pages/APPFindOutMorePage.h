/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPFindOutMorePage.h
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import "APPOverlayPage.h"
#import "NUIBasePage.h"

@interface APPFindOutMorePage : APPOverlayPage<NUIBasePage>

@property (weak, nonatomic) IBOutlet UIView *sheet;
@property (unsafe_unretained, nonatomic) IBOutlet UIScrollView *scroller;
@property (unsafe_unretained, nonatomic) IBOutlet UIImageView *productLogo;
@property (unsafe_unretained, nonatomic) IBOutlet UILabel *desc;
@property (unsafe_unretained, nonatomic) IBOutlet UIButton *gfxBtn;
@property (unsafe_unretained, nonatomic) IBOutlet UIButton *kishontiBtn;
@property (unsafe_unretained, nonatomic) IBOutlet UIButton *contactBtn;
@property (unsafe_unretained, nonatomic) IBOutlet UIImageView *kishontiLogo;

@property (strong, nonatomic) IBOutletCollection(UIButton) NSArray *communitySiteLinks;

- (IBAction)linkedInClick:(id)sender;
- (IBAction)facebookClick:(id)sender;
- (IBAction)twitterClick:(id)sender;
- (IBAction)vimeoClick:(id)sender;
- (IBAction)gplusClick:(id)sender;
- (IBAction)tubeClick:(id)sender;
- (IBAction)productClick:(id)sender;
- (IBAction)kishontiClick:(id)sender;
- (IBAction)contactClick:(id)sender;

@property (weak, nonatomic) IBOutlet NSLayoutConstraint *trailingConstraint;

@end
