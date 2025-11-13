/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "descriptors.h"
#import "testinfo.h"
#include "iosdeviceinfocollector.h"
#include "dataformatter.h"
#include "configuration.h"

#include "benchmarkservice.h"
#include "deviceinfo/platformruntimeinfo.h"

#import "Utils/NUIReachability.h"

@interface APPModalMessageAction : NSObject

@property (assign, nonatomic, nonnull) NSString *title;
@property (assign, nonatomic) UIAlertActionStyle style;
@property (copy, nonnull) void (^handler)(APPModalMessageAction* _Nonnull action);

+ (nonnull instancetype)actionWithTitle:(nonnull NSString *)title style:(UIAlertActionStyle)style handler:(void (^ __nullable)(APPModalMessageAction* _Nonnull action))handler;

@end

@interface APPModalMessageObject : NSObject

@property (copy, nonatomic, nonnull) NSString *title;
@property (copy, nonatomic, nonnull) NSString *message;
@property (assign, nonatomic) bool isFatal;

@property (strong, nonatomic, nonnull) NSArray *choices;

+ (nonnull instancetype) messageWithTitle:(nonnull NSString *)title message:(nonnull NSString *)message isFatal:(BOOL)isFatal;

- (void) addChoice:(nonnull APPModalMessageAction *)action;

@end

@interface NUIAppData : NSFileManager

@property (strong, nonatomic, nonnull) NSMutableDictionary *uiStateCache;
@property (strong, nonatomic, nullable) NSString *loginName;

- (void)addNewAlert:(nonnull UIAlertController *)alert;
- (nonnull UIAlertController *)getAlert;

// Lifetime
- (void)Quit;
- (void)Startup;

// Static
+ (nonnull NSString *)ReadDataPath;
+ (nonnull NSString *)RwDataPath;
+ (nonnull NSString *)ConfigPath;
+ (nonnull NSString *)DataPathForTest:(nonnull const tfw::Descriptor *)desc;
+ (nonnull NSString *)DataPath;
+ (nonnull NSString *)ImagePath;
+ (nonnull NSString *)SyncPath;
+ (nonnull NSString *)GraphicsVersionStringFromMajor:(int)major Minor:(int)minor TestId:(nonnull NSString *)tid;

+ (void)addSkipBackupToDirectory:(nonnull NSURL *)URL;

+ (nonnull BenchmarkService *) getService;
+ (nonnull BenchmarkServiceCallback *) getCallback;
+ (nonnull tfw::RuntimeInfo *) getRuntimeInfo;

+ (nonnull NSString *) getLocalized:(nonnull NSString *)s;

+ (void)SetNeedsLatestResults:(BOOL)b;
+ (BOOL)NeedsLatestResults;

// Network
- (void)StopNetNotifier;
- (void)StartNetNotifier;
- (BOOL)isNetAvailable;

// Singleton
+ (nonnull NUIAppData *)sharedNUIAppData;

@end
