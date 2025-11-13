/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "THDefaultDropdownObject.h"
#import "Controls/THIconicLabel.h"

@implementation THDefaultDropdownObject

- (UIView *)createViewWithDropped:(BOOL)dropped {
    THIconicLabel *label = [[THIconicLabel alloc] init];
    [label setMainString:self.name];
    [label setIconName:self.iconName];

    if(dropped) {
        [label setPadding:label.padding + 5.0f];
    }

    return label;
}

@end
