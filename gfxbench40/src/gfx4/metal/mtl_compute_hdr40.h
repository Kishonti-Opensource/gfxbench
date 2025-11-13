/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPUTE_HDR40_H
#define COMPUTE_HDR40_H

#include <kcl_base.h>

#include "mtl_pipeline.h"
#include "mtl_compute_reduction40.h"
#include "metal/mtl_fragment_blur.h"


class BrightPass
{
public:
	BrightPass() {}
	virtual ~BrightPass() {}
	
	virtual void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type) = 0;
	virtual void Execute(id <MTLCommandBuffer> command_buffer, id <MTLTexture> m_input_texture, id <MTLSamplerState> m_input_sampler,
						 id <MTLTexture> m_output_texture, id<MTLBuffer> luminance_buffer, const UBOFrame &ubo_frame) = 0;
};


class ComputeHDR40
{
public:
    static const KCL::uint32 DOWNSAMPLE = 2;
    static const KCL::uint32 BLUR_TEXTURE_COUNT = 4;

	ComputeHDR40(id <MTLDevice> device);
	virtual ~ComputeHDR40();

    void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type, bool use_compute_bright_pass, KRL_Scene *scene);
	void SetInputTexture(id <MTLTexture> in_texture);

    void Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame);

    id <MTLTexture> GetBloomTexture() const;
    id <MTLSamplerState> GetBloomSampler() const;
    id <MTLBuffer> GetLuminanceBuffer() const;
	MetalRender::ComputeReduction40 * GetComputeReduction() const;

private:
	void SetupBloomTexture();

	KCL::uint32 m_width, m_height;
    
    MTLPixelFormat m_bloom_texture_type;
	bool m_compute_bright_pass;

	// textures
    id <MTLTexture> m_input_texture;
    id <MTLTexture> m_bloom_texture;

	// samplers
	id <MTLSamplerState> m_input_sampler;
    id <MTLSamplerState> m_bloom_sampler;

	BrightPass *m_bright_pass;
    MetalRender::ComputeReduction40 *m_reduction;
    MetalRender::FragmentBlur *m_fragment_blur;

	void CompileShader();
    
    id <MTLDevice> m_device ;
};


class ComputeBrightPass : public BrightPass
{
public:
	ComputeBrightPass();
	virtual ~ComputeBrightPass();
	virtual void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type);
	virtual void Execute(id <MTLCommandBuffer> command_buffer, id <MTLTexture> m_input_texture, id <MTLSamplerState> m_input_sampler,
						 id <MTLTexture> m_output_texture, id<MTLBuffer> luminance_buffer, const UBOFrame &ubo_frame);
	
private:
	KCL::uint32 m_bloom_texture_gl_type;
	
	// work group sizes
	KCL::uint32 m_work_group_size_x;
	KCL::uint32 m_work_group_size_y;
	KCL::uint32 m_dispatch_count_x;
	KCL::uint32 m_dispatch_count_y;
	
	// compute shader
	MetalRender::Pipeline *m_bright_pass;
	
	void SetWorkGroupSize();
};


class FragmentBrightPass : public BrightPass
{
public:
	FragmentBrightPass();
	virtual ~FragmentBrightPass();
	virtual void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type);
	virtual void Execute(id <MTLCommandBuffer> command_buffer, id <MTLTexture> m_input_texture, id <MTLSamplerState> m_input_sampler,
						 id <MTLTexture> m_output_texture, id<MTLBuffer> luminance_buffer, const UBOFrame &ubo_frame);
	
private:
	MTLRenderPassDescriptor * m_bright_pass_desc;
	MetalRender::Pipeline *m_pipeline;
	
	MetalRender::QuadBuffer *m_quad_buffer;
};


#endif  // COMPUTE_HDR40_H

