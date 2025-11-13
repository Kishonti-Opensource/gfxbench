/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

@class APPMainControl;

/**
 Handler protocol for AppMainControl controls.
 Lets the condforming interfaces handling of the main and side button pushes.
 */
@protocol APPMainControlHandler <NSObject>

@optional
- (void) handleMainPressedForMainControl:(APPMainControl *)control;
- (void) handleSidePressedForMainControl:(APPMainControl *)control;

@end


/**
 A control with two buttons aligned in a gearlike manner.
 Supports color, font size, and text changes. Provides a handler for the button interaction handling.
 */
@interface APPMainControl : UIControl

@property (strong, nonatomic) id<APPMainControlHandler> handler;

@property (strong, nonatomic) UIColor *mainColor;
@property (strong, nonatomic) UIColor *sideColor;
@property (strong, nonatomic) UIColor *backColor;
@property (strong, nonatomic) UIColor *mainTextColor;
@property (strong, nonatomic) UIColor *sideTextColor;

@property (assign, nonatomic) float mainFontFactor;
@property (assign, nonatomic) float sideFontFactor;

@property (copy, nonatomic) NSString *mainText;
@property (copy, nonatomic) NSString *sideText;

/**
 @name widthToCenteredWidthRatio
 @return the ratio that should be used as a width-to-width multiplier if a centering constraint based on the main button is needed.
 */
@property (readonly, nonatomic, getter=getWidthToCenteredWidthRatio) CGFloat widthToCenteredWidthRatio;

@end