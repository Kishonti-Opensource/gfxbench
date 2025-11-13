/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "gfxb_shader.h"
#include "gfxb_barrier.h"
#include "common/gfxb_bloom_downsample_blur.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_separable_blur.h"
#include "common/gfxb_gauss_blur_helper.h"

#include <sstream>
#include <algorithm>

using namespace GFXB;

void BloomDownsampleBlur::Init( Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels, const char* name )
{
	assert( !( lod_levels == 0 ) );
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_texture_format = texture_format;
	m_lod_levels = lod_levels;
	m_input_texture = input_texture;
	m_name = name;
	m_blur_texture = 0;

	std::string fragment_shader;

	fragment_shader = "bloom_downsample_blur.frag";

	SetKernelSize( blur_kernel_size );

	GenerateBlur( fragment_shader.c_str(), blur_kernel_size );
	ResizeJobs();
}

void BloomDownsampleBlur::GenerateBlur( const char *fragment_shader, const int blur_kernel_size )
{
	{
		// Compile the vertical shader
		ShaderDescriptor shader_desc;
		shader_desc.AddDefineInt( "LOD_LEVEL_COUNT", m_lod_levels );
		shader_desc.AddDefineInt( "MAX_KS", SeparableBlur::MAX_KS );
		shader_desc.AddDefineInt("KS", blur_kernel_size);
		shader_desc.SetVSFile( "gauss_blur_fs.vert" );
		shader_desc.SetFSFile( fragment_shader );

		shader_desc.AddDefineHalf("GAUSS_WEIGHT", m_weight);
		shader_desc.AddDefineString("PACKED_GAUSS_OFFSETS", GaussBlurHelper::GaussVector2DListToString(m_packed_offsets).c_str());

		m_blur_shader = ShaderFactory::GetInstance()->AddDescriptor( shader_desc );
	}
	{
		NGL_texture_descriptor desc;
		desc.m_name = m_name + "::blur";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_LINEAR_MIPMAPPED;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = m_width;
		desc.m_size[1] = m_height;
		desc.m_format = m_texture_format;
		desc.m_is_renderable = true;
		desc.m_num_levels = m_lod_levels;
		desc.SetAllClearValue( 0.0f );
		m_blur_texture = 0;
		nglGenTexture( m_blur_texture, desc, nullptr );
		Transitions::Get().Register(m_blur_texture, desc);		m_output_texture = m_blur_texture;	}

	for( uint32_t i = 0; i < m_lod_levels; ++i )
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_blur_texture;
			ad.m_attachment.m_level = i;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			std::stringstream sstream;
			sstream << m_name << "::blur_" << i;

			NGL_subpass sp;
			sp.m_name = sstream.str();
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_blur_job.push_back( nglGenJob( rrd ) );
	}
}

void BloomDownsampleBlur::Resize( KCL::uint32 blur_kernel_size, KCL::uint32 width, KCL::uint32 height )
{
	KCL::uint32 texture_size[3] = { width, height, 0 };
	KCL::uint32 textures[1] = { m_output_texture };

	nglResizeTextures( 1, textures, texture_size );

	//m_vertical_blur->Resize( blur_kernel_size, 0, 0, width, height );
	//m_horizontal_blur->Resize( blur_kernel_size, 0, 0, width, height );
}

void BloomDownsampleBlur::ResizeJobs()
{
	// Resize the jobs
	m_stepuv.resize( m_lod_levels );
	for( KCL::uint32 i = 0; i < m_lod_levels; i++ )
	{
		KCL::uint32 width = m_width / ( 1u << i );
		KCL::uint32 height = m_height / ( 1u << i );

		width = KCL::Max( width, 1u );
		height = KCL::Max( height, 1u );

		KCL::int32 viewport[4] =
		{
			0, 0, KCL::int32( width ), KCL::int32( height )
		};

		nglViewportScissor( m_blur_job[i], viewport, viewport );

		m_stepuv[i].x = 1.0f / float( width );
		m_stepuv[i].y = 1.0f / float( height );
	}
}

void BloomDownsampleBlur::SetKernelSize( KCL::uint32 blur_kernel_size )
{
	m_blur_kernel_size = KCL::Max( blur_kernel_size, 2u );

	m_weight = 1.0f / blur_kernel_size;
	m_packed_offsets.push_back( KCL::Vector2D(0.0f, 0.0f) );

	for( unsigned c = 0; c < blur_kernel_size - 1; ++c )
	{
		float start = 2.0f / (blur_kernel_size - 1);
		float scale = 0.66f * 4.0f * 2.0f;

		KCL::Vector2D pos = circle( start, float( blur_kernel_size - 1 ), float( c ) );

		m_packed_offsets.push_back(KCL::Vector2D( pos.x * scale, pos.y * scale));
	}
}

KCL::uint32 BloomDownsampleBlur::RenderBlurPass(KCL::uint32 command_buffer, KCL::uint32 lod_level )
{
	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_input_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(GetOutputTexture(), NGL_COLOR_ATTACHMENT);
	transitions.Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_TEXTURE_UNIT0] = &m_input_texture;

	p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
	KCL::Vector2D inv_res = m_stepuv[lod_level];
	inv_res.x *= 0.5f;
	inv_res.y *= 0.5f;
	p[UNIFORM_INV_RESOLUTION] = inv_res.v;

	nglBegin( m_blur_job[lod_level], command_buffer );
	nglDrawTwoSided( m_blur_job[lod_level], m_blur_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
	nglEnd( m_blur_job[lod_level] );

	return m_blur_job[lod_level];
}

KCL::uint32 BloomDownsampleBlur::GetOutputTexture() const
{
	return m_blur_texture;
}


void *BloomDownsampleBlur::GetUniformOutputTexture()
{
	return &m_blur_texture;
}

void BloomDownsampleBlur::DeletePipelines()
{
	for( KCL::uint32 i = 0; i < m_blur_job.size(); ++i )
	{
		nglDeletePipelines( m_blur_job[i] );
	}
}
