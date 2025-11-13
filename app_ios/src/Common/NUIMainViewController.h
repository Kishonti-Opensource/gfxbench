/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

@class NUITestInfoViewController;
@class NUILoadingViewController;
@class NUITestViewController;
@class NUITestSelectorViewController;
@class NUIBatteryDiagramViewController;

@class NUICustomTB;

@interface NUIMainViewController : UIViewController
@property (weak, nonatomic) IBOutlet UIView *FullViewport;
@property (weak, nonatomic) IBOutlet UIView *MainContainer;

@end
