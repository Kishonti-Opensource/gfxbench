/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPRegisterPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 10/09/2015.
//
//

#import "APPResultDetailPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIMessageKeys.h"
#import "Charts/THLineChart.h"
#import "Cells/APPResultDetailChartCell.h"


@interface APPResultDetailPage ()

@property (assign, nonatomic) BOOL isCustomized;
@property (assign, nonatomic) int64_t resultRowId;
@property (strong, nonatomic) NUICursor *cursor;

@property (strong, nonatomic) UITapGestureRecognizer *tapRecognizer;

@end

@implementation APPResultDetailPage

+ (NSString *)detailCellName {
    return @"resultDetailTextCell";
}

+ (NSString *)chartCellName {
    return @"APPResultDetailChartCell";
}

- (instancetype) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if(self) {
        [self setup];
    }
    return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if(self) {
        [self setup];
    }
    return self;
}


#pragma mark - Page Setup
- (void) setup {
    self.isCustomized = NO;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleLocalizationChanged) name:NUI_Notification_LocalizationChanged object:nil];
    
    self.tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapAnywhere:)];
}

- (void) customize {
    self.isCustomized = true;
    
    self.table.dataSource = self;
    self.table.delegate = self;
    [self.table registerNib:[UINib nibWithNibName:@"APPResultDetailChartCell" bundle:nil] forCellReuseIdentifier:[APPResultDetailPage chartCellName]];
    
    [self.backView addGestureRecognizer:self.tapRecognizer];
    
    [self applyTheme];
    [self applyLocalization];
}

-(void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Base Page Overrides

- (void)layoutSubviews {
    [super layoutSubviews];
    if(!self.isCustomized)
        [self customize];
}

- (void) applyLocalization {
    [super applyLocalization];
    [self.table reloadData];
}

- (void) applyTheme {
    [super applyTheme];
    [self.table reloadData];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRoundedRect:self.page.bounds cornerRadius:8.0];
    self.page.layer.masksToBounds = NO;
    self.page.layer.cornerRadius = 8;
    self.page.layer.shadowColor = [UIColor blackColor].CGColor;
    self.page.layer.shadowRadius = 10;
    self.page.layer.shadowOpacity = 0.5f;
    self.page.layer.shadowOffset = CGSizeMake(0, 0);
    self.page.layer.shadowPath = shadowPath.CGPath;
}

+ (APPResultDetailPage *)addResultDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId haseCloseBtn:(BOOL)hasCloseBtn {
    APPResultDetailPage *ret = (APPResultDetailPage *)[APPOverlayPage addOverlayPageIn:holder withSpecificClass:[APPResultDetailPage class] displayingCloseBtn:hasCloseBtn];
    
    [APPResultDetailPage setupAddedPage:ret forRowId:rowId];
    
    return ret;
}

+ (APPResultDetailPage *)addResultDetailPageIn:(UIView *)holder forRowId:(int64_t)rowId {
    APPResultDetailPage *ret = (APPResultDetailPage *)[APPOverlayPage addOverlayPageIn:holder withSpecificClass:[APPResultDetailPage class]];
    
    [APPResultDetailPage setupAddedPage:ret forRowId:rowId];
    
    return ret;
}

+ (void)setupAddedPage:(APPResultDetailPage *)page forRowId:(int64_t)rowId {
    page.resultRowId = rowId;
    page.cursor = [NUICursor cursorWithPointer:[NUIAppData getService]->getResultDetails(page.resultRowId)];
    page.cursor.callback = page;
}


#pragma mark - Animation Overrides

- (void) overlayWillAppear {
    [self setAlpha:0.0];
    
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
}

- (void) overlayAppearAnimation {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:8];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 1.0;
        [self layoutIfNeeded];
    }];
}

- (void) disappearWithCompletion:(void (^)(BOOL finished))completion; {
    [self layoutIfNeeded];
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        [self.trailingConstraint setConstant:-self.superview.frame.size.width * 0.5f];
    }
    
    
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 0.0;
        [self layoutIfNeeded];
        
    } completion:^(BOOL finished) {
        if(finished) {
            [self removeFromSuperview];
            if(completion != nil)
                completion(finished);
        }
    }];
}


#pragma mark - Event Handling

- (void)handleLocalizationChanged {
    [self applyLocalization];
}

-(void)didTapAnywhere: (UITapGestureRecognizer*) recognizer {
    [self disappearWithCompletion:nil];
}






#pragma mark - TableViewDataSource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}



- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return MAX(self.cursor.count - 1, 0);
}



- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.cursor moveToPosition:indexPath.row + 1];
    
    bool isDiagram = [self.cursor columnIsNil:@"major"];
    NSString *title = [self.cursor getStringInColumn:@"title"];
    NSString *major = [self.cursor getStringInColumn:@"major"];
    NSString *minor = [self.cursor getStringInColumn:@"minor"];
    
    if(!isDiagram) {
        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:[APPResultDetailPage detailCellName]];
        if (cell == nil) {
            cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:[APPResultDetailPage detailCellName]];
        }
        
        cell.textLabel.text = title;
        cell.detailTextLabel.text = [NSString stringWithFormat:@"%@ %@", major, minor];
        
        return cell;
    } else {
        APPResultDetailChartCell *cell = [tableView dequeueReusableCellWithIdentifier:[APPResultDetailPage chartCellName]];

        [cell setNeedsLayout];
        
        [cell setTranslatesAutoresizingMaskIntoConstraints:YES];
        cell.chart = [[THLineChart alloc] initWithFrame:cell.bounds];
        [cell addSubview:cell.chart];
        
        [cell.chart setDataJsonString:[NSString stringWithUTF8String:[NUIAppData getService]->getChartJson(self.resultRowId, [self.cursor getIntegerInColumn:@"_id"]).c_str()]];
        
        return cell;
    }
}



-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    [self.cursor moveToPosition:indexPath.row + 1];
    bool isDiagram = [self.cursor columnIsNil:@"major"];
    if(!isDiagram) {
        return 44;
    } else {
        if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
            return self.table.frame.size.width * 0.5f;
        else
            return self.table.frame.size.width * 0.75f;
    }
}



- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    UITableViewHeaderFooterView* header = [tableView dequeueReusableHeaderFooterViewWithIdentifier:@"Header"];
    if(header == nil) {
        header = [[UITableViewHeaderFooterView alloc] initWithReuseIdentifier:@"Header"];
    }
    
    [header.contentView setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
    [self.cursor moveToPosition:0];
    UILabel *titleLabel = [[UILabel alloc] init];
    NSString *title = [self.cursor getStringInColumn:@"title"];
    if(title != nil)
        [titleLabel setText:[NUIAppData getLocalized:title]];
    [titleLabel setTextAlignment:NSTextAlignmentCenter];
    [titleLabel setFont:[THTheme getFontNamed:@"TitleLFont"]];
    [titleLabel setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [titleLabel setTranslatesAutoresizingMaskIntoConstraints:NO];
    [header addSubview:titleLabel];
    
    [header addConstraint:[NSLayoutConstraint constraintWithItem:titleLabel
                                                       attribute:NSLayoutAttributeLeading
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:header
                                                       attribute:NSLayoutAttributeLeading
                                                      multiplier:1
                                                        constant:8]];

    [header addConstraint:[NSLayoutConstraint constraintWithItem:titleLabel
                                                       attribute:NSLayoutAttributeTrailing
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:header
                                                       attribute:NSLayoutAttributeTrailing
                                                      multiplier:1
                                                        constant:-8]];
    
    [header addConstraint:[NSLayoutConstraint constraintWithItem:titleLabel
                                                       attribute:NSLayoutAttributeTop
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:header
                                                       attribute:NSLayoutAttributeTop
                                                      multiplier:1
                                                        constant:0]];
    
    [titleLabel addConstraint:[NSLayoutConstraint constraintWithItem:titleLabel
                                                       attribute:NSLayoutAttributeHeight
                                                       relatedBy:NSLayoutRelationEqual
                                                          toItem:nil
                                                       attribute:NSLayoutAttributeNotAnAttribute
                                                      multiplier:1
                                                        constant:44]];
    
    
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:header.bounds];
    header.layer.masksToBounds = NO;
    header.layer.shadowColor = [THTheme getColorNamed:@"BackColor"].CGColor;
    header.layer.shadowRadius = 3;
    header.layer.shadowOpacity = 1.0f;
    header.layer.shadowOffset = CGSizeMake(0, 6);
    header.layer.shadowPath = shadowPath.CGPath;
    
    return header;
}



-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    return 44;
}



-(void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Remove seperator inset
    if ([cell respondsToSelector:@selector(setSeparatorInset:)]) {
        [cell setSeparatorInset:UIEdgeInsetsZero];
    }
    
    // Prevent the cell from inheriting the Table View's margin settings
    if ([cell respondsToSelector:@selector(setPreservesSuperviewLayoutMargins:)]) {
        [cell setPreservesSuperviewLayoutMargins:NO];
    }
    
    // Explictly set your cell's layout margins
    if ([cell respondsToSelector:@selector(setLayoutMargins:)]) {
        [cell setLayoutMargins:UIEdgeInsetsZero];
    }
}



#pragma mark - CursorDelegate
- (void)dataSetChangedFrom:(NSInteger)from to:(NSInteger)to
{
    if(self.table != nil)
        [self.table reloadData];
}



- (void)dataSetInvalidated
{
    if(self.table != nil)
        [self.table reloadData];
}


@end
