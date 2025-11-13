/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/graphicscontext.h"
#include <OpenGLES/EAGL.h>
#include <QuartzCore/QuartzCore.h>

#include "ios/TfwEAGLView.h"

#define USE_CADISPLAYLINK 1

class EAGLGraphicsContext : public GraphicsContext
{
public:
	EAGLGraphicsContext();
    virtual ~EAGLGraphicsContext();
    
    virtual bool makeCurrent();
    virtual bool swapBuffers();
    virtual bool isValid();
    virtual bool detachThread();
    virtual GraphicsType type();
    virtual int versionMajor();
    virtual int versionMinor();
    virtual bool hasFlag(int flag) { return flags_ & flag; }
    
    virtual bool setEaglView(TfwEAGLView *v);
    
    void setVersionMajor(int i) { major_ = i; }
    void setVersionMinor(int i) { minor_ = i; }
    
    TfwEAGLView* getView();
    
private:
    
    int major_;
    int minor_;
    int flags_;
    TfwEAGLView *view;
};