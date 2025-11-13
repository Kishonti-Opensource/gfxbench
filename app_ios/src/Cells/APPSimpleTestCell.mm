/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPResultCell.m
//  app_ios
//
//  Created by Balazs Hajagos on 31/08/2015.
//
//

#import "APPSimpleTestCell.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIUtilities.h"
#import "Common/THTheme.h"

@implementation APPSimpleTestCell

//
/**
 * Queries the best results of each type.
 * @returns a cursor with the following columns: _id, title, description, major, minor, icon,
 * testId, group, variantOf, status, primaryScore, primaryUnit, secondaryScore, secondaryUnit.
 */
//

+ (NSString *)getCellId {
    return @"APPSimpleTestCell";
}

+ (NSString *)getNibName {
    return @"APPSimpleTestCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    
    if(selected)
        [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"ListHighlightBackColor"]];
    else
        [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
}

@end
