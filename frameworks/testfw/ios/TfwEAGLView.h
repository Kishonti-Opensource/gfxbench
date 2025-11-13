/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include "schemas/descriptors.h"
#include "schemas/apidefinition.h"

#define USE_CADISPLAYLINK 1

@interface TfwEAGLView : UIView
{

@private

    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;

    /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
    GLuint depthRenderbuffer;

	GLuint msaaFramebuffer, msaaRenderbuffer, msaaDepthbuffer;

	int m_realWindowWidth;
	int m_realWindowHeight;



@public
    EAGLContext *context;
	int m_FSAA;
#if USE_CADISPLAYLINK
    BOOL animating;
    NSInteger animationFrameInterval;
#else
    NSTimer *animationTimer;
    NSTimeInterval animationInterval;
#endif

}

@property (assign, nonatomic) int m_realWindowWidth;
@property (assign, nonatomic) int m_realWindowHeight;
#if USE_CADISPLAYLINK
@property (assign, nonatomic) NSInteger animationFrameInterval;
@property (retain, nonatomic) CADisplayLink *displayLink;
#else
@property (assign, nonatomic) NSTimeInterval animationInterval;
#endif

- (bool)swapBuffers;
- (void)setCurrentContext;
- (BOOL)createNewbuffers:(const tfw::Graphics*)graphics;

- (EAGLContext *)getContext;
- (void)deleteContext;
- (id)initWithFrame:(CGRect)frame esVersion:(tfw::ApiDefinition)version;
- (void)setEsVersion:(tfw::ApiDefinition)version;

@end