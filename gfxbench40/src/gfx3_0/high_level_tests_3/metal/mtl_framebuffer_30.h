/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_FRAMEBUFFER
#define MTL_FRAMEBUFFER

#include "mtl_shader_constant_layouts_30.h"
#include "mtl_quadBuffer.h"
#include "mtl_lensflare_30.h"
#include "mtl_light.h"
#include <Metal/Metal.h>
#include <vector>

namespace MetalRender
{

class Framebuffer
{
public:
	
    explicit Framebuffer(id<MTLTexture> finalColorBuffer,
                   NSUInteger width,
                   NSUInteger height,
                   NSUInteger sampleCount,
                   MTLPixelFormat main_buffer_format,
                   KCL::uint32 scene_version);
    virtual ~Framebuffer();

    inline id <MTLRenderCommandEncoder> SetGBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
    {
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_GBuffer];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        return renderEncoder;
    }

    inline id <MTLRenderCommandEncoder> SetLightSkyBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
    {
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_lightSkyFrameBuffer];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        return renderEncoder;
    }

    inline id <MTLRenderCommandEncoder> SetMainBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
    {
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_mainFrameBuffer];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        return renderEncoder;
    }
    
    
    inline id <MTLRenderCommandEncoder> SetOnScreenMainBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer, id <MTLTexture> backBuffer)
    {
        m_mainFrameBuffer.colorAttachments[0].texture = backBuffer ;
        
        return SetMainBufferAsTargetAndGetEncoder(commandBuffer) ;
    }
    
    
    inline id <MTLRenderCommandEncoder> SetFinalBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer)
    {
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_finalFrameBuffer];

        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        return renderEncoder;
    }
	

    void CreateOcclusionQueryFrameBuffer(OcclusionQueryBuffer* occlusionQueries, NSUInteger queryCount);
    
    inline id <MTLRenderCommandEncoder> SetOcclusionQueryBufferAndGetEncoder(id <MTLCommandBuffer> commandBuffer, NSUInteger index)
    {
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_occlusionQueryFramebuffer[index]];
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
        return renderEncoder;
    }

    typedef enum {
        HORIZONTAL,
        VERTICAL,
        NUM_BLOOM_DIRECTIONS
    } BloomDirection;

    inline id <MTLRenderCommandEncoder> SetBloomBufferAsTargetAndGetEncoder(id <MTLCommandBuffer> commandBuffer, uint32_t level, BloomDirection dir)
    {
        assert(dir < NUM_BLOOM_DIRECTIONS);

        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:m_bloomLevels[level].framebuffer[dir]];

        return renderEncoder;
    }

    inline void SetBloomBufferAsTexture(id <MTLRenderCommandEncoder> renderEncoder, uint32_t level, BloomDirection dir, uint32_t slot)
    {
		//TODO linear or point sampler here?
        [renderEncoder setFragmentTexture:m_bloomLevels[level].texture[dir] atIndex:slot];
		[renderEncoder setFragmentSamplerState:m_linearSampler atIndex:slot];
    }

    inline uint32_t GetBloomBufferWidth(uint32_t level)
    {
        return (uint32_t)[m_bloomLevels[level].texture[0] width];
    }

    inline uint32_t GetBloomBufferHeight(uint32_t level)
    {
        return (uint32_t)[m_bloomLevels[level].texture[0] height];
    }

    inline uint32_t GetNumBloomLevels()
    {
        return (uint32_t)m_bloomLevels.size();
    }

    inline void SetRenderedTexturesForLightPass(id <MTLRenderCommandEncoder> renderEncoder)
    {
        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kLightPassColorMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_COLOR_MAP] atIndex:kLightPassColorMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kLightPassNormalMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_NORMAL_MAP] atIndex:kLightPassNormalMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kLightPassDepthMapSlot];
        [renderEncoder setFragmentTexture:m_depthBuffer atIndex:kLightPassDepthMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kLightPassParamMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_PARAM_MAP] atIndex:kLightPassParamMapSlot];
    }

    inline void SetRenderedTexturesForReflectionEmissionFilter(id <MTLRenderCommandEncoder> renderEncoder)
    {
        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kReflectionEmissionFilterColorMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_COLOR_MAP] atIndex:kReflectionEmissionFilterColorMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kReflectionEmissionFilterNormalMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_NORMAL_MAP] atIndex:kReflectionEmissionFilterNormalMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kReflectionEmissionFilterLightSkyMapSlot];
        [renderEncoder setFragmentTexture:m_lightSkyColorBuffer atIndex:kReflectionEmissionFilterLightSkyMapSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kReflectionEmissionFilterReflectionMapSlot];
        [renderEncoder setFragmentTexture:m_drawBuffer[GBUFFER_REFLECT_MAP] atIndex:kReflectionEmissionFilterReflectionMapSlot];
    }

    inline void SetRenderedTexturesForDepthOfFieldFilter(id <MTLRenderCommandEncoder> renderEncoder)
    {
        id<MTLTexture> blurBuffer = m_bloomLevels[m_bloomLevels.size()-1].texture[1];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kDepthOfFieldFilterBlurBufferSlot];
        [renderEncoder setFragmentTexture:blurBuffer atIndex:kDepthOfFieldFilterBlurBufferSlot];

        [renderEncoder setFragmentSamplerState:m_mipLinearSampler atIndex:kDepthOfFieldFilterMainBufferSlot];
        [renderEncoder setFragmentTexture:m_mainColorBuffer atIndex:kDepthOfFieldFilterMainBufferSlot];

        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:kDepthOfFieldFilterDepthBufferSlot];
        [renderEncoder setFragmentTexture:m_depthBuffer atIndex:kDepthOfFieldFilterDepthBufferSlot];
    }

    inline void SetDepthAsFragmentTexture(id <MTLRenderCommandEncoder> renderEncoder, char slot)
    {
        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:slot];
        [renderEncoder setFragmentTexture:m_depthBuffer atIndex:slot];
    }

    inline void SetRenderedTexturesForSubFilter(id <MTLRenderCommandEncoder> renderEncoder)
    {
        [renderEncoder setFragmentSamplerState:m_linearSampler atIndex:0];
        [renderEncoder setFragmentTexture:m_mainColorBuffer atIndex:0];
    }

    inline void GenerateMipmapsForMainBuffer(id <MTLCommandBuffer> commandBuffer)
    {
        id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];

        [blitEncoder generateMipmapsForTexture:m_mainColorBuffer];

        [blitEncoder endEncoding];
    }

    NSUInteger GetWidth() const
    {
        return m_width;
    }

    NSUInteger GetHeight() const
    {
        return m_height;
    }

    id<MTLTexture> GetDepthBuffer() const
    {
        return m_depthBuffer;
    }

    void UpdateFinalFrameBufferColorBuffer(id <MTLTexture> finalColorBuffer);
	
	id <MTLTexture> GetMainColorBuffer() const
	{
		return m_mainColorBuffer;
	}
	
	id <MTLTexture> GetGBufferAlbedoBuffer() const
	{
		return m_drawBuffer[0];
	}

    void SetupBlitObjects();

    void BlitGBuffer(id <MTLCommandBuffer> commandBuffer);

    void BlitMainBuffer(id <MTLCommandBuffer> commandBuffer);
protected:
    NSUInteger m_width;
    NSUInteger m_height;

    id<MTLTexture> m_depthBuffer;

    enum
    {
        GBUFFER_COLOR_MAP,
        GBUFFER_NORMAL_MAP,
        GBUFFER_REFLECT_MAP,
        GBUFFER_PARAM_MAP,
        NUM_GBUFFER_COLOR_BUFFERS
    };


    id<MTLTexture> m_drawBuffer[NUM_GBUFFER_COLOR_BUFFERS];

    id<MTLTexture> m_lightSkyColorBuffer;
    id<MTLTexture> m_mainColorBuffer;
    id<MTLTexture> m_finalColorBuffer;  // Managed by OS
	
	id <MTLTexture> m_dummyTexture; //for occlusion query rendering

    MTLRenderPassDescriptor* m_GBuffer;
    MTLRenderPassDescriptor* m_lightSkyFrameBuffer;
    MTLRenderPassDescriptor* m_mainFrameBuffer;
    MTLRenderPassDescriptor* m_finalFrameBuffer;

    MTLRenderPassDescriptor* m_occlusionQueryFramebuffer[MetalRender::Light::QUERY_COUNT];

    id <MTLSamplerState> m_linearSampler;
    id <MTLSamplerState> m_mipLinearSampler;

    typedef struct {
        id <MTLTexture> texture[NUM_BLOOM_DIRECTIONS];
        MTLRenderPassDescriptor* framebuffer[NUM_BLOOM_DIRECTIONS];
    } BloomLevel;

    std::vector<BloomLevel> m_bloomLevels;

    QuadBuffer* m_quadBuffer;

    id <MTLBuffer> m_blitBuf;
    id <MTLFunction> m_blitGbufferVS;
    id <MTLFunction> m_blitGbufferFS;
	id <MTLLibrary> m_blitLib;
    id <MTLRenderPipelineState> m_blitPipeline;
    
    id <MTLDevice> m_Device ;
};
	
void SaveNormalizedDepthImage(id<MTLTexture> depthTex, NSString* imageName);
void SaveDepthImage(id<MTLTexture> depthTex, NSString* imageName);
	
}

#endif // MTL_FRAMEBUFFER