/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

/**
 * @brief Extended button class.
 * @discussion Subclassing UIButton and extending its functionality
 * with default theme styling, image loading.
 */
@interface THButton : UIButton

/// Sets the background color for the specified state.
- (void) setBackgroundColor:(UIColor *)backgroundColor forState:(UIControlState)state;

/// Sets the image of the Button based on the given string for all states.
- (void) setImageNamed:(NSString *)imageName;

/// Sets the image of the Button based on the given string for a specific state.
- (void) setImageNamed:(NSString *)imageName forState:(UIControlState)state;

@end


/**
 * Helper extension for UIButton.
 */
@interface UIButton (THThemedButton)

- (void) setImageNamed:(NSString *)imageName withColorName:(NSString *)colorName forState:(UIControlState)state;

@end
