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

#import "APPResultCell.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIUtilities.h"
#import "Common/THTheme.h"

@implementation APPResultCell

//
/**
 * Queries the best results of each type.
 * @returns a cursor with the following columns: _id, title, description, major, minor, icon,
 * testId, group, variantOf, status, primaryScore, primaryUnit, secondaryScore, secondaryUnit.
 */
//

+ (NSString *)getCellId {
    return @"APPResultCell";
}

+ (NSString *)getNibName {
    return @"APPResultCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void) setupWithCursor:(NUICursor *)cursor {
    [super setupWithCursor:cursor];
    
    [self setupMainScore:cursor];
    [self setupSecondaryScore:cursor];
}

- (void) setupMainScore:(NUICursor *)cursor {
    [self.mainResult setText:[NUIAppData getLocalized:[cursor getStringInColumn:@"major"]]];
}

- (void) setupSecondaryScore:(NUICursor *)cursor {
    NSString *minor = [cursor getStringInColumn:@"minor"];
    [self.secondaryResultHeightConstraint setActive:[minor isEqualToString:@""]];
    [self.secondaryResult setText:[NUIAppData getLocalized:minor]];
}

- (void)applyTheme {
    [super applyTheme];
    
    [self.mainResult setFont:[THTheme getFontNamed:@"FontCellScoreUpper"]];
    [self.secondaryResult setFont:[THTheme getFontNamed:@"FontCellScoreLower"]];
    
    [self.mainResult setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
    [self.secondaryResult setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
}

@end
