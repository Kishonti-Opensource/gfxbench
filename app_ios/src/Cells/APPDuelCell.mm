/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import "APPDuelCell.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"
#import "Utils/NUIUtilities.h"

typedef enum appDuelDir
{
    DUEL_NONE,
    DUEL_LEFT,
    DUEL_MID,
    DUEL_RIGHT
} AppDuelDir;

@implementation APPDuelCell

//
/**
 * Queries comparisons of two devices from the compare database
 * @param api identifies the graphics or compute api (e.g. "gl" or "dx") to return comparisons
 * for.
 * @param deviceA identifies a device to return comparisons for. Pass "own" to get results
 * from the local device.
 * @param deviceB same as deviceA.
 * @returns a cursor with the following columns: _id, title, icon, unit, scoreA, scoreB. scoreA
 * and scoreB can be null, if no valid result was found.
 */
//

+ (NSString *)getCellId {
    return @"APPDuelCell";
}

+ (NSString *)getNibName {
    return @"APPDuelCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void) setupWithCursor:(NUICursor *)cursor {
    [super setupWithCursor:cursor];

    self.leftSecondaryHeightConstraint.constant = 0;
    self.rightSecondaryHeightConstraint.constant = 0;

    [self setupMainScore:cursor inLabel:self.leftPrimaryScore];
    [self setupMainScore:cursor inLabel:self.rightPrimaryScore];
    [self setupArrows:cursor];
}

- (void)setupIcon:(NUICursor *)cursor {
    [self.icon setImage:[THTheme imageNamed:[cursor getStringInColumn:@"icon"]]];
}

- (void)setupSubtitle:(NUICursor *)cursor {
    // Do nothing
}

- (void)setupMainScore:(NUICursor *)cursor inLabel:(UILabel *)label {
    NSString *scoreName = @"scoreA";
    if(label == self.rightPrimaryScore) scoreName = @"scoreB";

    NSInteger status = [cursor getIntegerInColumn:@"status"];
    if(status == tfw::Result::OK) {
        double score = [cursor getDoubleInColumn:scoreName];
        if(score <= 0) {
            [label setText:[NUIAppData getLocalized:@"Results_NA"]];

        } else {
            NSString *formattedScore = [NUIUtilities getFormattedResult:score];
            NSString *scoreString = [NSString stringWithFormat:@"%@ %@", formattedScore, [cursor getStringInColumn:@"unit"]];
            [label setText:scoreString];
        }

    } else if(status == tfw::Result::CANCELLED) {
        [label setText:[NUIAppData getLocalized:@"STATUS_CANCELLED"]];

    } else if(status == tfw::Result::FAILED) {
        [label setText:[NUIAppData getLocalized:@"STATUS_FAILED"]];

    } else if(status == tfw::Result::INCOMPATIBLE) {
        [label setText:[NUIAppData getLocalized:@"STATUS_INCOMPATIBLE"]];
    }
}

- (void)setupArrows:(NUICursor *)cursor {
    double leftScore = [cursor getDoubleInColumn:@"scoreA"];
    double rightScore = [cursor getDoubleInColumn:@"scoreB"];

    AppDuelDir dir = DUEL_NONE;
    float percent = 0;

    if(leftScore > 0 && rightScore > 0) {
        if(leftScore > rightScore) {
            percent = (leftScore/rightScore)-1;
            dir = DUEL_LEFT;
        } else {
            percent = (rightScore/leftScore)-1;
            dir = DUEL_RIGHT;
        }

        if(ABS(percent) < 0.05)
            dir = DUEL_MID;
    }

    NSString *formattedDiff = @"";
    formattedDiff = [NUIUtilities getDuelFormattedString:[NSNumber numberWithFloat:percent]];

    self.leftArrowPercent.text = (dir == DUEL_LEFT) ? formattedDiff : @"";
    self.rightPercent.text = (dir == DUEL_RIGHT) ? formattedDiff : @"";
    if(dir == DUEL_NONE) {
        [self.leftArrow setImage:nil];
        [self.rightArrow setImage:nil];
    } else if(dir == DUEL_MID) {
        [self.leftArrow setImage:[THTheme imageNamed:@"LeftBlueDuelArrow"]];
        [self.rightArrow setImage:[THTheme imageNamed:@"RightBlueDuelArrow"]];
    } else {
        [self.leftArrow setImage:(dir == DUEL_LEFT) ? [THTheme imageNamed:@"GreenDuelArrow"] : nil];
        [self.rightArrow setImage:(dir == DUEL_RIGHT) ? [THTheme imageNamed:@"RedDuelArrow"] : nil];
    }
}

- (void)applyTheme {
    [super applyTheme];

    [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.titleLabel setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
    [self.leftArrowPercent setTextColor:[THTheme getColorNamed:@"TextCompareCellGreenColor"]];
    [self.rightPercent setTextColor:[THTheme getColorNamed:@"TextCompareCellRedColor"]];
    [self.leftPrimaryScore setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
    [self.leftSecondaryScore setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
    [self.rightPrimaryScore setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
    [self.rightSecondaryScore setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];
    [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];

    [self.titleLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.leftPrimaryScore setFont:[THTheme getFontNamed:@"FontCellScoreUpper"]];
    [self.leftSecondaryScore setFont:[THTheme getFontNamed:@"FontCellScoreLower"]];
    [self.rightPrimaryScore setFont:[THTheme getFontNamed:@"FontCellScoreUpper"]];
    [self.rightSecondaryScore setFont:[THTheme getFontNamed:@"FontCellScoreLower"]];
    [self.rightPercent setFont:[THTheme getFontNamed:@"TextSFont"]];
    [self.leftArrowPercent setFont:[THTheme getFontNamed:@"TextSFont"]];
}

- (void)displayAsLastInGroup {
    // Do nothing
}

@end
