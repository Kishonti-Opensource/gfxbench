/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_GBUFFER_H
#define MTL_GBUFFER_H

#include <kcl_base.h>
#include "mtl_quadBuffer.h"
#include "mtl_pipeline.h"

namespace GFXB4
{

class GBuffer
{
    static const KCL::uint32 DEPTH_HIZ_LEVELS = 4;

public:
	static const bool VELOCITY_BUFFER_RGBA8 = false;
	static const bool NORMAL_BUFFER_RGBA8 = false;
	static const bool HIZ_DEPTH_ENABLED = true;


    GBuffer();
    ~GBuffer();

    bool Init(KCL::uint32 width, KCL::uint32 height);

	void DownsampleDepth(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, MetalRender::QuadBuffer * quad_buffer);

	id<MTLRenderCommandEncoder> GetGBufferPassEncoder(id<MTLCommandBuffer> command_buffer)
	{
		id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:m_gbuffer_pass];
		encoder.label = @"G-Buffer";
		[encoder setFrontFacingWinding:MTLWindingCounterClockwise];
		return encoder;
	}
    
    MTLRenderPassDescriptor* GetGBufferPassDescriptor()
    {
        return m_gbuffer_pass;
    }

	id<MTLRenderCommandEncoder> GetTransparentAccumulationPassEncoder(id<MTLCommandBuffer> command_buffer)
	{
		id<MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:m_transparent_accum_pass];
		encoder.label = @"Tranparent Accumulation";
		[encoder setFrontFacingWinding:MTLWindingCounterClockwise];		
		return encoder;
	}
    
    MTLRenderPassDescriptor* GetTransparentAccumulationPassDescriptor()
    {
        return m_transparent_accum_pass;
    }

	id<MTLTexture> GetAlbedoTexture() { return m_albedo_texture; }
	id<MTLTexture> GetDepthTexture() { return m_depth_texture; }
	id<MTLTexture> GetVelocityTexture() { return m_velocity_texture; }
	id<MTLTexture> GetNormalTexture() { return m_normal_texture; }
	id<MTLTexture> GetParamsTexture() { return m_params_texture; }
	id<MTLTexture> GetTransparentAccumTexture() { return m_transparent_accum_texture; }
	id<MTLTexture> GetDepthHiZTexture() { return m_depth_hiz_texture; }

	id<MTLSamplerState> GetNearestSampler() { return m_nearest_sampler; }
	id<MTLSamplerState> GetLinearMipmapSampler() { return m_linear_mipmap_sampler; }

	KCL::uint32 GetHiZDepthLevels() const { return m_hiz_depth_levels; }
    
    inline KCL::Vector2D GetViewport() const { return KCL::Vector2D(m_viewport_width, m_viewport_height); }

private:
	bool InitShaders();

	id<MTLDevice> m_device;

	id<MTLTexture> m_albedo_texture;
	id<MTLTexture> m_depth_texture;
	id<MTLTexture> m_velocity_texture;
	id<MTLTexture> m_normal_texture;
	id<MTLTexture> m_params_texture;
	id<MTLTexture> m_transparent_accum_texture;
	id<MTLTexture> m_depth_hiz_texture;

	id<MTLSamplerState> m_nearest_sampler;
	id<MTLSamplerState> m_linear_mipmap_sampler;

	MTLRenderPassDescriptor* m_gbuffer_pass;
	MTLRenderPassDescriptor* m_transparent_accum_pass;

    KCL::uint32 m_viewport_width;
    KCL::uint32 m_viewport_height;

    KCL::uint32 m_hiz_depth_levels;
	MetalRender::Pipeline * m_linearize_shader;
	MetalRender::Pipeline * m_downsample_shader;
    KCL::int32 m_texture_size_pos;
#endif
};

}

