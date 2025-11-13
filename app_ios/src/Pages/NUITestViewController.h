/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#include "graphics/eaglgraphicscontext.h"
#import "Controls/NUILoadingBar.h"
#import "Utils/TestContextProvider.h"
#import "Common/NUICommandLineParser.h"

@interface NUITestViewController : UIViewController
{
	@public EAGLContext *context_;
	//EAGLGraphicsContext *ctx_;
    GraphicsContext *ctx_;
}

@property (strong, nonatomic) NUILoadingBar *loadingBar;
//@property (strong, nonatomic) TfwEAGLView *glView;
@property (strong, nonatomic) UIView *glView;
@property (strong, nonatomic) NSArray *testNames;
@property (assign, nonatomic) BOOL isCancelled;

@property (strong, nonatomic) TestContextProvider *contextProvider;

// Commandline thingy...
@property (assign, nonatomic) NUICommandLineParser *CommandLine;

#if USE_CADISPLAYLINK
@property (nonatomic, assign) BOOL animating;
@property (nonatomic, assign) CADisplayLink *displayLink;
@property (nonatomic, assign) NSInteger animationFrameInterval;
#endif

- (void)CancelTests;

- (IBAction)buttonPressed:(id)sender;

@end