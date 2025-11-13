/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "APPCompareCell.h"
#import "Common/NUIAppData.h"
#import "Utils/NUIUtilities.h"
#import "Common/THTheme.h"

@implementation APPCompareCell

//
/**
 * Queries the best results of each type.
 * @returns a cursor with the following columns: _id, title, description, major, minor, icon,
 * testId, group, variantOf, status, primaryScore, primaryUnit, secondaryScore, secondaryUnit.
 */
//

+ (NSString *)getCellId {
    return @"APPCompareCell";
}

+ (NSString *)getNibName {
    return @"APPCompareCell";
}

+ (BOOL)needsDynamicLayout {
    return NO;
}

- (void)layoutSubviews {
    [super layoutSubviews];
}

- (void) setupWithCursor:(NUICursor *)cursor {
    [super setupWithCursor:cursor];

    [self setNeedsUpdateConstraints];
    [self updateConstraintsIfNeeded];
    [self setNeedsLayout];
    [self layoutIfNeeded];

    [self setupMainScore:cursor];
    [self setupSecondaryScore:cursor];
    [self setupProgressWithCursor:cursor];
}

- (void) setupProgressWithCursor:(NUICursor *)cursor {

    double selfScore = MAX([cursor getDoubleInColumn:@"primaryScore"], 0);
    double best = MAX([cursor getDoubleInColumn:@"maxScore"], 1);
    double progress = selfScore / best;

    if(self.progressWidthConstraint == nil) {
        self.progressWidthConstraint = [NSLayoutConstraint constraintWithItem:self.progressView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1 constant:0];
        [self.progressView.superview addConstraint:self.progressWidthConstraint];
    }
    [self.progressWidthConstraint setConstant:0];
    [self layoutIfNeeded];


    [self.progressWidthConstraint setConstant:self.frame.size.width * progress];
    [UIView animateWithDuration:0.2 animations:^{
        [self layoutIfNeeded];

    }];
}

- (void)setupIcon:(NUICursor *)cursor {
    NSString* deviceImage = [cursor getStringInColumn:@"icon"];
    [self.icon setImage:[THTheme imageNamed:deviceImage]];
}

- (void)setupSubtitle:(NUICursor *)cursor {
    NSString *subtitle = [NUIAppData getLocalized:[cursor getStringInColumn:@"api"]];
    [self.subtitleLabel setText:subtitle];
}

- (void) setupMainScore:(NUICursor *)cursor {
    NSInteger status = [cursor getIntegerInColumn:@"status"];
    if(status == tfw::Result::OK) {
        double score = [cursor getDoubleInColumn:@"primaryScore"];
        if(score <= 0) {
            [self.mainResult setText:[NUIAppData getLocalized:@"Results_NA"]];

        } else {
            NSString *formattedScore = [NUIUtilities getFormattedResult:score];
            NSString *scoreString = [NSString stringWithFormat:@"%@ %@", formattedScore, [cursor getStringInColumn:@"primaryUnit"]];
            [self.mainResult setText:scoreString];
        }

    } else if(status == tfw::Result::CANCELLED) {
        [self.mainResult setText:[NUIAppData getLocalized:@"STATUS_CANCELLED"]];

    } else if(status == tfw::Result::FAILED) {
        [self.mainResult setText:[NUIAppData getLocalized:@"STATUS_FAILED"]];

    } else if(status == tfw::Result::INCOMPATIBLE) {
        [self.mainResult setText:[NUIAppData getLocalized:@"Results_NA"]];
    }
}

- (void) setupSecondaryScore:(NUICursor *)cursor {
    NSInteger status = [cursor getIntegerInColumn:@"status"];
    [self.secondaryResult setText:[NUIAppData getLocalized:@""]];
    [self.secondaryResultHeightConstraint setActive:YES];

    if(status == tfw::Result::OK) {
        double score = [cursor getDoubleInColumn:@"secondaryScore"];
        if(score > 0) {
            NSString *formattedScore = [NUIUtilities getFormattedResult:score];
            NSString *scoreString = [NSString stringWithFormat:@"(%@ %@)", formattedScore, [cursor getStringInColumn:@"secondaryUnit"]];
            [self.secondaryResult setText:scoreString];
            [self.secondaryResultHeightConstraint setActive:NO];
        }

    }
}

- (void)applyTheme {
    [super applyTheme];

    [self.titleLabel setFont:[THTheme getFontNamed:@"TextMFont"]];
    [self.subtitleLabel setFont:[THTheme getFontNamed:@"TextSFont"]];

    [self.mainResult setFont:[THTheme getFontNamed:@"FontCellScoreUpper"]];
    [self.secondaryResult setFont:[THTheme getFontNamed:@"FontCellScoreLower"]];

    [self.mainResult setTextColor:[THTheme getColorNamed:@"TextScoreUpperColor"]];
    [self.secondaryResult setTextColor:[THTheme getColorNamed:@"TextScoreLowerColor"]];

    [self.progressView setBackgroundColor:[THTheme getColorNamed:@"CompareHighlightValueBackColor"]];

    [self.icon setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    self.icon.layer.borderWidth = 1.0;
    self.icon.layer.borderColor = [[THTheme getColorNamed:@"SeparatorColor"] CGColor];

    [self.titleLabel setNumberOfLines:2];
}

@end
