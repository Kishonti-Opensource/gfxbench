/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUIResultDetailView.h"

@interface NUIResultDetailView()
@property (copy, nonatomic) NSString *resultJsonString;
@property (copy, nonatomic) NSString *resultName;
@property (copy, nonatomic) NSString *flaggedReason;
@end

@implementation NUIResultDetailView


- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
    }
    return self;
}

- (void) setResultJsonString:(NSString *)str andResultName:(NSString *)name withflaggedReason:(NSString *)flag {
    [self loadHTMLString:@"Loading..." baseURL:nil];
    self.resultJsonString = str;
    self.resultName = name;
    self.flaggedReason = flag;

    [self loadResults];

    self.navigationDelegate = self;

    [self loadRequest:[NSURLRequest requestWithURL:[NSURL fileURLWithPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"/resultdetail/result_ng.html"] isDirectory:NO]]];
}


- (void)loadResults
{
    NSString *fileName = @"diagram.json";
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *applicationSupportDirectory = [paths firstObject];

    NSString *fileAtPath = [applicationSupportDirectory stringByAppendingPathComponent:@"/"];
    fileAtPath = [fileAtPath stringByAppendingPathComponent:fileName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fileAtPath]) {
        [[NSFileManager defaultManager] createFileAtPath:fileAtPath contents:nil attributes:nil];
    }

    [[self.resultJsonString dataUsingEncoding:NSUTF8StringEncoding] writeToFile:fileAtPath atomically:NO];
}

#pragma mark - Webview Delegate

- (void)webViewDidFinishLoad:(WKWebView *)webView {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *applicationSupportDirectory = [paths firstObject];

    NUIResultDetailView *resDetailView = ((NUIResultDetailView *)webView);
    NSString *updateString = [NSString stringWithFormat:@"%@/diagram.json", applicationSupportDirectory];
    updateString = [NSString stringWithFormat:@"updateResult('file://%@', '%@', %d, '%@')", updateString, resDetailView.resultName, ![resDetailView.flaggedReason isEqualToString:@""], resDetailView.flaggedReason];

    [webView evaluateJavaScript:updateString completionHandler:nil];
}

@end
