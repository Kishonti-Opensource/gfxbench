/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultViewController.m
//  app_ios
//
//  Created by Balazs Hajagos on 31/08/2015.
//
//

#import "APPResultViewController.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUICursorDataSource.h"
#import "Utils/NUIUtilities.h"
#import "APPResultDetailPage.h"
#import "Cells/APPResultCell.h"
#import "Cells/APPHeaderCell.h"
#import "Cells/APPHistoryCell.h"
#import "resultitem.h"

typedef enum appResultState
{
    RESULTPAGE_DETAIL,
    RESULTPAGE_SESSIONS
} AppResultState;


@interface APPResultViewController ()

@property (strong, nonatomic) NUICursorDataSource *resultDatasource;
@property (strong, nonatomic) NUICursorDataSource *historyDatasource;
@property (strong, nonatomic) APPPageNavigator *navigator;

@property (assign, nonatomic) AppResultState state;
@property (copy, nonatomic) NSString *navigatorTitle;

@end

@implementation APPResultViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        self.navigatorTitle = @"BestResults";
        self.state = RESULTPAGE_DETAIL;
        self.historyTable.alpha = 0;
        
    } else {
        self.navigatorTitle = @"TabResults";
        self.state = RESULTPAGE_SESSIONS;
        self.historyTable.alpha = 1;
    }
    
    
    [self setupResultTable];
    [self setupHistoryTable];
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

- (void)setupResultTable {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getBestResults()];
    self.resultDatasource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                                  Table:self.resultTable
                                                               CellType:[APPResultCell class]
                                                             HeaderType:[APPHeaderCell class]
                                                     PermanentSelection:false
                                                           AnimatedRows:true];
    
    self.resultTable.dataSource = self.resultDatasource;
    self.resultTable.delegate = self.resultDatasource;
    self.resultDatasource.delegate = self;
}

- (void)setupHistoryTable {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getSessions()];
    self.historyDatasource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                                   Table:self.historyTable
                                                                CellType:[APPHistoryCell class]
                                                              HeaderType:[NSNull class]
                                                      PermanentSelection:true];
    [self.historyDatasource setCellHeight:44];
    
    self.historyTable.dataSource = self.historyDatasource;
    self.historyTable.delegate = self.historyDatasource;
    self.historyDatasource.delegate = self;
}

- (void)setupNavigator {
    self.navigator = [APPPageNavigator addNavigatorIn:self.navigatorHolder];
    self.navigator.delegate = self;
}

- (void)loadLatestIfNeeded {
    if([NUIAppData NeedsLatestResults]) {
        [self.historyDatasource.cursor moveToPosition:1];
        [self cursorDataSource:self.historyDatasource handleCursorSelected:self.historyDatasource.cursor atIndexPath:[NSIndexPath indexPathForRow:1 inSection:0]];
        [NUIAppData SetNeedsLatestResults:NO];
    }
}


#pragma mark - Base Page Overrides

- (void) applyLocalization {
    if(self.navigator != nil) {
        self.navigator.title.text = [NUIAppData getLocalized:self.navigatorTitle != nil ? self.navigatorTitle : @""];
        
        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone &&
           self.state == RESULTPAGE_DETAIL) {
            [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
            [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@"ResultHistory"] forState:UIControlStateNormal];
            [self.navigator.rightBtn setUserInteractionEnabled:YES];
            [self.navigator.leftBtn setUserInteractionEnabled:NO];
            
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
    self.resultTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.resultTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    self.historyTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.historyTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    
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

- (void)navigationRightPressed {
    if(self.state == RESULTPAGE_DETAIL) {
        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
            [self transitionToSessions];
        }
    }
}

- (void)transitionToSessions {
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        self.navigatorTitle = @"ResultHistory";
        self.navigator.title.text = [NUIAppData getLocalized:self.navigatorTitle];
        [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
        [self.navigator.leftBtn setUserInteractionEnabled:NO];
        [self.navigator.rightBtn setUserInteractionEnabled:NO];
        
        [self.tablesHolder bringSubviewToFront:self.historyTable];
        [UIView animateWithDuration:0.2 animations:^{
            self.historyTable.alpha = 1.0;
            
        } completion:^ (BOOL finished) {
            if(finished) {
                self.state = RESULTPAGE_SESSIONS;
            }
        }];
    }
}

- (void)transitionToDetail {
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        self.navigator.title.text = [NUIAppData getLocalized:self.navigatorTitle];
        [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@"ResultHistory"] forState:UIControlStateNormal];
        [self.navigator.rightBtn setUserInteractionEnabled:YES];
        [self.navigator.leftBtn setUserInteractionEnabled:NO];
        
        [UIView animateWithDuration:0.2 animations:^{
            self.historyTable.alpha = 0.0;
            
        } completion:^ (BOOL finished) {
            if(finished) {
                [self.tablesHolder sendSubviewToBack:self.historyTable];
                self.state = RESULTPAGE_DETAIL;
            }
        }];
    }
}


#pragma mark - CellCursorDelegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path {
    if(dataSource == self.historyDatasource) {
        self.navigatorTitle = [cursor getStringInColumn:@"title"];
        
        NUICursor *newCursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getResultForSession([cursor getIntegerInColumn:@"_id"])];
        self.resultDatasource = [[NUICursorDataSource alloc] initWithCursor:newCursor
                                                                      Table:self.resultTable
                                                                   CellType:[APPResultCell class]
                                                                 HeaderType:[APPHeaderCell class]
                                                         PermanentSelection:false
                                                               AnimatedRows:true];
        
        self.resultTable.dataSource = self.resultDatasource;
        self.resultTable.delegate = self.resultDatasource;
        self.resultDatasource.delegate = self;
        [self.resultTable reloadData];
        
        [self transitionToDetail];
    }
    
    if(dataSource == self.resultDatasource) {
        NSInteger index = [self.resultDatasource cursorIndexFromIndexPath:path];
        [cursor moveToPosition:index];
        NSInteger status = [cursor getIntegerInColumn:@"status"];
        if(status == BenchmarkService::OK) {
            if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
                [APPResultDetailPage addResultDetailPageIn:self.parentViewController.parentViewController.view forRowId:[cursor getIntegerInColumn:@"_id"] haseCloseBtn:YES];
            } else {
                [APPResultDetailPage addResultDetailPageIn:self.tablesHolder forRowId:[cursor getIntegerInColumn:@"_id"]];
            }
        }
    }
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
    [self.resultTable reloadData];
    [self.historyTable reloadData];
}

- (void)handleShowResults {
    [self setupHistoryTable];
    [self.historyDatasource.cursor moveToPosition:1];
    
    NUICursor *newCursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getResultForSession([self.historyDatasource.cursor getIntegerInColumn:@"_id"])];
    self.resultDatasource = [[NUICursorDataSource alloc] initWithCursor:newCursor
                                                                  Table:self.resultTable
                                                               CellType:[APPResultCell class]
                                                             HeaderType:[APPHeaderCell class]
                                                     PermanentSelection:false
                                                           AnimatedRows:true];
    
    self.resultTable.dataSource = self.resultDatasource;
    self.resultTable.delegate = self.resultDatasource;
    self.resultDatasource.delegate = self;
    [self.resultTable reloadData];
    
    [self transitionToDetail];
}


#pragma mark - NUICursor delegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetChangedFrom:(NSInteger)from to:(NSInteger)to {
    if(dataSource == self.historyDatasource) {
        [self loadLatestIfNeeded];
    }
}

- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetInvalidated:(NUICursor *)cursor {
    if(dataSource == self.historyDatasource) {
        [self loadLatestIfNeeded];
    }
}

@end
