/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "graphics/graphicscontext.h"
#import "testfw.h"
#import "descriptors.h"

@interface METLView : UIView

@end

@interface TestContextProvider : NSObject

- (void) setVersion:(tfw::Graphics *)graphics withFrame:(CGRect)frame;
- (GraphicsContext *) getGraphicsContext;
- (UIView *) getTestView;

@end
