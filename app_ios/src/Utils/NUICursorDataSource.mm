/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUICursorDataSource.m
//  app_ios
//
//  Created by Balazs Hajagos on 25/08/2015.
//
//

#import "Utils/NUICursorDataSource.h"
#import "Cells/NUIBaseCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Controls/THProgressBar.h"

@interface NUICursorDataSource()

@property (strong, nonatomic) NUIBaseCell *sizingCell;
@property (strong, nonatomic) NUIBaseCell *sizingHeader;
@property (strong, nonatomic) NSDictionary *sharedData;

@property (assign, nonatomic) BOOL permanentSelection;
@property (assign, nonatomic) BOOL animated;

@end

@implementation NUICursorDataSource
{
    NSArray *sectionStarts;
    
    Class cellClass;
    Class headerClass;
    Class footerClass;
}



#pragma mark - Ids and Nib names
- (NSString *)getCellId {
    if([[cellClass class] respondsToSelector:@selector(getCellId)]) {
        return [[cellClass class] performSelector:@selector(getCellId)];
    }
    
    return @"";
}



- (NSString *)getCellNibName {
    if([[cellClass class] respondsToSelector:@selector(getNibName)]) {
        return [[cellClass class] performSelector:@selector(getNibName)];
    }
    
    return @"";
}



- (NSString *)getHeaderId {
    if([[headerClass class] respondsToSelector:@selector(getCellId)]) {
        return [[headerClass class] performSelector:@selector(getCellId)];
    }
    
    return @"";
}



- (NSString *)getHeaderNibName {
    if([[headerClass class] respondsToSelector:@selector(getNibName)]) {
        return [[headerClass class] performSelector:@selector(getNibName)];
    }
    
    return @"";
}



- (BOOL)isHeadered {
    return [headerClass isSubclassOfClass:[NUIBaseCell class]];
}



#pragma mark - Cursor to Table
- (NSInteger)cursorIndexFromIndexPath:(NSIndexPath *)indexPath
{
    if([self isHeadered])
        return [[sectionStarts objectAtIndex:indexPath.section] integerValue] + indexPath.row;
    else
        return indexPath.row;
}



- (void)refreshSectionStarts
{
    if([self isHeadered]) {
        NSMutableArray *newSectionStarts = [[NSMutableArray alloc] init];
        NSString *lastGroup = nil;
        for (NSInteger i = 0; i < self.cursor.count; ++i)
        {
            NSString *group = nil;
            self.cursor.position = i;
            group = [self.cursor getStringInColumn:@"group"];
            if (![group isEqualToString:lastGroup])
            {
                lastGroup = group;
                [newSectionStarts addObject:[NSNumber numberWithInteger:i]];
            }
        }
        [newSectionStarts addObject:[NSNumber numberWithInteger:self.cursor.count]];
        sectionStarts = newSectionStarts;
    }
}



- (id)initWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header
{
    if (self = [super init]) {
        [self setupWithCursor:cursor Table:table CellType:cell HeaderType:header SharedData:nil PermanentSelection:false AnimatedRows:false];
    }
    return self;
}



- (id)initWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header PermanentSelection:(BOOL)permanentSelection
{
    if (self = [super init]) {
        [self setupWithCursor:cursor Table:table CellType:cell HeaderType:header SharedData:nil PermanentSelection:permanentSelection AnimatedRows:false];
    }
    return self;
}



- (id)initWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header PermanentSelection:(BOOL)permanentSelection AnimatedRows:(BOOL)animated
{
    if (self = [super init]) {
        [self setupWithCursor:cursor Table:table CellType:cell HeaderType:header SharedData:nil PermanentSelection:permanentSelection AnimatedRows:animated];
    }
    return self;
}



- (id)initWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header PermanentSelection:(BOOL)permanentSelection AnimatedRows:(BOOL)animated SharedData:(NSDictionary *)shared
{
    if (self = [super init]) {
        [self setupWithCursor:cursor Table:table CellType:cell HeaderType:header SharedData:shared PermanentSelection:permanentSelection AnimatedRows:animated];
    }
    return self;
}


- (void) setupWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header SharedData:(NSDictionary *)shared PermanentSelection:(BOOL)selection AnimatedRows:(BOOL)animated
{
    cellClass = cell;
    headerClass = header;
    
    self.cursor = cursor;
    self.table = table;
    
    self.cellHeight = 100;
    self.headerHeight = 44;
    self.footerHeight = 20;
    self.placeholderHeight = 4;
    
    self.sharedData = shared == nil ? @{} : shared;
    self.animated = animated;
    
    [self.table setSeparatorStyle:UITableViewCellSeparatorStyleNone];
    
    
    //        [self.table setEstimatedRowHeight:70];
    //        [self.table setRowHeight:100];
    //        [self.table setEstimatedSectionHeaderHeight:70];
    //        [self.table setSectionHeaderHeight:50];
    
    [self.table registerNib:[UINib nibWithNibName:[self getCellNibName] bundle:nil] forCellReuseIdentifier:[self getCellId]];
    
    self.sizingCell = nil;
    UINib *nib = [UINib nibWithNibName:[self getCellNibName] bundle:nil];
    
    // Assumption: The XIB file only contains a single root UIView.
    UIView *rootView = [[nib instantiateWithOwner:nil options:nil] lastObject];
    
    if ([rootView isKindOfClass:[NUIBaseCell class]]) {
        self.sizingCell = (NUIBaseCell*)rootView;
    }
    
    [self registerHeaderFooterNibsForReuse];
}



- (void) registerHeaderFooterNibsForReuse {
    
    if([self isHeadered]) {
        [self.table registerNib:[UINib nibWithNibName:[self getHeaderNibName] bundle:nil] forCellReuseIdentifier:[self getHeaderId]];
        
        
        self.sizingHeader = nil;
        UINib *nib = [UINib nibWithNibName:[self getHeaderNibName] bundle:nil];
        
        // Assumption: The XIB file only contains a single root UIView.
        UIView *rootView = [[nib instantiateWithOwner:nil options:nil] lastObject];
        
        if ([rootView isKindOfClass:[NUIBaseCell class]]) {
            self.sizingHeader = (NUIBaseCell*)rootView;
        }
    }
    
    self.table.tableFooterView = [[UIView alloc] initWithFrame:CGRectZero];
}



- (void)setCursor:(NUICursor *)cursor
{
    if (_cursor)
        _cursor.callback = nil;
    _cursor = cursor;
    [self refreshSectionStarts];
    if (_cursor)
        _cursor.callback = self;
}



#pragma mark - TableViewDataSource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    if([self isHeadered])
        return sectionStarts.count - 1;
    else
        return 1;
}



- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if(self.cursor.count > 0) {
        return [self getNumberOfRowsInSection:section];
    } else {
        return 1;
    }
}



- (NSInteger)getNumberOfRowsInSection:(NSInteger)section
{
    if([self isHeadered]) {
        NSInteger sectionStart = [(NSNumber*)[sectionStarts objectAtIndex:section] integerValue];
        NSInteger sectionEnd = [(NSNumber*)[sectionStarts objectAtIndex:section + 1] integerValue];
        return sectionEnd - sectionStart;
    }
    else
        return self.cursor.count;
}



- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(self.cursor.count > 0) {
        [self.cursor moveToPosition:[self cursorIndexFromIndexPath:indexPath]];
        NUIBaseCell *cell = (NUIBaseCell *)[tableView dequeueReusableCellWithIdentifier:[self getCellId]];
        
        if([cell respondsToSelector:@selector(setupWithCursor:)])
            [cell performSelector:@selector(setupWithCursor:) withObject:self.cursor];
        
        if([cell respondsToSelector:@selector(setSharedData:)])
            [cell performSelector:@selector(setSharedData:) withObject:self.sharedData];
        
        if([cell respondsToSelector:@selector(applyTheme)])
            [cell performSelector:@selector(applyTheme)];
        
        if([cell respondsToSelector:@selector(displayAsLastInGroup)] && indexPath.row == [self getNumberOfRowsInSection:indexPath.section] - 1 && indexPath.section + 2 < sectionStarts.count)
            [cell performSelector:@selector(displayAsLastInGroup)];
        
        return cell;
        
    } else {
        UITableViewCell *loadingCellView = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"LoadingCell"];
        
        [loadingCellView setBackgroundColor:[UIColor clearColor]];
        
        THProgressBar *pBar = [[THProgressBar alloc] init];
        [pBar setIsInfinite:true];
        [pBar setForegroundColor:[THTheme getColorNamed:@"MainColor"]];
        [pBar setBackgroundColor:self.table.backgroundColor];
        [pBar setTranslatesAutoresizingMaskIntoConstraints:NO];
        
        [loadingCellView.contentView addSubview:pBar];
        [loadingCellView.contentView addConstraint:[NSLayoutConstraint constraintWithItem:pBar
                                                                                attribute:NSLayoutAttributeLeading
                                                                                relatedBy:NSLayoutRelationEqual
                                                                                   toItem:loadingCellView.contentView
                                                                                attribute:NSLayoutAttributeLeading
                                                                               multiplier:1
                                                                                 constant:0]];
        [loadingCellView.contentView addConstraint:[NSLayoutConstraint constraintWithItem:pBar
                                                                                attribute:NSLayoutAttributeTrailing
                                                                                relatedBy:NSLayoutRelationEqual
                                                                                   toItem:loadingCellView.contentView
                                                                                attribute:NSLayoutAttributeTrailing
                                                                               multiplier:1
                                                                                 constant:0]];
        [loadingCellView.contentView addConstraint:[NSLayoutConstraint constraintWithItem:pBar
                                                                                attribute:NSLayoutAttributeHeight
                                                                                relatedBy:NSLayoutRelationEqual
                                                                                   toItem:nil
                                                                                attribute:NSLayoutAttributeNotAnAttribute
                                                                               multiplier:0
                                                                                 constant:4]];
        [loadingCellView.contentView addConstraint:[NSLayoutConstraint constraintWithItem:pBar
                                                                                attribute:NSLayoutAttributeBottom
                                                                                relatedBy:NSLayoutRelationEqual
                                                                                   toItem:loadingCellView.contentView
                                                                                attribute:NSLayoutAttributeBottom
                                                                               multiplier:1
                                                                                 constant:0]];
        
        return loadingCellView;
    }
}



- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    if(!(self.cursor.count > 0)) return nil;
    
    if([self isHeadered]) {
        [self.cursor moveToPosition:[(NSNumber*)[sectionStarts objectAtIndex:section] integerValue]];
        NUIBaseCell *cell = (NUIBaseCell *)[tableView dequeueReusableCellWithIdentifier:[self getHeaderId]];
        
        if([cell respondsToSelector:@selector(setupWithCursor:)])
            [cell performSelector:@selector(setupWithCursor:) withObject:self.cursor];
        
        if([cell respondsToSelector:@selector(applyTheme)])
            [cell performSelector:@selector(applyTheme)];
    
        return cell;
    }
    
    return nil;
}



-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    if(!(self.cursor.count > 0)) return self.placeholderHeight;
    
    BOOL isLastSectionLine = indexPath.row == [self getNumberOfRowsInSection:indexPath.section] - 1 && indexPath.section < sectionStarts.count - 2 && [self isHeadered];
    
    if(![[cellClass class] respondsToSelector:@selector(needsDynamicLayout)])
        return isLastSectionLine ? self.cellHeight + self.footerHeight : self.cellHeight;
    
    if(![cellClass needsDynamicLayout])
        return isLastSectionLine ? self.cellHeight + self.footerHeight : self.cellHeight;
    
    
    //Worst case compute
    [self.cursor moveToPosition:[self cursorIndexFromIndexPath:indexPath]];
    
    if(self.sizingCell != nil) {
        
        if([self.sizingCell respondsToSelector:@selector(setupWithCursor:)])
            [self.sizingCell performSelector:@selector(setupWithCursor:) withObject:self.cursor];
        
        if([self.sizingCell respondsToSelector:@selector(applyTheme)])
            [self.sizingCell performSelector:@selector(applyTheme)];
        
        if([self.sizingCell respondsToSelector:@selector(displayAsLastInGroup)] && indexPath.row == [self getNumberOfRowsInSection:indexPath.section] - 1 && indexPath.section < sectionStarts.count - 2)
            [self.sizingCell performSelector:@selector(displayAsLastInGroup)];
        
        [self.sizingCell setNeedsUpdateConstraints];
        [self.sizingCell updateConstraintsIfNeeded];
        
        self.sizingCell.bounds = CGRectMake(0.0f, 0.0f, CGRectGetWidth(tableView.bounds), CGRectGetHeight(self.sizingCell.bounds));
        
        [self.sizingCell setNeedsLayout];
        [self.sizingCell layoutIfNeeded];
        
        CGSize size = [self.sizingCell systemLayoutSizeFittingSize:CGSizeMake(self.table.frame.size.width, 44)
                                withHorizontalFittingPriority:UILayoutPriorityRequired
                                      verticalFittingPriority:UILayoutPriorityFittingSizeLevel];
        return size.height + 1.0f; // Add 1.0f for the cell separator height
    }
    
    return self.cellHeight;
}



-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    if(!(self.cursor.count > 0)) return 0;
    
    if(![self isHeadered])
        return 0;
    
    if(![[headerClass class] respondsToSelector:@selector(needsDynamicLayout)])
        return 0;
    
    if(![headerClass needsDynamicLayout]) return self.headerHeight;

    
    //Worst case compute
    [self.cursor moveToPosition: [self cursorIndexFromIndexPath:[NSIndexPath indexPathForItem:0 inSection:section]]];
    
    if(self.sizingHeader != nil) {
        if([self.sizingHeader respondsToSelector:@selector(setupWithCursor:)])
            [self.sizingHeader performSelector:@selector(setupWithCursor:) withObject:self.cursor];
        
        if([self.sizingHeader respondsToSelector:@selector(applyTheme)])
            [self.sizingHeader performSelector:@selector(applyTheme)];
        
        [self.sizingHeader setNeedsLayout];
        [self.sizingHeader layoutIfNeeded];
        
        CGSize size = [self.sizingHeader systemLayoutSizeFittingSize:CGSizeMake(self.table.frame.size.width, 44)
                                  withHorizontalFittingPriority:UILayoutPriorityRequired
                                        verticalFittingPriority:UILayoutPriorityFittingSizeLevel];
        return size.height + 1.0f; // Add 1.0f for the cell separator height
    }
    
    return self.headerHeight;
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
    
    if(self.animated) {
        //2. Define the initial state (Before the animation)
        cell.layer.shadowColor = [[UIColor blackColor]CGColor];
        cell.layer.shadowOffset = CGSizeMake(10, 10);
        cell.alpha = 0;
        
        //3. Define the final state (After the animation) and commit the animation
        [UIView animateWithDuration:0.2 animations: ^{
        cell.alpha = 1;
        cell.layer.shadowOffset = CGSizeMake(0, 0);
        }];
    }
}



#pragma mark - CursorDelegate
- (void)dataSetChangedFrom:(NSInteger)from to:(NSInteger)to
{
    [self refreshSectionStarts];
    if(self.table != nil)
        [self.table reloadData];
    
    if([self.delegate respondsToSelector:@selector(cursorDataSource:dataSetChangedFrom:to:)])
        [self.delegate cursorDataSource:self dataSetChangedFrom:from to:to];
}


- (void)dataSetInvalidated
{
    [self refreshSectionStarts];
    if(self.table != nil)
        [self.table reloadData];
    
    if([self.delegate respondsToSelector:@selector(cursorDataSource:dataSetInvalidated:)])
        [self.delegate cursorDataSource:self dataSetInvalidated:self.cursor];
}



#pragma mark - TableViewDelegate
-(void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if(!(self.cursor.count > 0)) return;
        
        
    self.cursor.position = [self cursorIndexFromIndexPath:indexPath];
    
    if(self.delegate != NULL) {
        [self.delegate cursorDataSource:self handleCursorSelected:self.cursor atIndexPath:indexPath];
    }
    
    if(self.permanentSelection)
        [self.table deselectRowAtIndexPath:indexPath animated:NO];
}


@end
