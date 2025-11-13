/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

@interface NUIResultDetailView : WKWebView<WKNavigationDelegate>

- (void) setResultJsonString:(NSString *)str andResultName:(NSString *)name withflaggedReason:(NSString *)flag;

@end
