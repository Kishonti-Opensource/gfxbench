/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import "Controls/THDropdown.h"

/**
 * @brief A label like ui element with an icon and two lines of text.
 * @discussion A view with an icon in its leftmost part, and two text
 * (a main and a secondary). The texts are clamped at 2 lines a piece.
 * The ican can be capped, and the layout can be managed from code
 * and theme as well as the colors.
 */
@interface THIconicLabel : UIView

/// Main string of the control.
@property (copy, nonatomic) NSString *mainString;
/// Secondary string of the control. Displayed beneath the main string.
@property (copy, nonatomic) NSString *secondaryString;
/// Icon of the control. Displayed before the texts.
@property (copy, nonatomic) NSString *iconName;
/// Icon color.
@property (copy, nonatomic) NSString *iconColorName;

/// Padding of the control.
@property (assign, nonatomic) float padding;
/// The empty space between the icon and the texts.
@property (assign, nonatomic) float iconTextDist;
/// The empty space between the texts.
@property (assign, nonatomic) float textTextDist;
/// Size cap for the icon.
@property (assign, nonatomic) float iconSizeCap;

/// Color of the main text.
@property (strong, nonatomic) UIColor *mainTextColor;
/// Color of the secondary text.
@property (strong, nonatomic) UIColor *secondaryTextColor;


/// Font of the main text.
@property (strong, nonatomic) UIFont *mainTextFont;
/// Font of the secondary text.
@property (strong, nonatomic) UIFont *secondaryTextFont;


@end
