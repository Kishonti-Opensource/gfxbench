/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUIColoredImage.h
//  benchmark
//
//  Created by kishonti on 25/08/2014.
//
//

#import <UIKit/UIKit.h>

@interface UIImage (NUITint)

+ (UIImage *)imageWithContentsOfFile:(NSString *)path withTintColor:(UIColor *)color;

@end
