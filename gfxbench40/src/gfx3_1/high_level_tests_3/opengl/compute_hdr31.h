/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPUTE_HDR31_H
#define COMPUTE_HDR31_H

#include <kcl_base.h>

#include "opengl/glb_shader2.h"
#include "opengl/compute_reduction31.h"
#include "opengl/fragment_blur.h"

class ComputeHDR31
{
public:
    static const KCL::uint32 DOWNSAMPLE = 2;
    static const KCL::uint32 BLUR_TEXTURE_COUNT = 4;

	ComputeHDR31();
	virtual ~ComputeHDR31();

    void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo, GLB_Scene_ES2_ *scene);
	void SetInputTexture(KCL::uint32 in_texture);

    void Execute();	

    KCL::uint32 GetBloomTexture() const;
    KCL::uint32 GetBloomSampler() const;
    KCL::uint32 GetLuminanceBuffer() const;

private:
    void InitBrightPass();
	void SetupBrightTexture();
	void BrightPass();

	KCL::uint32 m_width, m_height;
    
    KCL::uint32 m_bright_texture_type;

	// textures
	KCL::uint32 m_input_texture;
    KCL::uint32 m_bright_texture;

	// samplers
	KCL::uint32 m_input_sampler;
    KCL::uint32 m_bloom_sampler;

	// work group sizes
	KCL::uint32 m_bright_pass_work_group_size;
    KCL::uint32 m_bright_pass_dispatch_count_x;
    KCL::uint32 m_bright_pass_dispatch_count_y;
    
	// compute shader
	GLB::GLBShader2 *m_bright_pass;

	// binds
	KCL::uint32 m_in_texture_bind;
	KCL::uint32 m_out_level0_bind;

    GLB::ComputeReduction *m_reduction;
    GLB::FragmentBlur *m_fragment_blur;

	void CompileShader();
};

#endif