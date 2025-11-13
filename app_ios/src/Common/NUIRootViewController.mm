/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Common/NUIRootViewController.h"
#import "NUIDataViewController.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Common/NUIAppData.h"

@interface NUIRootViewController ()

    @property (strong, nonatomic) NUICustomTB *TabBar;
    @property (weak, nonatomic) IBOutlet UIView *TabBarContainer;
    @property (nonatomic, strong) IBOutlet UIScrollView *scrollView;
    @property (nonatomic, strong) NSMutableArray *viewControllers;
    @property (nonatomic, strong) NSMutableDictionary *viewControllersDictionary;

    @property (assign) BOOL ScrollerInteraction;
    @property (assign) BOOL TabbarInteraction;

@end

@implementation NUIRootViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(PageChangeRequest:) name:NUI_Request_PageChange object:nil];

    //self.contentList = @[@"home", @"result", @"compare", @"device", @"info"];
    self.contentList = @[@"home", @"results", @"compare", @"device", @"info"];
    NSUInteger numberPages = self.contentList.count;

    NSMutableArray *controllers = [[NSMutableArray alloc] init];
    for (NSUInteger i = 0; i < numberPages; i++)
    {
		[controllers addObject:[NSNull null]];
    }
    self.viewControllers = controllers;
    self.viewControllersDictionary = [[NSMutableDictionary alloc] init];

    // Theme settings
    self.scrollView.backgroundColor = [THTheme getColorNamed:@"BackColor"];
    self.TabBarContainer.backgroundColor = [THTheme getColorNamed:@"BackColor"];

    self.ScrollerInteraction = NO;
    self.TabbarInteraction = NO;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NUI_Request_PageChange object:nil];
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];

    [self.scrollView layoutIfNeeded];
    [self setupTabbar];

    CGFloat pageWidth = CGRectGetWidth(self.scrollView.frame);
    NSUInteger page = floor((self.scrollView.contentOffset.x - pageWidth / 2) / pageWidth) + 1;
    [self.TabBar selectBtn:page];

    // a page is the width of the scroll view
    self.scrollView.pagingEnabled = YES;
    self.scrollView.contentSize = CGSizeMake(CGRectGetWidth(self.scrollView.frame) * self.contentList.count, CGRectGetHeight(self.scrollView.frame));
    self.scrollView.showsHorizontalScrollIndicator = NO;
    self.scrollView.showsVerticalScrollIndicator = NO;
    self.scrollView.scrollsToTop = NO;
    self.scrollView.delegate = self;

    [self loadScrollViewWithPage:0];
    [self loadScrollViewWithPage:1];
    [self loadScrollViewWithPage:2];
    [self loadScrollViewWithPage:3];
    [self loadScrollViewWithPage:4];
}

- (void)setupTabbar {
    [self.TabBarContainer setLayoutMargins:UIEdgeInsetsZero];

    if(self.TabBar != nil) {
        [self.TabBar removeFromSuperview];
        self.TabBar = nil;
    }
    self.TabBar = [self loadCustomTB];
    [self.TabBar setup];
    [self.TabBarContainer addSubview:self.TabBar];
    self.TabBar.delegate = self;

    NSDictionary *viewKeys = @{@"tabbar":self.TabBar};
    [self.TabBarContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-[tabbar]-|" options:0 metrics:nil views:viewKeys]];
    [self.TabBarContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[tabbar]-|" options:0 metrics:nil views:viewKeys]];
}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NUICustomTB*)loadCustomTB
{
    NSArray* topLevelObjects;
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
    {
        topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CustomTabBarPad" owner:self options:Nil];
    }
    else
    {
        topLevelObjects = [[NSBundle mainBundle] loadNibNamed:@"CustomTabBar" owner:self options:Nil];
    }

    NUICustomTB* CustomTB = [topLevelObjects objectAtIndex:0];
    [CustomTB setTranslatesAutoresizingMaskIntoConstraints:NO];

    return CustomTB;
}


- (void)loadScrollViewWithPage:(NSUInteger)page
{
    if (page >= self.contentList.count)
    return;

    // replace the placeholder if necessary
    UIViewController *controller = [self.viewControllers objectAtIndex:page];
    if ((NSNull *)controller == [NSNull null])
    {
//        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
//        {
//            switch (page) {
//                    case 0:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIPadHomeViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 1:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIPadResultsViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 2:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIPadCompareViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 3:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIPadInfoViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 4:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUILoginViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                default:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIMainViewController" fromStoryboard:self.storyboard];
//                    break;
//            }
//        }
//        else
//        {
//            switch (page) {
//                case 0:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIHome2ViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 1:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIResults2ViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 2:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUICompareViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 3:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIInfoViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                case 4:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUILoginViewController" fromStoryboard:self.storyboard];
//                    break;
//
//                default:
//                    controller = [[NUIAppData getUIData] getViewControllerNamed:@"NUIMainViewController" fromStoryboard:self.storyboard];
//                    break;
//            }
//        }

        switch (page) {
            case 0:
                controller = [self getViewControllerNamed:@"APPHomeViewController" fromStoryboard:self.storyboard];
                break;

            case 1:
                controller = [self getViewControllerNamed:@"APPResultViewController" fromStoryboard:self.storyboard];
                break;

            case 2:
                controller = [self getViewControllerNamed:@"APPCompareViewController" fromStoryboard:self.storyboard];
                break;

            case 3:
                controller = [self getViewControllerNamed:@"APPDeviceinfoViewController" fromStoryboard:self.storyboard];
                break;

            case 4:
                controller = [self getViewControllerNamed:@"APPSettingsViewController" fromStoryboard:self.storyboard];
                break;

            default:
                controller = [self getViewControllerNamed:@"NUIMainViewController" fromStoryboard:self.storyboard];
                break;
        }
        [self.viewControllers replaceObjectAtIndex:page withObject:controller];
    }

    // add the controller's view to the scroll view
    if (controller.view.superview == nil)
    {
        if([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad)
        {
            CGRect frame = self.scrollView.frame;
            frame.origin.x = frame.size.width * page;
            frame.origin.y = 0;
            frame.size.width = frame.size.width;
            frame.size.height = frame.size.height;
            controller.view.frame = frame;
        }
        else
        {
            CGRect frame = self.scrollView.frame;
            frame.origin.x = CGRectGetWidth(frame) * page;
            frame.origin.y = 0;
            controller.view.frame = frame;
        }

        [self addChildViewController:controller];
        [self.scrollView addSubview:controller.view];
        [controller didMoveToParentViewController:self];
    }
}

// at the end of scroll animation, reset the boolean used when scrolls originate from the UIPageControl
- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    // switch the indicator when more than 50% of the previous/next page is visible
    CGFloat pageWidth = CGRectGetWidth(self.scrollView.frame);
    NSUInteger page = floor((self.scrollView.contentOffset.x - pageWidth / 2) / pageWidth) + 1;

    // load the visible page and the page on either side of it (to avoid flashes when the user starts scrolling)
//    [self loadScrollViewWithPage:page - 1];
//    [self loadScrollViewWithPage:page];
//    [self loadScrollViewWithPage:page + 1];

    // a possible optimization would be to unload the views+controllers which are no longer visible

    // set the tabbar's selected button
    [self.TabBar selectBtn:page];

    self.TabbarInteraction = NO;
    self.ScrollerInteraction = NO;
    self.scrollView.userInteractionEnabled = YES;
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView
{
    self.ScrollerInteraction = YES;
    CGFloat pageWidth = CGRectGetWidth(self.scrollView.frame);
    NSUInteger page = floor((self.scrollView.contentOffset.x - pageWidth / 2) / pageWidth) + 1;

    [self hideKeyboardOnPage:self.TabBar.SelectedItem];
    [self refreshPage:page];
    [self refreshPage:page+1];
    [self refreshPage:page-1];
}

- (void)hideKeyboardOnPage:(NSUInteger)page
{
    if (page >= self.contentList.count)
    return;

    UIViewController *controller = [self.viewControllers objectAtIndex:page];
    if ((NSNull *)controller != [NSNull null])
    {
        [controller.view endEditing:YES];
    }

//    switch (page) {
//            case 0:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"HomeBase"}];
//            break;
//
//            case 1:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"ResultBase"}];
//            break;
//
//            case 2:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"CompareBase"}];
//            break;
//
//            case 3:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"InfoBase"}];
//            break;
//
//            case 4:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"SettingsBase"}];
//            break;
//
//        default:
//            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_CloseKeyboard object:self userInfo:@{@"PageName" : @"HomeBase"}];
//            break;
//    }
}

- (void)refreshPage:(NSUInteger)page
{
    if (page >= self.contentList.count)
    return;


    switch (page) {
            case 0:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"HomeBase"}];
            break;

            case 1:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"ResultBase"}];
            break;

            case 2:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"CompareBase"}];
            break;

            case 3:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"InfoBase"}];
            break;

            case 4:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"SettingsBase"}];
            break;

        default:
            [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_RefreshPage object:self userInfo:@{@"PageName" : @"HomeBase"}];
            break;
    }
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    if(!self.TabbarInteraction && self.ScrollerInteraction)
    {
        CGFloat pageWidth = CGRectGetWidth(self.scrollView.frame);
        NSUInteger page = floor((self.scrollView.contentOffset.x - pageWidth / 2) / pageWidth) + 1;
        [self.TabBar selectBtn:page];
    }
}

- (void)Tabbar:(NUICustomTB *)viewController didChooseTab:(NSInteger)index
{
    if(!self.TabbarInteraction && !self.ScrollerInteraction)
    {
        self.TabbarInteraction = YES;
        self.scrollView.userInteractionEnabled = NO;
        [self gotoPage:YES];
    }
}

- (void)gotoPage:(BOOL)animated
{
    NSInteger page = self.TabBar.SelectedItem;

    // load the visible page and the page on either side of it (to avoid flashes when the user starts scrolling)
    [self loadScrollViewWithPage:page - 1];
    [self loadScrollViewWithPage:page];
    [self loadScrollViewWithPage:page + 1];

//    [self refreshPage:page];
//    [self refreshPage:page+1];
//    [self refreshPage:page-1];

	// update the scroll view to the appropriate page
    CGRect bounds = self.scrollView.bounds;
    bounds.origin.x = CGRectGetWidth(bounds) * page;
    bounds.origin.y = 0;
    [self.scrollView scrollRectToVisible:bounds animated:animated];
    [self.TabBar selectBtn:page];

    self.TabbarInteraction = NO;
    self.ScrollerInteraction = NO;
    self.scrollView.userInteractionEnabled = YES;
}

- (void)PageChangeRequest:(NSNotification *)notification
{
    if(!self.TabbarInteraction && !self.ScrollerInteraction)
    {
        [self hideKeyboardOnPage:self.TabBar.SelectedItem];
        if(notification.userInfo != nil)
        {
            if([notification.userInfo objectForKey:@"ToPageIndex"] != nil)
            {
                self.TabBar.SelectedItem = [notification.userInfo[@"ToPageIndex"] integerValue];
                self.ScrollerInteraction = YES;
                [self gotoPage:YES];
            }
        }
    }
}

- (UIViewController *) getViewControllerNamed:(NSString *)name fromStoryboard:(UIStoryboard *)storyboard
{
    UIViewController *vc = [self.viewControllersDictionary objectForKey:name];
    if(vc == nil)
    {
        vc = [storyboard instantiateViewControllerWithIdentifier:name];
        [self.viewControllersDictionary setObject:vc forKey:name];
    }

    return vc;
}


@end
