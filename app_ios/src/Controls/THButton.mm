/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import "Controls/THButton.h"
#import "Common/THTheme.h"

@implementation THButton

- (id) init {
    self = [super init];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self setupWiTHTheme];
    }
    return self;
}

- (void) setupWiTHTheme {
    [self setBackgroundColor:[THTheme getColorNamed:@"Button_Background_Default"] forState:UIControlStateNormal];
    [self setBackgroundColor:[THTheme getColorNamed:@"Button_Background_Pressed"] forState:UIControlStateHighlighted];
    [self setTitleColor:[THTheme getColorNamed:@"Button_Foreground_Default"] forState:UIControlStateNormal];
    [self setTitleColor:[THTheme getColorNamed:@"Button_Foreground_Pressed"] forState:UIControlStateHighlighted];
}

- (void) setImageNamed:(NSString *)imageName {
    [self setImageNamed:imageName forState:UIControlStateNormal];
    [self setImageNamed:imageName forState:UIControlStateHighlighted];
    [self setImageNamed:imageName forState:UIControlStateDisabled];
}

- (void) setImageNamed:(NSString *)imageName forState:(UIControlState)state {
    [self setImage:[THTheme imageNamed:imageName withTintColor:[self titleColorForState:state]] forState:state];
}

- (void) setBackgroundColor:(UIColor *)backgroundColor {
    [self setBackgroundImage:[THTheme imageWithColor:backgroundColor] forState:UIControlStateNormal];
}

- (void) setBackgroundColor:(UIColor *)backgroundColor forState:(UIControlState)state {
    [self setBackgroundImage:[THTheme imageWithColor:backgroundColor] forState:state];
}

@end

@implementation UIButton (THThemedButton)

- (void) setImageNamed:(NSString *)imageName withColorName:(NSString *)colorName forState:(UIControlState)state {
    [self setImage:[THTheme imageNamed:imageName withTintColorName:colorName] forState:state];
}

@end
