/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "ViewController.h"
#import "TfwEAGLView.h"

@interface GlViewController : ViewController
{
    @public EAGLContext *context_;
}

@property (strong, nonatomic) IBOutlet TfwEAGLView *glview;

@end
