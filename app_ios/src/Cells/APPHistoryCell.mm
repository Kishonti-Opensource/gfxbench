/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPHistoryCell.m
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import "APPHistoryCell.h"
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"

@implementation APPHistoryCell

//
/**
 * Queries the list of past test sessions.
 * @returns a cursor with the following columns: _id, title, configuration.
 */
//

+ (NSString *)getCellId {
    return @"APPHistoryCell";
}

+ (NSString *)getNibName {
    return @"APPHistoryCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void)setupTitle:(NUICursor *)cursor {
    [self.titleLabel setText:[NUIAppData getLocalized:[cursor getStringInColumn:@"title"]]];
}

- (void)setupSubtitle:(NUICursor *)cursor {
    // Do nothing
}

- (void)setupIcon:(NUICursor *)cursor {
    // Do nothing
}

- (void)applyTheme {
    [super applyTheme];
    [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.titleLabel setFont:[THTheme getFontNamed:@"TextLFont"]];
}

- (void)displayAsLastInGroup {
    // Do nothing
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
    
    if(selected)
        [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"ListHighlightBackColor"]];
    else
        [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
}

@end
