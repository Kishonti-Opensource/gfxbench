/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBO_METAL_H
#define FBO_METAL_H

#include "fbo.h"

#import <Metal/Metal.h>

namespace GLB
{

	class Texture2D;

    class FBOMetalBase : public FBO
    {
	public:
		~FBOMetalBase();
        
        virtual id <MTLTexture> GetTexture() = 0 ;
        
        void AppendDepthBuffer(FBO_DEPTHMODE depth_mode) ;
        
        virtual id <MTLRenderCommandEncoder> SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer) = 0 ;
        
        
        // MTL_TODO: Create samplers
        inline void SetDepthBuffer(id <MTLRenderCommandEncoder> renderEncoder, int slot)
        {
            [renderEncoder setFragmentTexture:m_DepthTexture atIndex:slot] ;
        }
        
        
        // MTL_TODO: Create setter/getter methods
        inline MTLRenderPassDescriptor* getRenderPassDescriptor()
        {
            return m_FrameBufferDescriptor ;
        }

		
	protected:
        FBOMetalBase(const GlobalTestEnvironment* const gte);
        
        id <MTLDevice> m_Device ;
        MTLRenderPassDescriptor* m_FrameBufferDescriptor;

	private:
        
        id <MTLTexture> m_DepthTexture ;
	};
    
    
    
    class FBOMetal : public FBOMetalBase
    {
    public:
        FBOMetal(const GlobalTestEnvironment* const gte, KCL::uint32 width, KCL::uint32 height, int samples, FBO_COLORMODE color_mode, FBO_DEPTHMODE depth_mode, const char *debug_label);
        virtual ~FBOMetal() ;
        
        virtual id <MTLTexture> GetTexture() ;
        
        virtual id <MTLRenderCommandEncoder> SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer) ;
        
        
        // MTL_TODO: Create samplers
        inline void SetColorBuffer(id <MTLRenderCommandEncoder> renderEncoder, int slot)
        {
            [renderEncoder setFragmentTexture:m_ColorTexture atIndex:slot] ;
        }
        
    private:
        
        id <MTLTexture> m_ColorTexture ;
        
    };

}

#endif // FBO_METAL_H
