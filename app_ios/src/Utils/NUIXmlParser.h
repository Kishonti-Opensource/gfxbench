/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>

@interface NUIXmlParser : NSObject <NSXMLParserDelegate>

@property (strong, nonatomic) NSMutableArray *netTests;
@property (strong, nonatomic) NSMutableArray *netDevices;
@property (strong, nonatomic) NSMutableDictionary *netMaximum;
@property (strong, nonatomic) NSMutableDictionary *netResults;
@property (strong, nonatomic) NSMutableDictionary *test;
@property (strong, nonatomic) NSMutableDictionary *result;
@property (strong, nonatomic) NSMutableDictionary *deviceDict;
@property (copy, nonatomic) NSString *currentDevice;
@property (assign, nonatomic) BOOL errorParsing;

- (id)init;
- (NSMutableDictionary *)parseXml:(NSData *)data;

@end
