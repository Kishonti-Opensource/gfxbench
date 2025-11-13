/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "AppDelegate.h"
#import "ViewController.h"
#import "deviceinfo/glinfo.h"
#import "ng/log.h"

@interface AppDelegate ()


@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    self.window.backgroundColor = [UIColor whiteColor];

    NSString *storyboardName = @"MainStoryboard_iPhone";
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
    {
        storyboardName = @"MainStoryboard_iPad";
    }
    UIStoryboard *storyboard = [UIStoryboard storyboardWithName:storyboardName bundle:nil];
    ViewController *viewController = [storyboard instantiateViewControllerWithIdentifier:@"Main"];

    self.window.rootViewController = viewController;

    [self.window makeKeyAndVisible];

    tfw::GLInfoCollector glc;
    glc.collect();
    NGLOG_INFO("gles: %s", glc.serializeGLES());
    return YES;
}

+ (NSString *) getDirecotryPath:(NSString *) dir withWriteAccess:(BOOL) writeable
{
	NSString *documentsDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
	BOOL isDir = NO;
	NSString *documentsDirPath = [documentsDir stringByAppendingPathComponent:dir];
	[[NSFileManager defaultManager] fileExistsAtPath: documentsDirPath isDirectory:&isDir];
	if (isDir) {
		return documentsDirPath;
	}
	if (writeable) {
		return nil;
	}

	NSString *bundleDir = [[NSBundle mainBundle] bundlePath];
	NSString *bundleDirPath = [bundleDir stringByAppendingPathComponent:dir];
	[[NSFileManager defaultManager] fileExistsAtPath: bundleDirPath isDirectory:&isDir];
	if (isDir) {
		return bundleDirPath;
	}

	return nil;
}

+ (NSString *) getDataPath
{
	return [AppDelegate getDirecotryPath:@"data" withWriteAccess:NO];
}

+ (NSString *) getConfigPath
{
	return [AppDelegate getDirecotryPath:@"config" withWriteAccess:NO];
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
