/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUIAppDelegate.h"
#import "Common/NUIAppData.h"
#import "NUIMainViewController.h"
#import "Utils/NUIMessageKeys.h"
#import "Pages/NUITestViewController.h"
#import "NUICommandLineParser.h"

@interface NUIAppDelegate()

@property (strong, nonatomic) NUICommandLineParser *cmdline;

@end

@implementation NUIAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    try
    {
        self.cmdline = nil;

#if IS_CORPORATE
        if([[[NSProcessInfo processInfo] arguments] count] > 1) {
            [[self getCmdline] parse];
        }
#endif


        [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];

        _viewName = @"TheStoryboard";

        if(self.cmdline != nil)
        {
            self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
            UIStoryboard *storyboard = [UIStoryboard storyboardWithName:_viewName bundle:nil];
            NUITestViewController *viewController = [storyboard instantiateViewControllerWithIdentifier:@"NUITestViewController"];
            viewController.CommandLine = self.cmdline;
            self.window.rootViewController = (UIViewController *)viewController;
        }
        else
        {
            self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
            UIStoryboard *storyboard = [UIStoryboard storyboardWithName:_viewName bundle:nil];

            UIViewController *viewController = [storyboard instantiateViewControllerWithIdentifier:@"RootNavigatorVC"];

            self.window.rootViewController = viewController;
        }

        [self.window makeKeyAndVisible];

        if(self.cmdline == nil) {
            dispatch_queue_t startupQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, (unsigned long)NULL);
            dispatch_async(startupQueue, ^(void) {
                [[NUIAppData sharedNUIAppData] Startup];
            });
        }
    }
    catch(NSException *ex)
    {
        NSLog(@"-------- INIT ERROR ---------");
        NSLog(@"%@", ex.reason);
        NSLog(@"-----------------------------");
        exit(-1);
    }
    catch(...)
    {
        NSLog(@"-------- INIT ERROR ---------");
        exit(-1);
    }

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CancelTest object:self];
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:NO];

}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (NUICommandLineParser *)getCmdline
{
    if(self.cmdline == nil)
        self.cmdline = [[NUICommandLineParser alloc] init];
    return self.cmdline;
}


@end
