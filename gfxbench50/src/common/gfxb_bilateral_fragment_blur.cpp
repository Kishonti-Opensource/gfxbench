/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if 0

#include "ngl.h"
#include "gfxb_shader.h"
#include "common/gfxb_bilateral_fragment_blur.h"
#include "common/gfxb_gauss_blur_helper.h"
#include "common/gfxb_shapes.h"

#include <sstream>
#include <algorithm>

using namespace GFXB;

void BilateralFragmentBlur::Init(Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 depth_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name)
{
	assert( !(lod_levels == 0));
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_texture_format = texture_format;
	m_lod_levels = lod_levels;
	m_input_texture = input_texture;
	m_depth_texture = depth_texture;
	m_name = name;

	std::string fragment_shader;

	if (m_lod_levels > 1)
	{
		fragment_shader = "bilateral_gauss_blur_fs_lod.frag";
	}
	else
	{
		fragment_shader = "bilateral_gauss_blur_fs.frag";
	}

	SetKernelSize(blur_kernel_size);

	GenerateVertical(fragment_shader.c_str(), blur_kernel_size);
	GenerateHorizontal(fragment_shader.c_str(), blur_kernel_size);
	ResizeJobs();
}

void BilateralFragmentBlur::SetKernelSize( KCL::uint32 blur_kernel_size )
{
	m_blur_kernel_size = KCL::Max( blur_kernel_size, 2u );

	std::vector<KCL::Vector4D> gauss_weights;

	// Generate the shader constants
	//Don't normalize the weights, as not every one of them will be utilized (bc. of bilateral)
	gauss_weights = GaussBlurHelper::GetGaussWeights( m_blur_kernel_size, false );
	packed_weights = GaussBlurHelper::CalcPackedWeights( gauss_weights );

	if( m_lod_levels > 1 )
	{
		horizontal_packed_offsets = GaussBlurHelper::CalcPackedOffsets( 1, m_blur_kernel_size, gauss_weights );
		vertical_packed_offsets = horizontal_packed_offsets;
	}
	else
	{
		vertical_packed_offsets = GaussBlurHelper::CalcPackedOffsets( m_height, m_blur_kernel_size, gauss_weights );
		horizontal_packed_offsets = GaussBlurHelper::CalcPackedOffsets( m_width, m_blur_kernel_size, gauss_weights );
	}

	packed_weights.resize( MAX_KS );
	vertical_packed_offsets.resize( MAX_KS );
	horizontal_packed_offsets.resize( MAX_KS );
}

KCL::uint32 BilateralFragmentBlur::RenderVerticalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level )
{
	void *p[UNIFORM_MAX];
	p[UNIFORM_TEXTURE_UNIT0] = &m_input_texture;
	p[UNIFORM_TEXTURE_UNIT7] = &m_depth_texture;
	p[UNIFORM_DEPTH_PARAMETERS] = depth_parameters.v;
	p[UNIFORM_GAUSS_WEIGHTS] = packed_weights.data();
	p[UNIFORM_GAUSS_OFFSETS] = vertical_packed_offsets.data();
	uint32_t t = m_blur_kernel_size + 1;
	p[UNIFORM_GAUSS_KERNEL_SIZE] = &t;

	if( m_lod_levels > 1 )
	{
		p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
		p[UNIFORM_INV_RESOLUTION] = m_stepuv[lod_level].v;
	}

	nglBegin( m_blur_job_v[lod_level] );
	nglDrawTwoSided( m_blur_job_v[lod_level], m_blur_shader_v, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
	nglEnd( m_blur_job_v[lod_level] );

	return m_blur_job_v[lod_level];
}


KCL::uint32 BilateralFragmentBlur::RenderHorizontalPass( KCL::Vector4D depth_parameters, KCL::uint32 lod_level )
{
	void *p[UNIFORM_MAX];
	p[UNIFORM_TEXTURE_UNIT0] = &m_blur_texture_v;
	p[UNIFORM_TEXTURE_UNIT7] = &m_depth_texture;
	p[UNIFORM_DEPTH_PARAMETERS] = depth_parameters.v;
	p[UNIFORM_GAUSS_WEIGHTS] = packed_weights.data();
	p[UNIFORM_GAUSS_OFFSETS] = horizontal_packed_offsets.data();
	uint32_t t = m_blur_kernel_size + 1;
	p[UNIFORM_GAUSS_KERNEL_SIZE] = &t;

	if( m_lod_levels > 1 )
	{
		p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
		p[UNIFORM_INV_RESOLUTION] = m_stepuv[lod_level].v;
	}

	nglBegin( m_blur_job_h[lod_level] );
	nglDrawTwoSided( m_blur_job_h[lod_level], m_blur_shader_h, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
	nglEnd( m_blur_job_h[lod_level] );

	return m_blur_job_h[lod_level];
}

#endif