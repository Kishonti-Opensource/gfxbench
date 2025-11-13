/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "TfwEAGLView.h"

// A class extension to declare private methods
@interface TfwEAGLView ()

@property (nonatomic, retain) EAGLContext *context;
#ifndef USE_CADISPLAYLINK
@property (nonatomic, assign) NSTimer *animationTimer;
#endif
- (BOOL) createFramebuffer:(const tfw::Graphics*)gr;
- (void) destroyFramebuffer;

@end



@implementation TfwEAGLView

@synthesize context;
#if USE_CADISPLAYLINK
@synthesize displayLink;
@synthesize animationFrameInterval;
#else
@synthesize animationTimer;
@synthesize animationInterval;
#endif
@synthesize m_realWindowWidth;
@synthesize m_realWindowHeight;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)frame
{
	if ((self = [super initWithFrame:frame]))
	{
        if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
        {
            self.contentScaleFactor = [[UIScreen mainScreen] nativeScale];
        }
        else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
        {
            self.contentScaleFactor = [[UIScreen mainScreen] scale];
        }
		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if ((self = [super initWithCoder:aDecoder]))
    {
        if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
        {
            self.contentScaleFactor = [[UIScreen mainScreen] nativeScale];
        }
        else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
        {
            self.contentScaleFactor = [[UIScreen mainScreen] scale];
        }
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

    }
    return self;
}


- (id)initWithFrame:(CGRect)frame esVersion:(tfw::ApiDefinition)version
{
    self = [self initWithFrame:frame];

    if(version.major() >= 3)
    {
        NSLog(@"Creating OpenGLES 3.0 context.");
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if (context == nil)
        {
            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            NSLog(@"ES 3.0 not supported. Fallback to OpenGLES 2.0.");
        }
    }
    else
    {
        NSLog(@"ES 2.0 context created.");
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }

#if USE_CADISPLAYLINK
    animationFrameInterval = 1;
    animating = FALSE;
    displayLink = nil;
#else
    animationInterval = 1.0 / 60.0;
#endif

    return self;
}


- (void)setEsVersion:(tfw::ApiDefinition)version
{
    if(version.major() >= 3)
    {
        NSLog(@"Creating OpenGLES 3.0 context.");
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if (context == nil)
        {
            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            NSLog(@"ES 3.0 not supported. Fallback to OpenGLES 2.0.");
        }
    }
    else
    {
        NSLog(@"ES 2.0 context created.");
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }

#if USE_CADISPLAYLINK
    animationFrameInterval = 1;
    animating = FALSE;
    displayLink = nil;
#else
    animationInterval = 1.0 / 60.0;
#endif
}


-(BOOL)createNewbuffers:(const tfw::Graphics*)graphics
{
	[EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    return [self createFramebuffer:graphics];

}


int FindFSAAextension()
{
	int fsaa;
	glGetIntegerv(GL_MAX_SAMPLES_APPLE, &fsaa);
	return fsaa;
}


- (BOOL)createFramebuffer:(const tfw::Graphics*)graphics
{
    int red = 8;
    int green = 8;
    int blue = 8;
    int depth = -1;
    int fsaa = -1;

    if(graphics != NULL)
    {
        const tfw::Config *config = &(graphics->config());
        red = config->red();
        green = config->green();
        blue = config->blue();
        depth = config->depth();
        fsaa = config->samples();
    }

	if(fsaa > 0)
    {
		int max_fsaa = FindFSAAextension();

        if(max_fsaa < fsaa)
        {
            m_FSAA = 0;
            return NO;
        }
        else
            m_FSAA = fsaa;
	}
    else
    {
		m_FSAA = 0;
	}

    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

    NSString *color_format;

    if(red == 8 && green == 8 && blue == 8)
    {
        color_format = kEAGLColorFormatRGBA8;
    }
    else
    {
        color_format = kEAGLColorFormatRGB565;
    }

    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, color_format, kEAGLDrawablePropertyColorFormat, nil];

    glGenFramebuffers(1, &viewFramebuffer);
    glGenRenderbuffers(1, &viewRenderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, viewFramebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, viewRenderbuffer);

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_realWindowWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_realWindowHeight);

    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    if( depth == 16 || depth == -1 )
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_realWindowWidth, m_realWindowHeight);
    }
    else
    {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, m_realWindowWidth, m_realWindowHeight);
    }

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

	if(m_FSAA > 0)
	{
		glGenFramebuffers(1, &msaaFramebuffer);
		glGenRenderbuffers(1, &msaaRenderbuffer);
		glGenRenderbuffers(1, &msaaDepthbuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderbuffer);

        if( depth == 16 || depth == -1 )
        {
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, m_FSAA, GL_RGBA8_OES, m_realWindowWidth, m_realWindowHeight);
        }
        else
        {
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, m_FSAA, GL_RGB565, m_realWindowWidth, m_realWindowHeight);
        }

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthbuffer);

        if( depth == 16 || depth == -1 )
        {
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, m_FSAA, GL_DEPTH_COMPONENT16, m_realWindowWidth, m_realWindowHeight);
        }
        else
        {
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, m_FSAA, GL_DEPTH_COMPONENT24_OES, m_realWindowWidth, m_realWindowHeight);
        }

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthbuffer);
	}

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }

    return YES;
}


- (void)destroyFramebuffer {

    glDeleteFramebuffers(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffers(1, &viewRenderbuffer);
    viewRenderbuffer = 0;

	glDeleteFramebuffers(1, &msaaFramebuffer);
	msaaFramebuffer=0;
	glDeleteRenderbuffers(1, &msaaDepthbuffer);
	msaaDepthbuffer=0;
	glDeleteRenderbuffers(1, &msaaRenderbuffer);
	msaaRenderbuffer=0;


	glDeleteRenderbuffers(1, &depthRenderbuffer);
    depthRenderbuffer = 0;
}


- (bool) swapBuffers
{
    if(m_FSAA)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, msaaFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, viewFramebuffer);
		glResolveMultisampleFramebufferAPPLE();
        GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
		glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 3, attachments);
		glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);
	}
    else
	{
		GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments);
	}

	EAGLContext *oldContext = [EAGLContext currentContext];

	if(oldContext != context)
	{
		[EAGLContext setCurrentContext:context];
	}

	glBindRenderbuffer(GL_RENDERBUFFER, viewRenderbuffer);

	if(![context presentRenderbuffer:GL_RENDERBUFFER])
    {
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
        return false;
    }

	if(oldContext != context)
		[EAGLContext setCurrentContext:oldContext];

    return true;
}

- (void) setCurrentContext
{
	EAGLContext *oldContext = [EAGLContext currentContext];

	if (oldContext != context)
	{
		[EAGLContext setCurrentContext:context];
	}
}


- (void)dealloc {

    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }

    [context release];
    context = nil;

    [super dealloc];
}

- (void)deleteContext
{
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }

    [context release];
    context = nil;
}

- (EAGLContext *)getContext
{
    return context;
}


@end
