/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

#define NGLOG_ENABLE NGLOG_SEVERITY_ANY

@class NUIAppInfo;

@interface NUIAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) NSString *viewName;
@property (strong, nonatomic) NUIAppInfo *info;

@end
