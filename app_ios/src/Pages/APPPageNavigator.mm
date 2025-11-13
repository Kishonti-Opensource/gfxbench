/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPPageNavigator.m
//  app_ios
//
//  Created by Balazs Hajagos on 28/08/2015.
//
//

#import "APPPageNavigator.h"

@implementation APPPageNavigator

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

- (IBAction)leftBtnPressed:(id)sender {
    if([self.delegate respondsToSelector:@selector(navigationLeftPressed)])
        [self.delegate navigationLeftPressed];
}

- (IBAction)rightBtnPressed:(id)sender {
    if([self.delegate respondsToSelector:@selector(navigationRightPressed)])
        [self.delegate navigationRightPressed];
}

+ (APPPageNavigator *)addNavigatorIn:(UIView *)holder {
    APPPageNavigator *ret = nil;
    UINib *nib = [UINib nibWithNibName:NSStringFromClass([APPPageNavigator class]) bundle:nil];
    
    // Assumption: The XIB file only contains a single root UIView.
    UIView *rootView = [[nib instantiateWithOwner:nil options:nil] lastObject];
    [rootView setTranslatesAutoresizingMaskIntoConstraints:NO];
    
    if ([rootView isKindOfClass:[APPPageNavigator class]]) {
        ret = (APPPageNavigator *)rootView;
        
        [holder setLayoutMargins:UIEdgeInsetsZero];
        [holder addSubview:ret];
        
        NSDictionary *viewKeys = @{@"navigator":ret};
        [holder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-[navigator]-|" options:0 metrics:nil views:viewKeys]];
        [holder addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[navigator]-|" options:0 metrics:nil views:viewKeys]];
    }
    
    return ret;
}

@end
