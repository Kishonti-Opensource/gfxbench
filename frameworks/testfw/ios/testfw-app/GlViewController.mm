/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#import "GlViewController.h"
#import "NSFileManagerLocations.h"
#import "ng/log.h"
#import "testfw.h"
#import "schemas/descriptors.h"
#import "graphics/eaglgraphicscontext.h"
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>
#ifdef HAVE_GLES
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#endif
#ifdef HAVE_GLES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif


#pragma mark - Glchecks

void _CHECK_GLERROR ()
{
    int err = glGetError();
    if (err)
        NGLOG_TRACE("GLError: %s", err);
}

void _CHECK_GLFBOERROR ()
{
    int err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE)
    {
        NGLOG_TRACE("Status: %s", err);
    }
}
#define CHECK_GLERROR _CHECK_GLERROR();
#define CHECK_GLFBOERROR _CHECK_GLFBOERROR();



#pragma mark - GLViewController

@interface GlViewController ()

@end

@implementation GlViewController

- (void)initContext
{
    self->ctx_ = new EAGLGraphicsContext();
    ((EAGLGraphicsContext*)self->ctx_)->setVersionMajor(2);
    ((EAGLGraphicsContext*)self->ctx_)->setVersionMinor(0);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)setupEnv:(tfw::Descriptor *)des
{
	using namespace tfw;
    [super setupEnv:des];

    Environment &env = des->env();
    const Graphics &graphics = env.graphics();

    const ApiDefinition *actualVersion = nullptr;
    for(size_t versionIndex = 0; versionIndex < graphics.versions().size(); versionIndex++)
    {
        const ApiDefinition *aVersion = &graphics.versions()[versionIndex];

        if(aVersion->type() == ApiDefinition::ES)
        {
            actualVersion = &graphics.versions()[versionIndex];
        }
    }

    if(actualVersion != nullptr)
    {
        [self.glview setEsVersion:*actualVersion];
        [self.glview createNewbuffers:&des->env().graphics()];
        ((EAGLGraphicsContext*)self->ctx_)->setEaglView(self.glview);
    }
    env.setHeight(self.glview.bounds.size.height * self.glview.contentScaleFactor);
    env.setWidth(self.glview.bounds.size.width * self.glview.contentScaleFactor);

}

- (void)dealloc {
    delete ctx_;
    [context_ release];
    [_glview release];
    [super dealloc];
}

- (void)viewDidUnload {
    [self setGlview:nil];
    [super viewDidUnload];
}
@end