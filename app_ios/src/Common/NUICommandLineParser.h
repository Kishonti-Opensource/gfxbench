/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import <Poco/Util/Option.h>
#import "descriptors.h"

@interface NUICommandLineParser : NSObject

- (void)parse;
- (tfw::DescriptorList)getDescriptors;


- (NSString *)getInputPath;
- (NSString *)getDescPath;
- (NSString *)getOutputPathWithDefault:(NSString *)default_path;

@end
