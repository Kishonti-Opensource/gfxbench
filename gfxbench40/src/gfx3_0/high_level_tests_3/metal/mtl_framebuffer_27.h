/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_FRAMEBUFFER_27
#define MTL_FRAMEBUFFER_27

#include "kcl_base.h"
#include <Metal/Metal.h>

namespace MetalRender
{
    
    class Framebuffer27
    {
    public:
        
        static const MTLPixelFormat MAIN_FRAME_BUFFER_FORMAT = MTLPixelFormatRGBA8Unorm ;
        
        Framebuffer27(KCL::uint32 width, KCL::uint32 height, bool motion_blur_enabled) ;
        ~Framebuffer27() ;
        
        id <MTLRenderCommandEncoder> SetFinalBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer, id <MTLTexture> finalColorBuffer) ;
        
        id <MTLRenderCommandEncoder> SetMainBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer) ;
        
        id <MTLRenderCommandEncoder> SetVelocityBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer) ;
        
        inline void SetMainColorBufferForMotionBlur(id <MTLRenderCommandEncoder> renderEncoder, int slot)
        {
            [renderEncoder setFragmentTexture:m_mainColorBuffer atIndex:slot] ;
        }
        
        inline void SetVelocityBufferForMotionBlur(id <MTLRenderCommandEncoder> renderEncoder, int slot)
        {
            [renderEncoder setFragmentTexture:m_velocityColorBuffer atIndex:slot] ;
        }
        
    protected:
        
        id <MTLDevice> m_Device ;
        
        KCL::uint32 m_width ;
        KCL::uint32 m_height ;
        
        MTLRenderPassDescriptor* m_finalFrameBuffer;
        MTLRenderPassDescriptor* m_mainBuffer;
        MTLRenderPassDescriptor* m_velocityBuffer;
        
        id<MTLTexture> m_mainDepthBuffer;
        id<MTLTexture> m_velocityDepthBuffer;
        
        id<MTLTexture> m_mainColorBuffer;
        id<MTLTexture> m_velocityColorBuffer;
    };
    
    
}



#endif