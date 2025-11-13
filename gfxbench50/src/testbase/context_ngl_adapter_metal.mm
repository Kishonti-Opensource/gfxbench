/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "context_ngl_adapter_metal.h"

#include "graphics/metalgraphicscontext.h"
#include "metal/ngl_metal_adapter_interface.h"


class NGL_metal_adapter : public NGL_metal_adapter_interface
{
public:
    NGL_metal_adapter(GraphicsContext *ctx);
    virtual id <MTLTexture> GetBackBuffer() ;
    virtual MTLPixelFormat GetBackBufferPixelFormat();

private:
    MetalGraphicsContext * mtl_context ;
};


NGL_metal_adapter::NGL_metal_adapter(GraphicsContext *ctx)
{
    mtl_context = dynamic_cast<MetalGraphicsContext*>(ctx) ;
}


id <MTLTexture> NGL_metal_adapter::GetBackBuffer()
{
    return mtl_context->getBackBufferTexture();
}


NGL_metal_adapter* GetNGLMetalAdapter(GraphicsContext* context, const std::string &device_id, bool screenshot_mode, bool macos_use_subpass)
{
    @autoreleasepool {
        MetalGraphicsContext* mtl_context = dynamic_cast<MetalGraphicsContext*>(context) ;
		mtl_context->initContext(device_id);
        
        if (screenshot_mode)
        {
            mtl_context->setFrameBufferOnly(false);
        }
        
        NGL_metal_adapter* metal_adapter = new NGL_metal_adapter(context);
        metal_adapter->device = mtl_context->getDevice();
        metal_adapter->commandQueue = mtl_context->getMainCommandQueue();
        metal_adapter->macos_use_subpass = macos_use_subpass;
        
		printf("ngl-metal adapter device: %s\n", [[metal_adapter->device name] UTF8String]);
		
        return metal_adapter;
    }
}


MTLPixelFormat NGL_metal_adapter::GetBackBufferPixelFormat()
{
    return mtl_context->getMetalLayer().pixelFormat;
}

