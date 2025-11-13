/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>

/**
 * @typedef THProgressbarStyle
 * @brief Styles defined for THProgressBar.
 */
typedef NS_OPTIONS(NSUInteger, THProgressbarStyle) {
    /// Foreground proportionally fill background.
    THProgressbarStyleNormal        = 0,
    /// Foreground proportionally fill background but has padding.
    THProgressbarStylePadded        = 1 << 0,
    /// Background is a narrow line under the Foreground rect.
    THProgressbarStyleUnderlined    = 1 << 1
};

/**
 * @brief Progress bar with custom drawing, and styles
 * @discussion May draw
 * gradiented foreground.
 */
@interface THProgressBar : UIView

/// Progress to be shown in this progressbar.
@property (nonatomic, assign) float progress;

/// Progress to be shown in this progressbar.
@property (nonatomic, assign) BOOL isInfinite;

/// Padding for styles, defaults to: Progressbar_Padding_Default
@property (nonatomic, assign) float padding;

/// Sets the layers to be fully rounded
@property (nonatomic, assign) BOOL isRounded;

/// Style of this progress bar, defaults to: THProgressbarStyleNormal
@property (nonatomic, assign) THProgressbarStyle style;

/// Sets the foreground color without gradient, defaults to: Progressbar_Foreground_Default
- (void)setForegroundColor:(UIColor *)color;

/// Sets the foreground gradients color for 0 value, defaults to: Progressbar_Foreground_Default
- (void)setForegroundStartColor:(UIColor *)color;

/// Sets the foreground gradients color for 1 value, defaults to: Progressbar_Foreground_Default
- (void)setForegroundEndColor:(UIColor *)color;

/// Sets the background color, defaults to: Progressbar_Background_Default
- (void)setBackgroundColor:(UIColor *)color;

@end
