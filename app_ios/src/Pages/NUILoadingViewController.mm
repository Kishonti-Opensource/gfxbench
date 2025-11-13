/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUILoadingViewController.h"
#import "Utils/NUIMessageKeys.h"
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"
#import "Controls/NUILoadingBar.h"
#import "Utils/NUIUtilities.h"

@interface NUILoadingViewController ()

@property (assign, nonatomic) BOOL done;
@property (strong, nonatomic) NSMutableArray *alertViews;

@end

@implementation NUILoadingViewController

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view.

    self.done = false;

    self.alertViews = [[NSMutableArray alloc] init];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleAlertRequest:) name:NUI_Request_ShowModalMessage object:nil];

    self.ActivityLabel.text = @"";
    self.BackView.alpha = 0;
    self.LogoView.alpha = 1;

    self.ActivityLabel.font = [[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad ? [THTheme getFontNamed:@"FontPadLoadingBar"] : [THTheme getFontNamed:@"FontPhoneLoadingBar"];
    self.ActivityLabel.textColor = [THTheme getColorNamed:@"TextMsgBoxColor"];

    [self.progressbar setBackgroundColor:[THTheme getColorNamed:@"LoadingBarBackColor"]];
    [self.progressbar setForegroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.progressbar setIsRounded:true];

    [self infiniteLoadStarted:nil];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(loadingChanged:) name:NUI_Notification_LoadingChanged object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(infiniteLoadStarted:) name:NUI_Notification_LoadingForeverStarted object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(infiniteLoadStopped) name:NUI_Notification_LoadingForeverStopped object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(fadeOut) name:NUI_Request_HideLoading object:nil];



    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(loadingChanged:) name:NUI_Request_UpdateInitMessage object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(loadingChanged:) name:NUI_Request_UpdateSyncProgress object:nil];


    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(showMessage:) name:NUI_Request_ShowModalMessage object:nil];
}


- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)loadingChanged:(NSNotification *)notification
{
    if(notification.userInfo != nil)
    {
        if([notification.userInfo objectForKey:@"Status"] != nil)
        {
            NSString *status = notification.userInfo[@"Status"];
            self.ActivityLabel.text = status;
        }
        if([notification.userInfo objectForKey:@"Progress"] != nil)
        {
            NSNumber *percent = notification.userInfo[@"Progress"];
            [self.progressbar setProgress:[percent floatValue]];
        }


        if([notification.userInfo objectForKey:@"message"] != nil)
        {
            dispatch_async(dispatch_get_main_queue(),
                             ^{
                                NSString *status = [NUIAppData getLocalized:notification.userInfo[@"message"]];
                                self.ActivityLabel.text = status;
                             });
        }

        if([notification.userInfo objectForKey:@"progress"] != nil)
        {
            NSNumber *percent = notification.userInfo[@"progress"];
            NSNumber *bytesNeeded = notification.userInfo[@"bytesNeeded"];
            NSNumber *bytesWritten = notification.userInfo[@"bytesWritten"];

            NSString *percentString = [NSString stringWithFormat:@"%.2f", [percent floatValue] * 100];
            NSString *bytesNeededString = [NSString stringWithFormat:@"%.2f", [bytesNeeded longLongValue] /1024.0f /1024.0f];
            NSString *bytesWrittenString = [NSString stringWithFormat:@"%.2f", [bytesWritten longLongValue] /1024.0f /1024.0f];

            [self.progressbar setProgress:[percent floatValue]];

            self.ActivityLabel.text = [NSString stringWithFormat:[NUIAppData getLocalized:@"SyncronizingLoadingString"], [percentString UTF8String], [bytesWrittenString UTF8String], [bytesNeededString UTF8String]];
        }
    }
}

- (void)infiniteLoadStarted:(NSNotification *)notification
{
    [self.progressbar setIsInfinite:true];
}

- (void)infiniteLoadStopped
{
    [self.progressbar setIsInfinite:false];
}

- (void)showMessage:(NSNotification *)notification {
    if([self presentedViewController] != nil) return;

    UIAlertController *alert = [[NUIAppData sharedNUIAppData] getAlert];
    if(alert != nil)
        [self presentViewController:alert animated:YES completion:nil];
}


- (void)fadeOut
{
    self.done = true;

    if(self.presentedViewController == nil)
        [self performSegueWithIdentifier:@"ShowMainViewSegue" sender:self];
}



#pragma mark - alerts

- (void) handleAlertRequest:(NSNotification *)notification {
    if(notification.userInfo == nil) {
        NSLog(@"Missing userinfo on modalmessagerequest.");
        return;
    }

    if([notification.userInfo objectForKey:@"messageObj"] == nil) {
        NSLog(@"Missing messageObject on modalmessagerequest.");
        return;
    }

    APPModalMessageObject *mObject = notification.userInfo[@"messageObj"];

    UIAlertController *alert = [UIAlertController alertControllerWithTitle:[NUIAppData getLocalized:mObject.title]
                                                                   message:[NUIAppData getLocalized:mObject.message]
                                                            preferredStyle:UIAlertControllerStyleAlert];


    if(mObject.choices.count == 0) {
        [alert addAction:[UIAlertAction actionWithTitle:[NUIAppData getLocalized:@"Ok"] style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            if(mObject.isFatal) {
                [self.alertViews addObject:alert];
            }
            [self popAlert];
        }]];

    } else {
        for (APPModalMessageAction *mAction in mObject.choices) {
            [alert addAction:[UIAlertAction actionWithTitle:[NUIAppData getLocalized:mAction.title] style:mAction.style handler:^(UIAlertAction * _Nonnull action) {
                if(mAction.handler != nil) {
                    mAction.handler(mAction);
                }
                if(mObject.isFatal) {
                    [self.alertViews addObject:alert];
                }
                [self popAlert];
            }]];
        }
    }

    [self.alertViews addObject:alert];
    [self popAlert];
}

- (void) popAlert {
    UIAlertController *alert = [self.alertViews firstObject];
    if(self.presentedViewController == nil && alert != nil) {
        [self.alertViews removeObject:alert];
        [self presentViewController:alert animated:YES completion:nil];

    } else if (self.presentedViewController == nil && self.done) {
        [self fadeOut];
    }
}

@end
