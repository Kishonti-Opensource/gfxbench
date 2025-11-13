/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import "Controls/NUICustomTB.h"

@class NUIAppInfo;

@interface NUIRootViewController : UIViewController <UIScrollViewDelegate, NUICustomTabbarDelegate>

@property (nonatomic, strong) NSArray *contentList;

@end
