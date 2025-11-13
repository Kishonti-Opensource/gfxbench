/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "eaglgraphicscontext.h"
#include "ng/require.h"


EAGLGraphicsContext::EAGLGraphicsContext() : major_(0), minor_(0), view(nil)
{
}


EAGLGraphicsContext::~EAGLGraphicsContext()
{
    view = nil;
}

bool EAGLGraphicsContext::makeCurrent()
{
    if (view == nil)
        return false;
    
    require([view getContext]);
    
    bool ok = [EAGLContext setCurrentContext:[view getContext]];
    //NSLog(@"context made curent: %d", ok);
    
    return ok;
}


bool EAGLGraphicsContext::swapBuffers()
{
    if (view == nil)
        return false;
    
    return [view swapBuffers];
}


bool EAGLGraphicsContext::detachThread()
{
    return [EAGLContext setCurrentContext:nil];
}


bool EAGLGraphicsContext::isValid()
{
    return [view getContext] != nil;
}

GraphicsContext::GraphicsType EAGLGraphicsContext::type()
{
    return GLES;
}

int EAGLGraphicsContext::versionMajor()
{
    return major_;
}

int EAGLGraphicsContext::versionMinor()
{
    return minor_;
}


#pragma mark - ios specific functions

bool EAGLGraphicsContext::setEaglView(TfwEAGLView *v)
{
    view = v;
    return true;
}

TfwEAGLView* EAGLGraphicsContext::getView()
{
    return view;
}