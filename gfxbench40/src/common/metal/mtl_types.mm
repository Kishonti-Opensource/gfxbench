/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_types.h"
//#include "mtl_factories.h"
//#include "mtl_image.h"

#import "graphics/metalgraphicscontext.h"

MetalGraphicsContext* g_MetalGraphicsContext = nullptr;

void MetalRender::InitMetalGraphicsContext(GraphicsContext *ctx, const std::string device_id)
{
	MetalGraphicsContext *mtl_ctx = dynamic_cast<MetalGraphicsContext*>(ctx);
	mtl_ctx->initContext(device_id);
}

void MetalRender::Initialize(const GlobalTestEnvironment* const gte)
{
	Release();
 
    g_MetalGraphicsContext = dynamic_cast<MetalGraphicsContext*>(gte->GetGraphicsContext()) ;
    
    if ( !(gte->GetTestDescriptor()->m_screenshot_frames.empty()) )
    {
        g_MetalGraphicsContext->setFrameBufferOnly(false) ;
    }

    KCL::Initialize(true);
}

void MetalRender::Release()
{
    g_MetalGraphicsContext = nullptr ;
}

MetalGraphicsContext* MetalRender::GetContext()
{
    return g_MetalGraphicsContext ;
}


const char* MetalRender::GetDeviceName()
{
    return [g_MetalGraphicsContext->getDevice().name UTF8String] ;
}


void MetalRender::Finish()
{
    @autoreleasepool {
        if(g_MetalGraphicsContext) {
            id <MTLCommandQueue> commandQueue = g_MetalGraphicsContext->getMainCommandQueue() ;
            id <MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer] ;
            [commandBuffer commit];
            [commandBuffer waitUntilCompleted];
            commandBuffer = nil;
        }
    }
}


bool MetalRender::isASTCSupported()
{
#if TARGET_OS_IPHONE
    return [g_MetalGraphicsContext->getDevice() supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1] ;
#else
    return false;
#endif
}


