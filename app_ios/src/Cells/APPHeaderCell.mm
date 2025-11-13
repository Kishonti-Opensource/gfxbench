/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "APPHeaderCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"

@interface APPHeaderCell()

@end

@implementation APPHeaderCell

//
/**
 * Queries the list of tests packaged with the application.
 * @returns a cursor with the following columns: _id, title, icon, description,
 * incompatibilityText, isEnabled, isChecked, group, variantOf, isRunalone.
 */
//

+ (NSString *)getCellId {
    return @"APPHeaderCell";
}

+ (NSString *)getNibName {
    return @"APPHeaderCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void)setupTitle:(NUICursor *)cursor {
    [self.titleLabel setText:[NUIAppData getLocalized:[cursor getStringInColumn:@"group"]]];
}

- (void)setupSubtitle:(NUICursor *)cursor {
    // Do nothing
}

- (void)setupIcon:(NUICursor *)cursor {
    // Do nothing
}

- (void)applyTheme {
    [self setBackgroundColor:[THTheme getColorNamed:@"MainColor"]];
    [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextHeaderColor"]];
    [self.titleLabel setFont:[THTheme getFontNamed:@"TextLFont"]];
}

- (void)displayAsLastInGroup {
    // Do nothing
}

@end
