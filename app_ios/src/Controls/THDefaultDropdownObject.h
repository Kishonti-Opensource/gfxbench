/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import "Controls/THDropdown.h"

@interface THDefaultDropdownObject : NSObject<DroppableObject>

@property (nonatomic, copy) NSString *name;
@property (nonatomic, copy) NSString *iconName;

- (UIView *)createViewWithDropped:(BOOL)dropped;

@end
