/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDeviceinfoViewController.m
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import "APPDeviceinfoViewController.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIUtilities.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUICursorDataSource.h"
#import "Pages/APPDeviceDetailPage.h"
#import "Cells/APPDeviceinfoCell.h"

typedef enum appDeviceinfoState
{
    APPDEVICEINFO_INFO,
    APPDEVICEINFO_DETAIL
} AppDeviceinfoState;

@interface APPDeviceinfoViewController()

@property (strong, nonatomic) NUICursorDataSource *infoDataSource;
@property (strong, nonatomic) NUICursorDataSource *detailDataSource;
@property (strong, nonatomic) APPPageNavigator *navigator;
@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@property (copy, nonatomic) NSString *navigatorTitle;
@property (assign, nonatomic) AppDeviceinfoState state;

@end

@implementation APPDeviceinfoViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    
    self.navigatorTitle = @"TabInfo";
    self.state = APPDEVICEINFO_INFO;
    self.deviceinfoTable.alpha = 1;
    
    [self setupInfoTable];
    [self setupNavigator];
    
    [self applyTheme];
    [self applyLocalization];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Page Setup

- (void)setupInfoTable {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getSystemInfo()];
    self.infoDataSource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                                  Table:self.deviceinfoTable
                                                               CellType:[APPDeviceinfoCell class]
                                                             HeaderType:[NSNull class]];
    
    
    self.deviceinfoTable.dataSource = self.infoDataSource;
    self.deviceinfoTable.delegate = self.infoDataSource;
    self.infoDataSource.delegate = self;
}

- (void)setupNavigator {
    self.navigator = [APPPageNavigator addNavigatorIn:self.navigationHolder];
    self.navigator.delegate = self;
}


#pragma mark - Base Page Overrides

- (void) applyLocalization {
    if(self.navigator != nil) {
        self.navigator.title.text = [NUIAppData getLocalized:self.navigatorTitle];
        
        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone &&
           self.state == APPDEVICEINFO_DETAIL) {
            [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@"Back"] forState:UIControlStateNormal];
            [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.rightBtn setUserInteractionEnabled:NO];
            [self.navigator.leftBtn setUserInteractionEnabled:YES];
            
        } else {
            [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.leftBtn setUserInteractionEnabled:NO];
            [self.navigator.rightBtn setUserInteractionEnabled:NO];
        }
        
        self.navigator.subtitle.text = [NUIUtilities getVersionString];
    }
}

- (void) applyTheme {
    self.view.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.deviceinfoTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.deviceinfoTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    
    if(self.navigator != nil) {
        [self.navigator setBackgroundColor:[THTheme getColorNamed:@"NavBarColor"]];
        [self.navigator.title setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self.navigator.subtitle setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
        [self.navigator.leftBtn setTitleColor:[THTheme getColorNamed:@"TextHeaderColor"] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitleColor:[THTheme getColorNamed:@"TextHeaderColor"] forState:UIControlStateNormal];
        
        [self.navigator.title setFont:[THTheme getFontNamed:@"TitleSFont"]];
        [self.navigator.subtitle setFont:[THTheme getFontNamed:@"TextSFont"]];
        [self.navigator.leftBtn.titleLabel setFont:[THTheme getFontNamed:@"TextSFont"]];
        [self.navigator.rightBtn.titleLabel setFont:[THTheme getFontNamed:@"TextSFont"]];
    }
}



#pragma mark - User Interaction

- (void)navigationLeftPressed {
}


#pragma mark - CellCursorDelegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path {
    
    if([NUIUtilities shouldGetGeekyDetails:[[cursor getStringInColumn:@"title"] lowercaseString]]) {
        BOOL needsCloseBtn = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone;
        UIView *root = [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone ?
        self.parentViewController.parentViewController.view :
        self.page;
        
        [APPDeviceDetailPage addDeviceDetailPageIn:root forRowId:[cursor getIntegerInColumn:@"_id"] hasCloseBtn:needsCloseBtn];
    }
    
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
    [self.deviceinfoTable reloadData];
}

@end
