/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  APPOverlayPage.m
//  app_ios
//
//  Created by Balazs Hajagos on 11/09/2015.
//
//

#import "APPOverlayPage.h"
#import "Common/NUIAppData.h"
#import "Common/THTheme.h"

@implementation APPOverlayPage

- (void) overlayWillAppear {
    [self setAlpha:0.0];
}

- (void) overlayAppearAnimation {
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 1.0;
        
    }];
}

- (void) disappearWithCompletion:(void (^)(BOOL finished))completion; {
    [UIView animateWithDuration:0.2 animations:^{
        self.alpha = 0.0;
        
    } completion:^(BOOL finished) {
        if(finished) {
            [self removeFromSuperview];
            if(completion != nil)
                completion(finished);
        }
    }];
}

- (void)displayCloseBtn {
    [self.backView setBackgroundColor:[UIColor clearColor]];
    if(self.blurView != nil) {
        [self.blurView setHidden:NO];
    }
    if(self.closeBtn != nil) {
        [self.closeBtn setHidden:NO];
    }
    if(self.closeHidingBottomConstraint != nil) {
        [self.closeHidingBottomConstraint setConstant:60];
    }
    
    [self layoutIfNeeded];
    [self applyTheme];
}

- (IBAction)closeBtnClick:(id)sender {
    [self disappearWithCompletion:NULL];
}

- (void) applyLocalization {
    if(self.closeBtn != nil) {
        [self.closeBtn setTitle:[NUIAppData getLocalized:@"Close"] forState:UIControlStateNormal];
    }
}

- (void) applyTheme {
    if(self.closeBtn != nil) {
        [self.closeBtn setTitleColor:[THTheme getColorNamed:@"MainColor"] forState:UIControlStateNormal];
        [self.closeBtn setBackgroundColor:[THTheme getColorNamed:@"BackColor"]];
        UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRoundedRect:self.closeBtn.bounds cornerRadius:8.0];
        self.closeBtn.layer.masksToBounds = NO;
        self.closeBtn.layer.cornerRadius = 8;
        self.closeBtn.layer.shadowColor = [UIColor blackColor].CGColor;
        self.closeBtn.layer.shadowRadius = 10;
        self.closeBtn.layer.shadowOpacity = 0.5f;
        self.closeBtn.layer.shadowOffset = CGSizeMake(0, 0);
        self.closeBtn.layer.shadowPath = shadowPath.CGPath;
    }
}

+ (APPOverlayPage *)addOverlayPageIn:(UIView *)holder withSpecificClass:(Class)overlayClass displayingCloseBtn:(BOOL)addCloseBtn {
    APPOverlayPage *page = [APPOverlayPage addOverlayPageIn:holder withSpecificClass:overlayClass];
    if(addCloseBtn) {
        [page displayCloseBtn];
    }
    
    return page;
}

+ (APPOverlayPage *)addOverlayPageIn:(UIView *)holder withSpecificClass:(Class)overlayClass {
    APPOverlayPage *ret = nil;
    UINib *nib = [UINib nibWithNibName:NSStringFromClass(overlayClass) bundle:nil];
    
    // Assumption: The XIB file only contains a single root UIView.
    UIView *rootView = [[nib instantiateWithOwner:nil options:nil] lastObject];
    [rootView setTranslatesAutoresizingMaskIntoConstraints:NO];
    
    if ([rootView isKindOfClass:[APPOverlayPage class]]) {
        ret = (APPOverlayPage *)rootView;
        
        
        [holder setLayoutMargins:UIEdgeInsetsZero];
        [holder addSubview:ret];
        [holder bringSubviewToFront:ret];
        
        [ret overlayWillAppear];
        
        [holder addConstraint:[NSLayoutConstraint constraintWithItem:ret
                                                           attribute:NSLayoutAttributeLeading
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem:holder
                                                           attribute:NSLayoutAttributeLeading
                                                          multiplier:1
                                                            constant:0]];
        
        [holder addConstraint:[NSLayoutConstraint constraintWithItem:ret
                                                           attribute:NSLayoutAttributeTrailing
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem:holder
                                                           attribute:NSLayoutAttributeTrailing
                                                          multiplier:1
                                                            constant:0]];
        
        [holder addConstraint:[NSLayoutConstraint constraintWithItem:ret
                                                           attribute:NSLayoutAttributeTop
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem:holder
                                                           attribute:NSLayoutAttributeTop
                                                          multiplier:1
                                                            constant:0]];
        
        [holder addConstraint:[NSLayoutConstraint constraintWithItem:ret
                                                           attribute:NSLayoutAttributeBottom
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem:holder
                                                           attribute:NSLayoutAttributeBottom
                                                          multiplier:1
                                                            constant:0]];
        
        [ret overlayAppearAnimation];
    }
    
    return ret;
}

@end
