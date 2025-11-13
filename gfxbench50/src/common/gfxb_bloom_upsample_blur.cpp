/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "gfxb_shader.h"
#include "gfxb_barrier.h"
#include "common/gfxb_bloom_upsample_blur.h"
#include "common/gfxb_gauss_blur_helper.h"
#include "common/gfxb_shapes.h"

#include <sstream>
#include <algorithm>

using namespace GFXB;

void BloomUpsampleBlur::Init( Shapes *shapes, KCL::uint32 input_texture0, KCL::uint32 input_texture1, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name )
{
	assert( !( lod_levels == 0 ) );
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_texture_format = texture_format;
	m_lod_levels = lod_levels;
	m_input_texture0 = input_texture0;
	m_input_texture1 = input_texture1;
	m_name = name;
	m_blur_texture = 0;

	std::string fragment_shader;

	fragment_shader = "bloom_upsample_blur.frag";

	SetKernelSize( blur_kernel_size );

	GenerateBlur( fragment_shader.c_str(), blur_kernel_size );
	ResizeJobs();
}

void BloomUpsampleBlur::SetKernelSize( KCL::uint32 blur_kernel_size )
{
	m_blur_kernel_size = KCL::Max( blur_kernel_size, 2u );

	m_weight = 1.0f / blur_kernel_size * 0.5f;
	m_packed_offsets.push_back(KCL::Vector2D( 0.0f, 0.0f));

	for( unsigned c = 0; c < blur_kernel_size - 1; ++c )
	{
		float start = 2.0f / ( blur_kernel_size - 1 );
		float scale = 0.66f * 1.0f * 1.0f;

		KCL::Vector2D pos = circle( start, float(blur_kernel_size - 1), float(c) );

		m_packed_offsets.push_back(KCL::Vector2D( pos.x * scale, pos.y * scale));
	}

	m_packed_offsets.push_back(KCL::Vector2D(0.0f, 0.0f));

	for( unsigned c = 0; c < blur_kernel_size - 1; ++c )
	{
		float start = 2.0f / ( blur_kernel_size - 1 );
		float scale = 0.66f * 2.0f * 1.0f;

		KCL::Vector2D pos = circle( start, float( blur_kernel_size - 1 ), float( c ) );

		m_packed_offsets.push_back(KCL::Vector2D( pos.x * scale, pos.y * scale));
	}
}

KCL::uint32 BloomUpsampleBlur::RenderBlurPass( KCL::uint32 command_buffer, KCL::uint32 lod_level )
{
	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_input_texture0, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_input_texture1, NGL_SHADER_RESOURCE)
		.TextureBarrier(GetOutputTexture(), NGL_COLOR_ATTACHMENT);
	transitions.Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_TEXTURE_UNIT0] = &m_input_texture0;
	p[UNIFORM_INPUT_TEXTURE] = &m_input_texture1;

	p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
	KCL::Vector4D inv_res;
	inv_res.x = 0.5f * m_width;
	inv_res.y = 0.5f * m_width;
	inv_res.z = 1.0f / inv_res.x;
	inv_res.w = 1.0f / inv_res.y;
	p[UNIFORM_INV_RESOLUTION] = inv_res.v;

	nglBegin( m_blur_job[lod_level], command_buffer );
	nglDrawTwoSided( m_blur_job[lod_level], m_blur_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
	nglEnd( m_blur_job[lod_level] );

	return m_blur_job[lod_level];
}