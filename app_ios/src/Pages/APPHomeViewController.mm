/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPHomeViewController.m
//  app_ios
//
//  Created by Balazs Hajagos on 26/08/2015.
//
//

#import "APPHomeViewController.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUIUtilities.h"
#import "Utils/NUICursorDataSource.h"
#import "Cells/APPTestCell.h"
#import "Cells/APPHeaderCell.h"
#import "APPPageNavigator.h"


typedef enum appHomeState
{
    HOMEPAGE_START,
    HOMEPAGE_TESTSELECT
} AppHomeState;


@interface APPHomeViewController ()

@property (strong, nonatomic) NUICursorDataSource *datasource;
@property (strong, nonatomic) APPPageNavigator *navigator;

@property (assign, nonatomic) AppHomeState state;

@end

@implementation APPHomeViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    
    self.state = HOMEPAGE_START;
    
    [self.mainControl setHandler:self];
    
    [self setupTable];
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

- (void)setupTable {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getTests()];
    self.datasource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                            Table:self.testTable
                                                         CellType:[APPTestCell class]
                                                       HeaderType:[APPHeaderCell class]];
    self.datasource.delegate = self;
    
    self.testTable.dataSource = self.datasource;
    self.testTable.delegate = self.datasource;
}

- (void)setupNavigator {
    self.navigator = [APPPageNavigator addNavigatorIn:self.navigatorHolder];
    self.navigator.delegate = self;
}


#pragma mark - Base Page Overrides

- (void) applyLocalization {
    [self.mainControl setMainText:[NUIAppData getLocalized:@"StartAll"]];
    [self.mainControl setSideText:[NUIAppData getLocalized:@"TestSelection"]];
    
    if(self.navigator != nil) {
        self.navigator.title.text = [NUIAppData getLocalized:@"TestSelection"];
        self.navigator.subtitle.text = [NUIUtilities getVersionString];
        [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@"Back"] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@"Start"] forState:UIControlStateNormal];
        [self.navigator.leftBtn setUserInteractionEnabled:YES];
        [self.navigator.rightBtn setUserInteractionEnabled:YES];
    }
}

- (void) applyTheme {
    self.view.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.testTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.testTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    [self.mainControl setMainTextColor:[THTheme getColorNamed:@"TextStartBtnColor"]];
    [self.mainControl setSideTextColor:[THTheme getColorNamed:@"TextTestSelectBtnColor"]];
    
    [self.mainControl setBackColor:[THTheme getColorNamed:@"SeparatorColor"]];
    [self.mainControl setMainColor:[THTheme getColorNamed:@"MainColor"]];
    [self.mainControl setSideColor:[THTheme getColorNamed:@"TabBarColor"]];
    
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

- (IBAction)startAllPressed:(id)sender {
    [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_TestStyle object:self];
    [NUIAppData getService]->runAllTests();
}

- (IBAction)testSelectPressed:(id)sender {
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        if(self.state == HOMEPAGE_START) {
            [self.view layoutIfNeeded];
            [self.startToFullConstraintBase setActive:NO];
            [self.VerticalToStartConstraintBase setActive:NO];
            [UIView animateWithDuration:0.2 animations:^{
                [self.view layoutIfNeeded];
                
            } completion:^ (BOOL finished){
                if (finished) {
                    self.state = HOMEPAGE_TESTSELECT;
                }
            }];
            
        } else {
            [self navigationLeftPressed];
        }
        
    } else {
        [self.mainControl layoutIfNeeded];
        [UIView animateWithDuration:0.2 animations:^{
            self.startView.alpha = 0.0;
        } completion:^ (BOOL finished){
            if (finished) {
                self.state = HOMEPAGE_TESTSELECT;
                [self.view sendSubviewToBack:self.startView];
            }
        }];
    }
}

- (void)navigationLeftPressed {
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.view layoutIfNeeded];
        [self.startToFullConstraintBase setActive:YES];
        [self.VerticalToStartConstraintBase setActive:YES];
        [UIView animateWithDuration:0.2 animations:^{
            [self.view layoutIfNeeded];
            
        } completion:^ (BOOL finished){
            if (finished) {
                self.state = HOMEPAGE_START;
            }
        }];
        
    } else {
        [self.view bringSubviewToFront:self.startView];
        [UIView animateWithDuration:0.2 animations:^{
            self.startView.alpha = 1.0;
        } completion:^ (BOOL finished){
            if (finished) {
                self.state = HOMEPAGE_START;
            }
        }];
    }
}

- (void)navigationRightPressed {
    if(self.state == HOMEPAGE_TESTSELECT) {
        [[NSNotificationCenter defaultCenter] postNotificationName:NUI_Request_TestStyle object:self];
        [NUIAppData getService]->runSelectedTests();
    }
}


#pragma mark - NUICursorDataSourceDelegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path {
    [NUIAppData getService]->setTestSelection([[cursor getStringInColumn:@"title"] UTF8String], ![cursor getBooleanInColumn:@"isChecked"]);
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
    [self.testTable reloadData];
}


#pragma mark - Main Control Handler
- (void)handleMainPressedForMainControl:(APPMainControl *)control {
    [self startAllPressed:self];
}

- (void)handleSidePressedForMainControl:(APPMainControl *)control {
    [self testSelectPressed:self];
}


@end
