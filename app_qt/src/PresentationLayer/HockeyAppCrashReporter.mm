/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "HockeyAppCrashReporter.h"

#import <Foundation/Foundation.h>
#import <HockeySDK/HockeySDK.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSViewController.h>
#import <Foundation/NSURLConnection.h>



@interface BITAppDelegate : NSResponder <NSApplicationDelegate>
    @property (nonatomic, strong) NSWindow *window;
    @property (nonatomic, strong) NSViewController *rootViewController;
    - (void)setup:(NSString *)appId logFilePath:(NSString *)logPath;
@end



NSString *g_logPath = nil;

@interface BITAppDelegate() <BITHockeyManagerDelegate, BITCrashManagerDelegate> {}
@end



@implementation BITAppDelegate
- (NSString *)applicationLogForCrashManager:(BITCrashManager *)crashManager
{
    if(g_logPath) {
        NSError *error;
        NSString *log = [[NSString alloc] initWithContentsOfFile:g_logPath encoding:NSASCIIStringEncoding error:&error];
        if(log) {
            return log;
        }
    }
    return @"";
}



- (void)setup:(NSString *)appId logFilePath:(NSString *)logPath
{
    if(logPath) {
        g_logPath = logPath;
    }
    [[BITHockeyManager sharedHockeyManager] setDelegate: self];
    [[BITHockeyManager sharedHockeyManager] configureWithIdentifier: appId];
    [[BITHockeyManager sharedHockeyManager].crashManager setAutoSubmitCrashReport: YES];
    [[BITHockeyManager sharedHockeyManager] setDebugLogEnabled: YES];
    [[BITHockeyManager sharedHockeyManager] startManager];
}
@end



BITAppDelegate *delegate = nil;



HockeyAppCrashReporter::HockeyAppCrashReporter(const ApplicationConfig &applicationConfig):
    m_applicationConfig(&applicationConfig)
{
}



HockeyAppCrashReporter::~HockeyAppCrashReporter()
{
    if(delegate) {
        [delegate release];
    }
}



void HockeyAppCrashReporter::start()
{
    std::string appId = m_applicationConfig->crashReporterToken();
    std::string logPath = m_applicationConfig->logPath();
    NSString *nslogPath = nil;
    if(!logPath.empty())
    {
        nslogPath = [NSString stringWithCString:logPath.c_str() encoding:NSASCIIStringEncoding];
    }
    if(!appId.empty())
    {
        NSString *nsid = [NSString stringWithCString:appId.c_str() encoding:NSASCIIStringEncoding];
        delegate = [[BITAppDelegate alloc] init];
        [delegate setup: nsid logFilePath:nslogPath];
    }
}
