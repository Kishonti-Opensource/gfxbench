/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "APPTestCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"

@interface APPTestCell()

@property (assign, nonatomic) BOOL isChecked;
@property (assign, nonatomic) BOOL isEnabled;

@end

@implementation APPTestCell

//
/**
 * Queries the list of tests packaged with the application.
 * @returns a cursor with the following columns: _id, title, icon, description,
 * incompatibilityText, isEnabled, isChecked, group, variantOf, isRunalone.
 */
//

+ (NSString *)getCellId {
    return @"APPTestCell";
}

+ (NSString *)getNibName {
    return @"APPTestCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void) setupWithCursor:(NUICursor *)cursor {
    [super setupWithCursor:cursor];

    self.isEnabled = [cursor getBooleanInColumn:@"isEnabled"];
    self.tickIcon.alpha = self.isEnabled ? 1.0 : 0.0;
    self.disableLabel.alpha = self.isEnabled ? 0.0 : 1.0;
    self.disableLabel.text = [NUIAppData getLocalized:[cursor getStringInColumn:@"incompatibilityText"]];

    [self setupTick:[cursor getBooleanInColumn:@"isChecked"]];
}

- (void) updateSelectionFromCursor:(NUICursor *)cursor {
    [self setupTick:[cursor getBooleanInColumn:@"isChecked"]];
}

- (void) setupTick:(BOOL)isChecked {
    self.isChecked = isChecked;
    if(isChecked && self.isEnabled) {
        [self.tickIcon setImage:[THTheme imageNamed:@"HomeNormalTickYes" withTintColorName:@"MainColor"]];
    } else {
        [self.tickIcon setImage:[THTheme imageNamed:@"HomeNormalTickNo"]];
    }
}

- (void)applyTheme {
    [super applyTheme];

    if(!self.isEnabled) {
        [self setBackgroundColor:[THTheme getColorNamed:@"ListDisabledBackColor"]];
        [self.icon setAlpha:0.3];
        [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleDisableColor"]];
        [self.subtitleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellDescDisableColor"]];

    } else {
        [self.icon setAlpha:1.0];
    }

    [self.disableLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleDisableColor"]];
    [self.disableLabel setFont:[THTheme getFontNamed:@"TextSFont"]];

    [self setupTick:self.isChecked];
}

@end
