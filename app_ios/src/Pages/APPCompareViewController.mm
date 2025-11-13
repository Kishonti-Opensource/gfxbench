/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPCompareViewContoller.m
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import "APPCompareViewController.h"
#import "APPDuelPage.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIUtilities.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Utils/NUICursorDataSource.h"
#import "Utils/NUIFadeCursorDataSource.h"
#import "Cells/APPSimpleTestCell.h"
#import "Cells/APPCompareCell.h"
#import "Cells/APPDuelCell.h"
#import "Cells/APPHeaderCell.h"

typedef enum appCompareState
{
    APPCOMPARE_COMPARE,
    APPCOMPARE_DUEL
} AppCompareState;


@interface APPCompareViewController ()

@property (strong, nonatomic) NUICursor *systemInfoCursor;
@property (strong, nonatomic) NUICursorDataSource *testListDataSource;
@property (strong, nonatomic) NUICursorDataSource *compareDataSource;
@property (strong, nonatomic) APPPageNavigator *navigator;
@property (strong, nonatomic) APPVerticalSelector *testSelector;
@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@property (assign, nonatomic) AppCompareState state;
@property (copy, nonatomic) NSString *userDeviceIcon;

@end

@implementation APPCompareViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleKeyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
    self.searchbar.delegate = self;
    
    self.state = APPCOMPARE_COMPARE;
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        self.testlistTable.alpha = 0;
        
    } else {
        self.testlistTable.alpha = 1;
    }
    
    [self.userTitle setText:[NSString stringWithFormat:@"%@",
                             [NUIAppData getLocalized:@"YourDevice"]]];
    [self.userSubtitle setText:@""];
    self.userMainScore.text = @"";
    self.userSecondaryScore.text = @"";
    
    self.systemInfoCursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getSystemInfo()];
    [self.systemInfoCursor setCallback:self];
    
    
    [self setupTestList];
    [self setupSelector];
    [self setupUserResult];
    [self setupCompareTable];
    [self setupNavigator];
    
    [self applyTheme];
    [self applyLocalization];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidAppear:(BOOL)animated {
    [self.compareTable reloadData];
}


#pragma mark - Page Setup

- (void)setupTestList {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getBestResults()];
    self.testListDataSource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                                    Table:self.testlistTable
                                                                 CellType:[APPSimpleTestCell class]
                                                               HeaderType:[APPHeaderCell class]
                                                       PermanentSelection:true];
    
    self.testlistTable.dataSource = self.testListDataSource;
    self.testlistTable.delegate = self.testListDataSource;
    self.testListDataSource.delegate = self;
}

- (void)setupCompareTable {
    if(self.testListDataSource.cursor.count > 0) {
        [self loadCompareDataWithTestId:[self.testListDataSource.cursor getStringInColumn:@"title"]];
    }
}

- (void)setupNavigator {
    self.navigator = [APPPageNavigator addNavigatorIn:self.navigatorHolder];
    self.navigator.delegate = self;
}

- (void)setupSelector {
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        if(self.testSelector == nil) {
            self.testSelector = [APPVerticalSelector addSelectorIn:self.testSelectorHolder];
            self.testSelector.delegate = self;
        }
        
        if(self.testListDataSource.cursor.count > 0) {
            [self.testSelector updateFromCursor:self.testListDataSource.cursor];
        }
    } else {
        [self.testSelectorHeightConstraint setConstant:0];
    }
}

- (void) setupUserResult {
    if(self.testListDataSource.cursor.count > 0) {
        [self setupMainScore:self.testListDataSource.cursor];
        [self setupSecondaryScore:self.testListDataSource.cursor];
        
        double progress = 0;
        if(self.compareDataSource.cursor.count > 0) {
            progress = [self.testListDataSource.cursor getDoubleInColumn:@"primaryScore"] / [self.compareDataSource.cursor getDoubleInColumn:@"maxScore"];
        }
        
        [self.view layoutIfNeeded];
        [self.userProgressConstraint setConstant:self.userResultHolder.frame.size.width * progress];
        [UIView animateWithDuration:0.2 animations:^{
            [self.view layoutIfNeeded];
            
        }];
    }
    
    if(self.systemInfoCursor.count > 0) {
        [self.systemInfoCursor moveToPosition:0];
        NSString *attribString = [self.systemInfoCursor getStringInColumn:@"attributesJson"];
        NSDictionary *attribs = [NSJSONSerialization JSONObjectWithData:[attribString dataUsingEncoding:NSUTF8StringEncoding] options:0 error:nil];
        if(attribs != nil) {
            if([attribs objectForKey:@"image"]) {
                NSString *imagePath = [NUIAppData SyncPath];
                imagePath = [imagePath stringByAppendingPathComponent:@"image/device"];
                imagePath = [imagePath stringByAppendingPathComponent:[attribs objectForKey:@"image"]];
                self.userDeviceIcon = imagePath;
                [self.userIcon setImage:[THTheme imageNamed:imagePath]];
                
            } else if([attribs objectForKey:@"image-alt"]) {
                NSString *imagePath = [NUIAppData SyncPath];
                imagePath = [imagePath stringByAppendingPathComponent:@"image/device"];
                imagePath = [imagePath stringByAppendingPathComponent:[attribs objectForKey:@"image-alt"]];
                self.userDeviceIcon = imagePath;
                [self.userIcon setImage:[THTheme imageNamed:imagePath]];
                
            } else {
                self.userDeviceIcon = @"device";
                [self.userIcon setImage:[THTheme imageNamed:@"device" withTintColor:[THTheme getColorNamed:@"MainColor"]]];
            }
        }
        
        [self.userTitle setText:[NSString stringWithFormat:@"%@ (%@)",
                                 [NUIAppData getLocalized:@"YourDevice"],
                                 [self.systemInfoCursor getStringInColumn:@"major"]]];
        
        [self.userSubtitle setText:[NUIAppData getLocalized:[NUIUtilities getProductBaseApi]]];
    } else {
        self.userDeviceIcon = @"device";
    }
}

- (void) setupMainScore:(NUICursor *)cursor {
    NSInteger status = [cursor getIntegerInColumn:@"status"];
    if(status == BenchmarkService::OK) {
        double score = [cursor getDoubleInColumn:@"primaryScore"];
        if(score <= 0) {
            [self.userMainScore setText:[NUIAppData getLocalized:@"Results_NA"]];
            
        } else {
            NSString *formattedScore = [NUIUtilities getFormattedResult:score];
            NSString *scoreString = [NSString stringWithFormat:@"%@ %@", formattedScore, [cursor getStringInColumn:@"primaryUnit"]];
            [self.userMainScore setText:scoreString];
        }
        
    } else if(status == BenchmarkService::CANCELLED) {
        [self.userMainScore setText:[NUIAppData getLocalized:@"STATUS_CANCELLED"]];
        
    } else if(status == BenchmarkService::FAILED) {
        [self.userMainScore setText:[NUIAppData getLocalized:@"STATUS_FAILED"]];
        
    } else if(status == BenchmarkService::NOT_AVAILABLE) {
        [self.userMainScore setText:[NUIAppData getLocalized:@"Results_NA"]];
    }
}

- (void) setupSecondaryScore:(NUICursor *)cursor {
    NSInteger status = [cursor getIntegerInColumn:@"status"];
    [self.userSecondaryScore setText:[NUIAppData getLocalized:@""]];
    [self.userSecondaryScoreHeightConstraint setActive:YES];
    
    if(status == tfw::Result::OK) {
        double score = [cursor getDoubleInColumn:@"secondaryScore"];
        if(score > 0) {
            NSString *formattedScore = [NUIUtilities getFormattedResult:score];
            NSString *scoreString = [NSString stringWithFormat:@"(%@ %@)", formattedScore, [cursor getStringInColumn:@"secondaryUnit"]];
            [self.userSecondaryScore setText:scoreString];
            [self.userSecondaryScoreHeightConstraint setActive:NO];
        }
        
    }
}


#pragma mark - Base Page Overrides

- (void) applyLocalization {
    if(self.navigator != nil) {
        
        self.navigator.title.text = [NUIAppData getLocalized:@"TabCompare"];
        [self.navigator.leftBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
        [self.navigator.rightBtn setTitle:[NUIAppData getLocalized:@""] forState:UIControlStateNormal];
        [self.navigator.rightBtn setUserInteractionEnabled:NO];
        [self.navigator.leftBtn setUserInteractionEnabled:NO];
        
        self.navigator.subtitle.text = [NUIUtilities getVersionString];
        self.userTitle.text = [NUIAppData getLocalized:@"YourBestScore"];
        self.userSubtitle.text = [NUIAppData getLocalized:@"TODO: Api and manufacturer here"];
        
        [self.searchbar setPlaceholder:[NUIAppData getLocalized:@"Search"]];
        
        if(self.testSelector != nil) {
            [self setupSelector];
        }
    }
}

- (void) applyTheme {
    self.view.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.pageBack.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.testlistTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.testlistTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    self.compareTable.backgroundColor = [THTheme getColorNamed:@"ListBackColor"];
    self.compareTable.separatorColor = [THTheme getColorNamed:@"SeparatorColor"];
    
    [self.userResultHolder setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.userTitle setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.userSubtitle setTextColor:[THTheme getColorNamed:@"TextNormalCellDescColor"]];
    [self.userTitle setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.userSubtitle setFont:[THTheme getFontNamed:@"TextSFont"]];
    [self.userMainScore setFont:[THTheme getFontNamed:@"FontCellScoreUpper"]];
    [self.userSecondaryScore setFont:[THTheme getFontNamed:@"FontCellScoreLower"]];
    [self.userMainScore setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
    [self.userSecondaryScore setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
    [self.userIcon setImage:[THTheme imageNamed:@"device" withTintColorName:@"MainColor"]];
    [self.userProgress setBackgroundColor:[THTheme getColorNamed:@"CompareHighlightValueBackColor"]];
    
    [self.userIcon setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    self.userIcon.layer.borderWidth = 1.0;
    self.userIcon.layer.borderColor = [[THTheme getColorNamed:@"SeparatorColor"] CGColor];
    [self.userIcon.layer setCornerRadius:4.0f];
    [self.userIcon.layer setMasksToBounds:YES];
    
    [self.compareBack setBackgroundColor:[THTheme getColorNamed:@"ListBackColor"]];
    [self.testSelectorHolder setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.searchHolder setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
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
    
    if(self.testSelector != nil) {
        [self.testSelector applyTheme];
    }
}



#pragma mark - User Interaction

- (void)navigationLeftPressed {
}

- (void)navigationRightPressed {
}

- (void)deviceSelected {
    UIView *root = self.pageBack;
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone) {
        root = self.parentViewController.parentViewController.view;
    }
    
    [APPDuelPage addDuelPageIn:root
                   deviceAName:[self.userTitle text]
               deviceASubtitle:[NUIUtilities getProductBaseApi]
                  deviceAImage:self.userDeviceIcon
                       forAApi:[NUIUtilities getProductBaseApi]
                   deviceBName:[self.compareDataSource.cursor getStringInColumn:@"title"]
               deviceBSubtitle:[self.compareDataSource.cursor getStringInColumn:@"api"]
                  deviceBImage:[self.compareDataSource.cursor getStringInColumn:@"icon"]
                       forBApi:[self.compareDataSource.cursor getStringInColumn:@"api"]
                   hasCloseBtn:[UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone];
}

- (void)loadCompareDataWithTestId:(NSString *)test_id {
    NUICursor *cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getCompareResults([test_id UTF8String], [self.searchbar.text UTF8String])];
    self.compareDataSource = [[NUICursorDataSource alloc] initWithCursor:cursor
                                                                   Table:self.compareTable
                                                                CellType:[APPCompareCell class]
                                                              HeaderType:[NSNull class]
                                                      PermanentSelection:false
                                                            AnimatedRows:true];
    [self.compareDataSource setCellHeight:70];
    
    self.compareTable.dataSource = self.compareDataSource;
    self.compareTable.delegate = self.compareDataSource;
    self.compareDataSource.delegate = self;
    
    [self setupUserResult];
}


#pragma mark - CellCursorDelegate
- (void)cursorDataSource:(NUICursorDataSource *)dataSource handleCursorSelected:(NUICursor *)cursor atIndexPath:(NSIndexPath *)path {
    if(dataSource == self.compareDataSource) {
        [self deviceSelected];
        
    } else if(dataSource == self.testListDataSource) {
        [self loadCompareDataWithTestId:[self.testListDataSource.cursor getStringInColumn:@"title"]];
        [self.testSelector updateFromCursor:self.testListDataSource.cursor];
        [self setupUserResult];
    }
}

- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetChangedFrom:(NSInteger)from to:(NSInteger)to {
    if(dataSource == self.testListDataSource) {
        [self.testListDataSource.cursor moveToPosition:0];
        [self setupSelector];
        [self setupUserResult];
        [self setupCompareTable];
        [self.compareTable reloadData];
        
    } else if(dataSource == self.compareDataSource) {
        [self setupUserResult];
        
    }
}

- (void)cursorDataSource:(NUICursorDataSource *)dataSource dataSetInvalidated:(NUICursor *)cursor {
    if(dataSource == self.testListDataSource) {
        [self.testListDataSource.cursor moveToPosition:0];
        [self setupSelector];
        [self setupUserResult];
        [self setupCompareTable];
        [self.compareTable reloadData];
        
    } else if(dataSource == self.compareDataSource) {
        [self setupUserResult];
        
    }
}


#pragma mark - Vertical Selector Delegate
- (void)leftBtnPressedForSelector:(APPVerticalSelector *)selector {
    [self.view endEditing:YES];
    if(self.testListDataSource.cursor.count > 0) {
        NSInteger currentIndex = [self.testListDataSource.cursor getPosition];
        NSInteger loopedIndex = ((currentIndex - 1) + self.testListDataSource.cursor.count) % self.testListDataSource.cursor.count;
        
        [self.testListDataSource.cursor moveToPosition:loopedIndex];
        [self loadCompareDataWithTestId:[self.testListDataSource.cursor getStringInColumn:@"title"]];
        [self.testSelector updateFromCursor:self.testListDataSource.cursor];
        [self setupUserResult];
    }
}

- (void)rightBtnPressedForSelector:(APPVerticalSelector *)selector {
    [self.view endEditing:YES];
    if(self.testListDataSource.cursor.count > 0) {
        NSInteger currentIndex = [self.testListDataSource.cursor getPosition];
        NSInteger loopedIndex = ((currentIndex + 1) + self.testListDataSource.cursor.count) % self.testListDataSource.cursor.count;
        
        [self.testListDataSource.cursor moveToPosition:loopedIndex];
        [self loadCompareDataWithTestId:[self.testListDataSource.cursor getStringInColumn:@"title"]];
        [self.testSelector updateFromCursor:self.testListDataSource.cursor];
        [self setupUserResult];
    }
}



#pragma mark - Searchbar Delegate
- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText {
    [self setupCompareTable];
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar {
    [self.searchbar resignFirstResponder];
}

- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar {
    [self.searchbar resignFirstResponder];
}

//- (void)searchBarTextDidEndEditing:(UISearchBar *)searchBar {
//    [self setupCompareTable];
//}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
    [self.testlistTable reloadData];
    [self.compareTable reloadData];
}

-(void) handleKeyboardWillShow:(NSNotification *) note {
    [self.view addGestureRecognizer:self.tapRecognizer];
    
    NSDictionary *info = [note userInfo];
    NSValue *kbFrame = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    CGRect keyboardFrame = [kbFrame CGRectValue];
    
    CGFloat height = keyboardFrame.size.height;
    
    [self respondToKeyboardHeight:height];
    
    [UIView animateWithDuration:MAX(animationDuration,0.1) animations:^{
        [self.view layoutIfNeeded];
    }];
}

-(void) handleKeyboardWillHide:(NSNotification *) note
{
    [self.view removeGestureRecognizer:self.tapRecognizer];
    
    NSDictionary *info = [note userInfo];
    NSTimeInterval animationDuration = [[info objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    
    [self respondToKeyboardHeight:0];
    [UIView animateWithDuration:MAX(animationDuration,0.1) animations:^{
        [self.view layoutIfNeeded];
    }];
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self.view endEditing:YES];
}

- (void) respondToKeyboardHeight:(CGFloat)height {
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    CGFloat diff = screenRect.size.height - self.view.frame.size.height;
    [self.searchholderBottomConstraint setConstant:MAX(height - diff,0)];
}


#pragma mark - NUICursorCallback

- (void) dataSetWillChangeFrom:(NSInteger)from to:(NSInteger)to {
    [self setupUserResult];
}

- (void) dataSetChangedFrom:(NSInteger)from to:(NSInteger)to {
    [self setupUserResult];
}

- (void) dataSetWillBeInvalidated {
    [self setupUserResult];
}

- (void) dataSetInvalidated {
    [self setupUserResult];
}

@end
