/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#import "TestContextProvider.h"
#import "ios/TfwEAGLView.h"
#import "graphics/eaglgraphicscontext.h"
#import "Common/NUIAppData.h"

#import "graphics/metalgraphicscontext.h"

#include <TargetConditionals.h>

#if !(TARGET_IPHONE_SIMULATOR)
#import <QuartzCore/CAMetalLayer.h>

@implementation METLView

+ (Class)layerClass
{
    return [CAMetalLayer class];
}

@end

#endif

@implementation TestContextProvider
{

@private GraphicsContext *currentContext;
@private UIView *currentView;
}

- (void) setVersion:(tfw::Graphics *)graphics withFrame:(CGRect)frame
{
    const tfw::ApiDefinition *actualVersion = nullptr;
    for(int versionIndex = 0; versionIndex < graphics->versions().size(); versionIndex++)
    {
        const tfw::ApiDefinition *aVersion = &(graphics->versions()[versionIndex]);

        if(actualVersion == nullptr &&
           (aVersion->type() == tfw::ApiDefinition::METAL || aVersion->type() == tfw::ApiDefinition::ES))
        {
            actualVersion = &(graphics->versions()[versionIndex]);
        }
    }


    if(currentView != NULL){
        [currentView removeFromSuperview];
        [currentView release];
    }
    if(currentContext != NULL) {
        delete currentContext;
        currentContext = nil;
    }

    if(actualVersion->type() == tfw::ApiDefinition::ES)
    {
        currentView = [[TfwEAGLView alloc] initWithFrame:frame esVersion:*actualVersion];

        currentContext = new EAGLGraphicsContext();
        ((EAGLGraphicsContext *)currentContext)->setVersionMajor(actualVersion->major());
        ((EAGLGraphicsContext *)currentContext)->setVersionMinor(actualVersion->minor());

        [((TfwEAGLView *)currentView) createNewbuffers:graphics];
        ((EAGLGraphicsContext *)currentContext)->setEaglView((TfwEAGLView *)currentView);
    }
    else if (actualVersion->type() == tfw::ApiDefinition::METAL)
    {
		// the graphics context should be initialized in testbase by device name
        currentContext = new MetalGraphicsContextImp();
#if !(TARGET_IPHONE_SIMULATOR)
        currentView = [[METLView alloc] initWithFrame:frame];

        if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
        {
            currentView.contentScaleFactor = [[UIScreen mainScreen] nativeScale];
        }
        else if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
        {
            currentView.contentScaleFactor = [[UIScreen mainScreen] scale];
        }
        ((MetalGraphicsContext *)currentContext)->setMetalLayer((CAMetalLayer*)currentView.layer);
#endif
    }
}

- (GraphicsContext *) getGraphicsContext
{
    return currentContext;
}

- (UIView *) getTestView
{
    return currentView;
}

- (void) dealloc
{
    if(currentView != nil) {
        [currentView release];
    }
	[super dealloc];
}

@end
