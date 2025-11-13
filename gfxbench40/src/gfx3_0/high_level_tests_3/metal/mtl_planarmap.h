/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_PLANARMAP_H
#define MTL_PLANARMAP_H

#include <kcl_base.h>
#include <kcl_planarmap.h>

#include <vector>
#include <float.h>

#include <kcl_os.h>

#import <Metal/Metal.h>

namespace MetalRender
{
    
	class PlanarMap : public KCL::PlanarMap
	{
	public:
#if TARGET_OS_EMBEDDED
        static const MTLPixelFormat PLANAR_FRAME_BUFFER_FORMAT = MTLPixelFormatB5G6R5Unorm ;
#else
        static const MTLPixelFormat PLANAR_FRAME_BUFFER_FORMAT = MTLPixelFormatRGBA8Unorm;
#endif
        
        friend class KCL::PlanarMap ;
        
		virtual ~PlanarMap();

		void Bind();	
		void Unbind();
        
        
        inline id <MTLRenderCommandEncoder> SetAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
        {
            return [commandBuffer renderCommandEncoderWithDescriptor:m_framebuffer];
        }
        
        
        inline void SetAsTexture(id <MTLRenderCommandEncoder> renderEncoder, unsigned char slot)
        {
            [renderEncoder setFragmentSamplerState:m_sampler atIndex:slot];
            [renderEncoder setFragmentTexture:m_colorTexture atIndex:slot];
        }
        
	protected:
		PlanarMap( int w, int h, const char *name);
        
        id <MTLTexture> m_depthTexture;
        id <MTLTexture> m_colorTexture;
        id <MTLSamplerState> m_sampler;
        MTLRenderPassDescriptor* m_framebuffer;
        
        id <MTLDevice> m_Device ;
	};
}

#endif // MTL_PLANARMAP_H
