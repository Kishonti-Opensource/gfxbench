/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "Cells/NUIBaseCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"

@implementation NUIBaseCell

//
/**
 * Queries the list of tests packaged with the application.
 * @returns a cursor with the following columns: _id, title, icon, description,
 * incompatibilityText, isEnabled, isChecked, group, variantOf, isRunalone.
 */
//

+ (NSString *)getCellId {
    return @"NUIBaseCell";
}

+ (NSString *)getNibName {
    return @"NUIBaseCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void) setupWithCursor:(NUICursor *)cursor {
    [self setupTitle:cursor];
    [self setupSubtitle:cursor];
    [self setupIcon:cursor];
}

- (void)setupIcon:(NUICursor *)cursor {
    [self.icon setImage:[THTheme imageNamed:[cursor getStringInColumn:@"title"]]];
}

- (void)setupTitle:(NUICursor *)cursor {
    NSString *title = [NUIAppData getLocalized:[cursor getStringInColumn:@"title"]];
    [self.titleLabel setText:title];
}

- (void)setupSubtitle:(NUICursor *)cursor {

    NSString *subtitle = [NUIAppData getLocalized:[cursor getStringInColumn:@"description"]];

    // Some subtitles can be really long, so only display the
    // first 100 characters
    NSUInteger maxLength = 100;
    if([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        maxLength = 300;
    }

    if (subtitle.length > maxLength) {
        subtitle = [NSString stringWithFormat:@"%@...", [subtitle substringToIndex:maxLength]];
    }

    [self.subtitleLabel setText:subtitle];
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

- (void)applyTheme {
    [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.subtitleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellDescColor"]];

    [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];
    [self.lastLineBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];
    [self.separatorBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];
    [self.lastLineHeightConstraint setConstant:0];

    [self.separatorHeightConstraint setConstant:0.5];
    [self.separatorBack setBackgroundColor:[THTheme getColorNamed:@"SeparatorColor"]];

    [self.titleLabel setFont:[THTheme getFontNamed:@"TextLFont"]];
    [self.subtitleLabel setFont:[THTheme getFontNamed:@"TextMFont"]];

    [self.icon.layer setCornerRadius:4.0];
    [self.icon.layer setMasksToBounds:YES];
}

- (void)displayAsLastInGroup {
    [self.lastLineBack setBackgroundColor:[THTheme getColorNamed:@"ListBackColor"]];
    [self.lastLineHeightConstraint setConstant:20];
}

@end
