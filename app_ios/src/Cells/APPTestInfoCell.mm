/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPDuelCell.m
//  app_ios
//
//  Created by Balazs Hajagos on 02/09/2015.
//
//

#import "APPTestInfoCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIUtilities.h"

@implementation APPTestInfoCell

+ (NSString *)getCellId {
    return @"APPTestInfoCell";
}

+ (NSString *)getNibName {
    return @"APPTestInfoCell";
}

+ (BOOL)needsDynamicLayout {
    return YES;
}

- (void)setupSubtitle:(NUICursor *)cursor {
    NSString *subtitle = [NUIAppData getLocalized:[cursor getStringInColumn:@"description"]];
    [self.subtitleLabel setText:subtitle];
}

- (void)setupIcon:(NUICursor *)cursor {
    NSString *icon = [cursor getStringInColumn:@"title"];
    
    if(![icon containsString:@"_off"]) {
        icon = [icon stringByAppendingString:@"_full"];
        [self.icon setImage:[THTheme imageNamed:icon]];
    } else {
        self.icon.image = nil;
    }
}

- (void)applyTheme {
    [super applyTheme];
//    [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
//    [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
//    [self.leftArrowPercent setTextColor:[THTheme getColorNamed:@"TextCompareCellGreenColor"]];
//    [self.rightPercent setTextColor:[THTheme getColorNamed:@"TextCompareCellRedColor"]];
//    [self.leftPrimaryScore setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
//    [self.leftSecondaryScore setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
//    [self.rightPrimaryScore setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
//    [self.rightSecondaryScore setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
//    [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];
//    
//    [self.titleLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
//    [self.leftPrimaryScore setFont:[THTheme getFontNamed:@"TitleSFont"]];
//    [self.leftSecondaryScore setFont:[THTheme getFontNamed:@"TextSFont"]];
//    [self.rightPrimaryScore setFont:[THTheme getFontNamed:@"TitleSFont"]];
//    [self.rightSecondaryScore setFont:[THTheme getFontNamed:@"TextSFont"]];
}

- (void)displayAsLastInGroup {
    // Do nothing
}

@end
