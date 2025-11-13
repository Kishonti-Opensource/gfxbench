/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import "Controls/THProgressBar.h"

@interface NUILoadingViewController : UIViewController

@property (weak, nonatomic) IBOutlet UIImageView *BackView;
@property (weak, nonatomic) IBOutlet UIImageView *LogoView;
@property (weak, nonatomic) IBOutlet UILabel *ActivityLabel;
@property (weak, nonatomic) IBOutlet THProgressBar *progressbar;
@property (strong, nonatomic) NSTimer *RefreshTimer;

- (void)fadeOut;

@end
