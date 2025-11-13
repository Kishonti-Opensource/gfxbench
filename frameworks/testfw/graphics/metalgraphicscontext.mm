/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "metalgraphicscontext.h"
#include "ng/require.h"
#include "ng/log.h"

#if !(TARGET_IPHONE_SIMULATOR)

extern id <MTLDevice> MTLCreateSystemDefaultDevice(void) WEAK_IMPORT_ATTRIBUTE;

MetalGraphicsContextImp::MetalGraphicsContextImp()
: m_Drawable_valid(false)
, m_Drawable(nil)
, m_framebuffer_only(true)
, valid_(false)
, layer(nil)
{

}


MetalGraphicsContextImp::~MetalGraphicsContextImp()
{
    layer = nil;
    destroyContext() ;
}

bool MetalGraphicsContextImp::makeCurrent()
{
    return true;
}


void MetalGraphicsContextImp::getNextDrawable()
{
    @autoreleasepool {
        m_Drawable_valid = true ;
        m_Drawable = [layer nextDrawable];
    }
}

bool MetalGraphicsContextImp::swapBuffers()
{
    @autoreleasepool {
        
        id <MTLCommandBuffer> commandBuffer = [m_MainCommandQueue commandBuffer];
        
        if (!m_Drawable_valid)
        {
            getNextDrawable() ;
        }
        [commandBuffer presentDrawable:m_Drawable];
        [commandBuffer commit];
        
        m_Drawable_valid = false ;
    }
    
    return true;
}


bool MetalGraphicsContextImp::detachThread()
{
    return true;
}


bool MetalGraphicsContextImp::isValid()
{
	return valid_;
}

GraphicsContext::GraphicsType MetalGraphicsContextImp::type()
{
    return METAL;
}

int MetalGraphicsContextImp::versionMajor()
{
    return 1;
}

int MetalGraphicsContextImp::versionMinor()
{
    return 0;
}

id <MTLDevice> MetalGraphicsContextImp::getDevice()
{
    return m_Device ;
}

id <MTLCommandQueue> MetalGraphicsContextImp::getMainCommandQueue()
{
    return m_MainCommandQueue ;
}

id <MTLTexture> MetalGraphicsContextImp::getBackBufferTexture()
{
    if (!m_Drawable_valid)
    {
        getNextDrawable();
    }
    
    return m_Drawable.texture ;
}

void MetalGraphicsContextImp::initContext(const std::string& deviceName)
{
    if (MTLCreateSystemDefaultDevice == nil)
    {
        return;
    }
#if TARGET_OS_IPHONE
    m_Device = MTLCreateSystemDefaultDevice();
#else
    NSArray *devices = MTLCopyAllDevices();
    for ( int i = 0; i < [devices count]; ++i )
    {
        m_Device = devices[i];
        const char *name = [[m_Device name] UTF8String];
        if(deviceName.empty()) //default to high power
        {
            if(!m_Device.isLowPower)
            {
                break;
            }
        }
        else
        {
            if(strcmp(name, deviceName.c_str()) == 0)
            {
                break;
            }
            else
            {
                m_Device = nil;
            }
        }
    }
    
    if(m_Device == nil)
    {
        return;
    }
#endif

    m_MainCommandQueue = [m_Device newCommandQueue];
	valid_ = true;
    m_Drawable_valid = false;
    NGLOG_INFO("Metal context created on device: %s", [[m_Device name] UTF8String]);
    
    initLayer();
}

std::string MetalGraphicsContextImp::getDeviceName(int index)
{
    std::string ret;
    if (MTLCreateSystemDefaultDevice == nil)
    {
        return ret;
    }
#if TARGET_OS_IPHONE
    id<MTLDevice> dev = MTLCreateSystemDefaultDevice();
    ret = std::string([[dev name] UTF8String]);
    @autoreleasepool {
        dev = nil ;
    }
#else
    NSArray *devs = MTLCopyAllDevices();
    if(devs.count > index)
    {
        ret = std::string([[devs[index] name] UTF8String]);
    }
    @autoreleasepool {
        devs = nil ;
    }
#endif
    return ret;
}

void MetalGraphicsContextImp::destroyContext()
{
    @autoreleasepool {
        m_Device = nil ;
        m_MainCommandQueue = nil ;
        m_Drawable = nil ;
    }
}

void MetalGraphicsContextImp::initLayer()
{
    @autoreleasepool {
        
        if(!layer){ NSLog(@">> ERROR: Failed acquring Core Animation Metal layer!"); } // if
        
        layer.presentsWithTransaction = NO;
        layer.drawsAsynchronously     = YES;
        
        layer.device          = m_Device;
        layer.pixelFormat     = MTLPixelFormatBGRA8Unorm;
        
        layer.framebufferOnly = m_framebuffer_only;
        
//        // set a background color to make sure the layer appears
//        CGColorSpaceRef pColorSpace = CGColorSpaceCreateDeviceRGB();
//        
//        if(pColorSpace != NULL)
//        {
//            CGFloat components[4] = {1.0, 0.0, 0.0, 1.0};
//            
//            CGColorRef pGrayColor = CGColorCreate(pColorSpace,components);
//            
//            if(pGrayColor != NULL)
//            {
//                layer.backgroundColor = pGrayColor;
//                
//                CFRelease(pGrayColor);
//            } // if
//            
//            CFRelease(pColorSpace);
//        } // if
        

    }
}


#pragma mark - ios specific functions
bool MetalGraphicsContextImp::setMetalLayer(CAMetalLayer *l)
{
    layer = l;
    
    initLayer() ;
    
    return true;
}

CAMetalLayer* MetalGraphicsContextImp::getMetalLayer()
{
    return layer;
}

bool MetalGraphicsContextImp::isFrameBufferOnly()
{
    return m_framebuffer_only ;
}

void MetalGraphicsContextImp::setFrameBufferOnly(bool framebuffer_only)
{
    m_framebuffer_only = framebuffer_only ;
    
    if (layer != nil)
    {
        layer.framebufferOnly = m_framebuffer_only;
    }
}


#endif
