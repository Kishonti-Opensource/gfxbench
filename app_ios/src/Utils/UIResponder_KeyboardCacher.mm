/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "UIResponder_KeyboardCacher.h"

static BOOL hasAlreadyCachedKeyboard;

@interface UIResponder (KeyboardCache_Private)
+(void) __cacheKeyboard;
@end

@implementation UIResponder (KeyboardCache)

+(void) cacheKeyboard {
    [[self class] cacheKeyboard:NO];
}

+(void) cacheKeyboard:(BOOL)onNext {
    if (! hasAlreadyCachedKeyboard)
    {
        hasAlreadyCachedKeyboard = YES;
        if (onNext)
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.0), dispatch_get_main_queue(), ^(void){ [[self class] __cacheKeyboard]; });
        else
            [[self class] __cacheKeyboard];
    }
}

+(void) __cacheKeyboard {
    UITextField *field = [UITextField new];
    [[[[UIApplication sharedApplication] windows] lastObject] addSubview:field];
    field.keyboardAppearance = UIKeyboardAppearanceDefault;
    [field becomeFirstResponder];
    [field resignFirstResponder];
    [field removeFromSuperview];
}

@end
