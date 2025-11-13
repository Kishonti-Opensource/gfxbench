/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPVerticalSelector.m
//  app_ios
//
//  Created by Balazs Hajagos on 01/09/2015.
//
//

#import "APPVerticalSelector.h"
#import "Common/THTheme.h"
#import "Common/NUIAppData.h"

@implementation APPVerticalSelector

- (void) updateFromCursor:(NUICursor *)cursor {
    [self setupTitle:cursor];
    [self setupSubtitle:cursor];
    [self setupIcon:cursor];
}

- (void)setupIcon:(NUICursor *)cursor {
    [self.icon setImage:[THTheme imageNamed:[cursor getStringInColumn:@"title"]]];
}

- (void)setupTitle:(NUICursor *)cursor {
    NSString *title = [NUIAppData getLocalized:[cursor getStringInColumn:@"title"]];
    [self.title setText:title];
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
    
//    [self.subtitle setText:subtitle];
}

- (void)applyTheme {
    [self setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
    [self.title setTextColor:[THTheme getColorNamed:@"TextNormalCellTitleColor"]];
//    [self.subtitle setTextColor:[THTheme getColorNamed:@"TextNormalCellDescColor"]];
    
    [self.cellBack setBackgroundColor:[THTheme getColorNamed:@"TransparentColor"]];
    
    [self.title setFont:[THTheme getFontNamed:@"TextLFont"]];
//    [self.subtitle setFont:[THTheme getFontNamed:@"TextMFont"]];
    
    [self.icon.layer setCornerRadius:4.0f];
    [self.icon.layer setMasksToBounds:YES];
}

- (IBAction)leftBtnPressed:(id)sender {
    if([self.delegate respondsToSelector:@selector(leftBtnPressedForSelector:)])
        [self.delegate leftBtnPressedForSelector:self];
}

- (IBAction)rightBtnPressed:(id)sender {
    if([self.delegate respondsToSelector:@selector(rightBtnPressedForSelector:)])
        [self.delegate rightBtnPressedForSelector:self];
}

+ (APPVerticalSelector *)addSelectorIn:(UIView *)holder {
    APPVerticalSelector *ret = nil;
    UINib *nib = [UINib nibWithNibName:NSStringFromClass([APPVerticalSelector class]) bundle:nil];
    
    // Assumption: The XIB file only contains a single root UIView.
    UIView *rootView = [[nib instantiateWithOwner:nil options:nil] lastObject];
    [rootView setTranslatesAutoresizingMaskIntoConstraints:NO];
    
    if ([rootView isKindOfClass:[APPVerticalSelector class]]) {
        ret = (APPVerticalSelector *)rootView;
        
        [holder setLayoutMargins:UIEdgeInsetsZero];
        [holder addSubview:ret];
        
        NSDictionary *viewKeys = @{@"selector":ret};
        [holder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-[selector]-|" options:0 metrics:nil views:viewKeys]];
        [holder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[selector]-|" options:0 metrics:nil views:viewKeys]];
    }
    
    return ret;
}

@end
