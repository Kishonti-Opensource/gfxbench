/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPUTE_HDR31_H
#define COMPUTE_HDR31_H

#include <kcl_base.h>

#include "mtl_pipeline.h"
#include "metal/mtl_compute_reduction31.h"
#include "metal/mtl_fragment_blur.h"

class ComputeHDR31
{
public:
    static const KCL::uint32 DOWNSAMPLE = 2;
    static const KCL::uint32 BLUR_TEXTURE_COUNT = 4;

	ComputeHDR31(id <MTLDevice> device);
	virtual ~ComputeHDR31();

    void Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type, KRL_Scene *scene);
	void SetInputTexture(id <MTLTexture> in_texture);

    void Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame);

    id <MTLTexture> GetBloomTexture() const;
    id <MTLSamplerState> GetBloomSampler() const;
    id <MTLBuffer> GetLuminanceBuffer() const;

private:
    void InitBrightPass();
	void SetupBrightTexture();
	void BrightPass(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame);

	KCL::uint32 m_width, m_height;
    
    MTLPixelFormat m_bright_texture_type;

	// textures
    id <MTLTexture> m_input_texture;
    id <MTLTexture> m_bright_texture;

	// samplers
	id <MTLSamplerState> m_input_sampler;
    id <MTLSamplerState> m_bloom_sampler;

	// work group sizes
	KCL::uint32 m_bright_pass_work_group_size;
    KCL::uint32 m_bright_pass_dispatch_count_x;
    KCL::uint32 m_bright_pass_dispatch_count_y;
    
	// compute shader
    MetalRender::Pipeline *m_bright_pass;

    MetalRender::ComputeReduction *m_reduction;
    MetalRender::FragmentBlur *m_fragment_blur;

	void CompileShader();
    
    id <MTLDevice> m_device ;
};

#endif