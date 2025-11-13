/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "NUIMainViewController.h"
#import "Common/THTheme.h"
#import "Controls/NUICustomTB.h"
#import "Common/NUIRootViewController.h"
#import "Pages/NUITestViewController.h"
#import "Utils/NUIMessageKeys.h"

#import "Common/NUIAppData.h"

@interface NUIMainViewController ()

@property (strong, nonatomic) NUIRootViewController *modelController;
@property (strong, nonatomic) NUITestViewController *testController;
//@property (strong, nonatomic) NUITestInfoViewController *testInfoVC;
@property (strong, nonatomic) NUILoadingViewController *loadingVC;
@property (strong, nonatomic) NUITestSelectorViewController *testSelectorVC;
@property (strong, nonatomic) NUIBatteryDiagramViewController *batteryDiagramVC;

@property (strong, nonatomic) NSMutableDictionary *viewDict;
@property (strong, nonatomic) UIViewController *currViewController;

@property (strong, nonatomic) NSMutableArray *alertViews;

@end

@implementation NUIMainViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {

    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Hide status bar.
    if ([self respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)]) {
        // iOS 7
        [self prefersStatusBarHidden];
        [self performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];
    }

    self.alertViews = [[NSMutableArray alloc] init];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ToTestPage:) name:NUI_Request_TestStyle object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ToUIPage:) name:NUI_Request_UIStyle object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ShowNonTabView:) name:NUI_Request_ShowNonTabView object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(CloseNonTabView:) name:NUI_Request_CloseNonTabView object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleAlertRequest:) name:NUI_Request_ShowModalMessage object:nil];
}


- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    CGRect MainViewFrame;
    CGRect FullViewFrame;
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
    {
        MainViewFrame = CGRectMake( 0, 0, self.view.frame.size.width, self.view.frame.size.height );
        FullViewFrame = CGRectMake( 0, 0, self.view.frame.size.height, self.view.frame.size.width );
    }
    else
    {
        MainViewFrame = CGRectMake( 0, 0, self.view.frame.size.width, self.view.frame.size.height );
        FullViewFrame = CGRectMake( 0, 0, self.view.frame.size.width, self.view.frame.size.height );
    }

    self.MainContainer.frame = MainViewFrame;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)ShowNonTabView:(NSNotification *)notification
{
    if(notification.userInfo != nil)
    {
        if([notification.userInfo objectForKey:@"PageName"] != nil)
        {
            if([notification.userInfo[@"PageName"] isEqualToString:@"LoadingPage"])
            {

                [self performSegueWithIdentifier:@"ToLoadingSegue" sender:self];
//                UIStoryboard *st = [UIStoryboard storyboardWithName:@"LaunchScreen" bundle:[NSBundle mainBundle]];
//
//                self.loadingVC = (NUILoadingViewController *)[[NUIAppData getUIData] getViewControllerNamed:@"NUILoadingViewController" fromStoryboard:st];
//                [self.FullViewport addSubview:self.loadingVC.view];
//                [self.loadingVC setFrameOnView:CGRectMake(0, 0, self.FullViewport.frame.size.width, self.FullViewport.frame.size.height)];
            }
//            if([notification.userInfo[@"PageName"] isEqualToString:@"TestInfoPage"])
//            {
//                self.testInfoVC = (NUITestInfoViewController *)[[NUIAppData sharedNUIAppData] getUIViewControllerNamed:@"NUITestInfoViewController" fromStoryboard:self.storyboard];
//                [self.FullViewport addSubview:self.testInfoVC.view];
//                [self.testInfoVC setFrameOnView:CGRectMake(0, 0, self.FullViewport.frame.size.width, self.FullViewport.frame.size.height)];
//            }
            if([notification.userInfo[@"PageName"] isEqualToString:@"BatteryDiagram"])
            {
//                self.batteryDiagramVC = [self.storyboard instantiateViewControllerWithIdentifier:@"NUIBatteryDiagramViewController"];
//                [self.FullViewport addSubview:self.batteryDiagramVC.view];
//                [self.batteryDiagramVC.view setFrame:CGRectMake(0, 0, self.FullViewport.frame.size.width, self.FullViewport.frame.size.height)];
            }
            if([notification.userInfo[@"PageName"] isEqualToString:@"MessageBox"])
            {
//                NUIMessageBoxInfo *mbInfo = notification.userInfo[@"MessageBoxInfo"];
//                self.messageBoxVC = [self.storyboard instantiateViewControllerWithIdentifier:@"NUIMessageBoxViewController"];
//                [self.messageBoxVC initWithInfo:mbInfo];
//                [self.FullViewport addSubview:self.messageBoxVC.view];
//                [self.messageBoxVC.view setFrame:CGRectMake(0, 0, self.FullViewport.frame.size.width, self.FullViewport.frame.size.height)];
            }
        }
    }
}


- (void)CloseNonTabView:(NSNotification *)notification
{
//    if(notification.userInfo != nil)
//    {
//        if([notification.userInfo objectForKey:@"PageName"] != nil)
//        {
//            if([notification.userInfo[@"PageName"] isEqualToString:@"LoadingPage"])
//            {
//                [self.loadingVC.view removeFromSuperview];
//                self.loadingVC = nil;
//            }
////            if([notification.userInfo[@"PageName"] isEqualToString:@"TestInfoPage"])
////            {
////                [self.testInfoVC.view removeFromSuperview];
////                self.testInfoVC = nil;
////            }
//            if([notification.userInfo[@"PageName"] isEqualToString:@"BatteryDiagram"])
//            {
//                [self.batteryDiagramVC.view removeFromSuperview];
//                self.batteryDiagramVC = nil;
//            }
//            if([notification.userInfo[@"PageName"] isEqualToString:@"MessageBox"])
//            {
//                [self.messageBoxVC.view removeFromSuperview];
//                self.messageBoxVC = nil;
//            }
//
//            [self.view setNeedsDisplay];
//            [self.view setNeedsLayout];
//        }
//    }

}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"RootViewControllerSegue"])
    {
        _modelController = [segue destinationViewController];
    }
}


- (BOOL)prefersStatusBarHidden
{
    return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden
{
    return NO;
}

- (UIViewController *)childViewControllerForHomeIndicatorAutoHidden
{
    if (_testController != nil)
    {
        return _testController;
    }

    return nil;
}


- (void)ToTestPage:(NSNotification *)notification
{
    if(_modelController != nil) {
        [_modelController removeFromParentViewController];
        [_modelController.view removeFromSuperview];
        _modelController = nil;
    }

    if(_testController == nil) {
        _testController = [self.storyboard instantiateViewControllerWithIdentifier:@"NUITestViewController"];
        _testController.testNames = notification.userInfo[@"SelectedTests"];

        _testController.view.frame = self.MainContainer.frame;
        [self.MainContainer addSubview:_testController.view];
        [self addChildViewController:_testController];
    }

    if (@available(iOS 11.0, *)) {
        [self setNeedsUpdateOfHomeIndicatorAutoHidden];
    }
}

- (void)ToUIPage:(NSNotification *)notification
{
    [_testController removeFromParentViewController];
    [_testController.view removeFromSuperview];
    _testController = nil;
    _modelController = [self.storyboard instantiateViewControllerWithIdentifier:@"NUIRootViewController"];
    _modelController.view.frame = self.MainContainer.frame;
    [self.MainContainer addSubview:_modelController.view];
    [self addChildViewController:_modelController];
    NSNotification *n = [[NSNotification alloc] initWithName:NUI_Request_PageChange object:self userInfo:@{@"ToPageIndex": @1}];
    [NUIAppData SetNeedsLatestResults:YES];
    [self.modelController performSelector:@selector(PageChangeRequest:) withObject:n afterDelay:0.05];

    if (@available(iOS 11.0, *)) {
        [self setNeedsUpdateOfHomeIndicatorAutoHidden];
    }
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
    }
}

@end
