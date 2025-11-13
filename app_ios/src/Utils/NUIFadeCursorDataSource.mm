/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  NUIFadeCursorDataSource.m
//  app_ios
//
//  Created by Balazs Hajagos on 26/10/2015.
//
//

#import "NUIFadeCursorDataSource.h"
#import "Utils/NUICursor.h"
#import "Common/THTheme.h"

@implementation NUIFadeCursorDataSource



- (BOOL)isHeadered {
    return false;
}



- (void) registerHeaderFooterNibsForReuse {
}




//- (void) setupWithCursor:(NUICursor *)cursor Table:(UITableView *)table CellType:(Class)cell HeaderType:(Class)header SharedData:(NSDictionary *)shared PermanentSelection:(BOOL)selection AnimatedRows:(BOOL)animated
//{
//    [super setupWithCursor:cursor Table:table CellType:cell HeaderType:header SharedData:shared PermanentSelection:selection AnimatedRows:animated];
//    self.placeholderHeight = 20;
//}



#pragma mark - TableViewDataSource
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}



- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
    if(!(self.cursor.count > 0)) return nil;
    
    UITableViewHeaderFooterView* header = [tableView dequeueReusableHeaderFooterViewWithIdentifier:@"Header"];
    if(header == nil) {
        header = [[UITableViewHeaderFooterView alloc] initWithReuseIdentifier:@"Header"];
    }
    
    [header.contentView setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:CGRectMake(0, 0, tableView.frame.size.width, 10)];
    header.layer.masksToBounds = NO;
    header.layer.shadowColor = [THTheme getColorNamed:@"BackColor"].CGColor;
    header.layer.shadowRadius = 2;
    header.layer.shadowOpacity = 1.0f;
    header.layer.shadowOffset = CGSizeMake(0, 6);
    header.layer.shadowPath = shadowPath.CGPath;
    
    return header;
}



-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    if(self.cursor.count > 0)
        return 10;
    else
        return 0;
}



- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
    if(!(self.cursor.count > 0)) return nil;
    
    UITableViewHeaderFooterView* footer = [tableView dequeueReusableHeaderFooterViewWithIdentifier:@"Footer"];
    if(footer == nil) {
        footer = [[UITableViewHeaderFooterView alloc] initWithReuseIdentifier:@"Footer"];
    }
    
    [footer.contentView setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:CGRectMake(0, 0, tableView.frame.size.width, 10)];
    footer.layer.masksToBounds = NO;
    footer.layer.shadowColor = [THTheme getColorNamed:@"BackColor"].CGColor;
    footer.layer.shadowRadius = 2;
    footer.layer.shadowOpacity = 1.0f;
    footer.layer.shadowOffset = CGSizeMake(0, -6);
    footer.layer.shadowPath = shadowPath.CGPath;
    
    return footer;
}



-(CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
    if(self.cursor.count > 0)
        return 10;
    else
        return 0;
}



@end
