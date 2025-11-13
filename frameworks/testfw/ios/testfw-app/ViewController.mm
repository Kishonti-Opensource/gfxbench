/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "ViewController.h"
#import "NSFileManagerLocations.h"
#import "ng/log.h"
#import "testfw.h"
#import "schemas/descriptors.h"
#import "AppDelegate.h"


@interface ViewController ()

@end


@implementation ViewController


- (void)viewDidLoad
{
    [super viewDidLoad];

    [self.view layoutIfNeeded];
    [self initContext];
    [self setupMainViewWithContext:ctx_];
    [self prefersStatusBarHidden];

    // Hide status bar.
    if ([self respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)]) {
        // iOS 7
        [self performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];
    }
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

-(void)viewDidAppear:(BOOL)animated
{
    dispatch_queue_t runner_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, (unsigned long)NULL);
    dispatch_async(runner_queue, ^(void)
    {
        [self start];
    });
}

- (void)setTest:(tfw::TestBase *)test
{
    test_ = test;
}

- (void)initContext
{
    ctx_ = nil;
}

- (void)setupMainViewWithContext:(GraphicsContext *)context
{

}

-(void)swipe:(UISwipeGestureRecognizer*)gestureRecognizer
{
	if (test_) {
		test_->cancel();
	}
}

- (void)start
{
    using namespace tfw;

    [UIApplication sharedApplication].idleTimerDisabled=YES;

    tfw::Descriptor d;
    tfw::Descriptor::fromJsonString(test_->config().c_str(), &d);

    [self setupEnv:&d];

	UISwipeGestureRecognizer * swipe=[[UISwipeGestureRecognizer alloc]initWithTarget:self action:@selector(swipe:)];
	swipe.direction=UISwipeGestureRecognizerDirectionRight;
	[self.view addGestureRecognizer:swipe];

    test_->setConfig(d.toJsonString());
    test_->setGraphicsContext(self->ctx_);
	NGLOG_INFO("Initializing test %s", test_->name());
	if (test_->init()) {
		NGLOG_INFO("Running test %s", test_->name());
		test_->run();
	} else {
		NGLOG_ERROR("failed to init test: %s", d.testId());
	}
	[self.view removeGestureRecognizer:swipe];

    NSString* savePath = [[NSFileManager defaultManager] applicationSupportDirectory];
    savePath = [savePath stringByAppendingString:@"/results/"];
    [self saveResult:test_->result() fileName:[NSString stringWithUTF8String:d.testId().c_str()] toPath:savePath];

    [UIApplication sharedApplication].idleTimerDisabled=NO;

    dispatch_queue_t main_queue = dispatch_get_main_queue();
    dispatch_async(main_queue, ^{
        [self.navigationController popViewControllerAnimated:true];
    });
}

- (void)saveResult:(std::string)jsonString fileName:(NSString *)name toPath:(NSString*)path
{
    NSString *filePath = path;
    [[NSFileManager defaultManager] createDirectoryAtPath:filePath withIntermediateDirectories:YES attributes:nil error:nil];
    NSString *fileName = [NSString stringWithFormat:@"%@.json", name];
    NSString *fileAtPath = [filePath stringByAppendingPathComponent:@"/"];
    fileAtPath = [fileAtPath stringByAppendingPathComponent:fileName];
    if (![[NSFileManager defaultManager] fileExistsAtPath:fileAtPath]) {
        [[NSFileManager defaultManager] createFileAtPath:fileAtPath contents:nil attributes:nil];
    }
    [[[NSString stringWithUTF8String:jsonString.c_str()] dataUsingEncoding:NSUTF8StringEncoding] writeToFile:fileAtPath atomically:NO];
}

- (void)setupEnv:(tfw::Descriptor *)des
{
    dataPrefix = [NSString stringWithUTF8String:des->dataPrefix().c_str()];

    NSString *readDataPath = [[AppDelegate getDataPath] stringByAppendingPathComponent:dataPrefix];
    if (readDataPath != nil) {
        des->env().setReadPath([readDataPath UTF8String]);
    } else {
        NGLOG_WARN("Failed to determine 'data' directory");
    }
    NSString* savePath = [[NSFileManager defaultManager] applicationSupportDirectory];
    savePath = [savePath stringByAppendingString:@"/results/"];
    [[NSFileManager defaultManager] createDirectoryAtPath:savePath withIntermediateDirectories:YES attributes:nil error:nil];
    des->env().setWritePath([[NSString stringWithFormat:@"%@", savePath] UTF8String]);
}

@end
