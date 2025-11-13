/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef METAL_GRAPHICS_CONTEXT_H_
#define METAL_GRAPHICS_CONTEXT_H_

#include "graphics/graphicscontext.h"
#include "schemas/descriptors.h"

#include <TargetConditionals.h>
#if (TARGET_IPHONE_SIMULATOR)

class MetalGraphicsContext : public GraphicsContext
{
public:
	virtual bool makeCurrent() { return false; }
	virtual bool swapBuffers() { return false; }
	virtual bool isValid() { return false; }
	virtual bool detachThread() { return false; }
	
	virtual GraphicsType type() { return GraphicsContext::METAL; }
	virtual int versionMajor() { return 0; }
	virtual int versionMinor() { return 0; }
	virtual bool hasFlag(int flag) { return false; }
};

class MetalGraphicsContextImp : public MetalGraphicsContext
{
};

#else

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>
#include <QuartzCore/CAMetalLayer.h>



class MetalGraphicsContext : public GraphicsContext
{
public:
    virtual ~MetalGraphicsContext() { };
    virtual bool makeCurrent() = 0;
    virtual bool swapBuffers() = 0;
    virtual bool isValid() = 0;
    virtual bool detachThread() = 0;
    
    virtual GraphicsType type() = 0;
    virtual int versionMajor() = 0;
    virtual int versionMinor() = 0;
    virtual bool hasFlag(int flag)  = 0 ;
    
    virtual bool setMetalLayer(CAMetalLayer *layer) = 0;
    virtual CAMetalLayer* getMetalLayer() = 0;
    
    virtual void setFrameBufferOnly(bool framebuffer_only) = 0 ;
    virtual bool isFrameBufferOnly() = 0 ;
    
    virtual id <MTLDevice> getDevice() = 0;
    virtual id <MTLCommandQueue> getMainCommandQueue() = 0 ;
    virtual id <MTLTexture> getBackBufferTexture() = 0;
	
	virtual void initContext(const std::string& deviceName) = 0;
	virtual std::string getDeviceName(int index) = 0;
};


class MetalGraphicsContextImp : public MetalGraphicsContext
{
public:
    MetalGraphicsContextImp();
    virtual ~MetalGraphicsContextImp();
    virtual bool makeCurrent();
    virtual bool swapBuffers();
    virtual bool isValid();
    virtual bool detachThread();
    
    virtual GraphicsType type();
    virtual int versionMajor();
    virtual int versionMinor();
    virtual bool hasFlag(int flag) { return flags_ & flag; }
    
    virtual bool setMetalLayer(CAMetalLayer *layer);
    virtual CAMetalLayer* getMetalLayer();
    
    void setFrameBufferOnly(bool framebuffer_only) ;
    bool isFrameBufferOnly() ;
    
    id <MTLDevice> getDevice() ;
    id <MTLCommandQueue> getMainCommandQueue() ;
    id <MTLTexture> getBackBufferTexture() ;
    
private:
    
    id <MTLDevice>        m_Device;
    id <MTLCommandQueue>  m_MainCommandQueue;
    
    volatile bool                  m_Drawable_valid ;
    volatile id <CAMetalDrawable>  m_Drawable ;
    
    void getNextDrawable() ;
public:
    virtual void initContext(const std::string& deviceName);
    virtual std::string getDeviceName(int index);
private:
    void destroyContext() ;
    void initLayer() ;
    
    bool m_framebuffer_only ;
	
	bool valid_;
    int flags_;
    CAMetalLayer *layer;
};

#endif  // TARGET_OS_SIMULATOR

#endif // METAL_GRAPHICS_CONTEXT_H_
