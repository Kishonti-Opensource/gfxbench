/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "result.h"
#import "resultitem.h"

@interface NUIUtilities : NSObject

+ (NSDateFormatter *)UIDateFormatter;
+ (NSNumber *)ApplyMetric:(NSString *)metric To:(double)num;
+ (NSString *)getStatusString:(NSString *)status;
+ (NSString *)getStatusStringFromEnum:(tfw::Result::Status)status;
+ (NSArray *)getStatusString:(NSString *)status withError:(NSString *)error;
+ (NSString *)getUnit:(NSString *)unit withScore:(NSNumber *)score withFps:(NSNumber *)fps;
+ (NSString *)getDuelFormattedString:(NSNumber *)number;
+ (NSString *) getFormattedResult:(double)score;
+ (NSDictionary *)getSystemVersion;
+ (BOOL) BundleVersionIsGreaterThanOrEqual:(NSString *)targetVersion;
+ (NSString *) getProductId;
+ (NSString *) getProductBaseApi;
+ (NSString *) getVersionString;
+ (CGRect)getSizeForLabel:(UILabel *)l withMaxWidth:(CGFloat)width;
+ (NSAttributedString *)getHeaderVersionStringWithBase:(NSString *)base;

+ (NSDictionary *)getDiagramInfo:(tfw::ResultItem)result;
+ (BOOL)shouldGetGeekyDetails:(NSString *)name;

@end
