/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_bloom_mobile.h"
#include "gfxb_barrier.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_hdr.h"
#include "common/gfxb_gen_mipmaps.h"
#include "common/gfxb_bloom_downsample_blur.h"
#include "common/gfxb_bloom_upsample_blur.h"
#include "common/gfxb_separable_blur.h"

#include <sstream>

using namespace GFXB;

BloomMobile::BloomMobile()
{
	m_blur_kernel_size = 0;
}

BloomMobile::~BloomMobile()
{
	for( unsigned c = 0; c < m_downsample_blurs.size(); ++c )
	{
		delete m_downsample_blurs[c];
	}

	for( unsigned c = 0; c < m_upsample_blurs.size(); ++c )
	{
		delete m_upsample_blurs[c];
	}
}

void BloomMobile::Init( Shapes *shapes, ComputeHDR *hdr, KCL::uint32 width, KCL::uint32 height, KCL::uint32 layers, KCL::uint32 strength_down, KCL::uint32 strength_up, NGL_format texture_format )
{
	m_shapes = shapes;
	m_hdr = hdr;
	m_width = width;
	m_height = height;
	m_layers = layers;
	m_texture_format = texture_format;

	m_bright_pass_manual_exposure_shader = ShaderFactory::GetInstance()->AddDescriptor( ShaderDescriptor( "bright_pass.vert", "bright_pass_mobile.frag" ).AddHeaderFile( "tonemapper.h" ).AddDefine( "EXPOSURE_MANUAL" ) );
	m_bright_pass_auto_exposure_shader = ShaderFactory::GetInstance()->AddDescriptor( ShaderDescriptor( "bright_pass.vert", "bright_pass_mobile.frag" ).AddHeaderFile( "tonemapper.h" ) .AddDefineInt( "MAX_KS", SeparableBlur::MAX_KS ) );

	// Bright textures
	m_bright_textures.resize( m_layers, 0 );
	for( KCL::uint32 i = 0; i < m_layers; i++ )
	{
		NGL_texture_descriptor texture_layout;

		std::stringstream sstream;
		sstream << "bright_texture_" << i;

		texture_layout.m_name = sstream.str();
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_num_levels = 1;
		texture_layout.m_format = m_texture_format;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue( 0.0f );

		GetLayerSize( i, texture_layout.m_size );

		nglGenTexture( m_bright_textures[i], texture_layout, nullptr );
		Transitions::Get().Register( m_bright_textures[i], texture_layout);
	}

	// Bright pass
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_bright_textures[0];
			ad.m_attachment.m_level = 0;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "bloom::bright pass";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_bright_pass = nglGenJob( rrd );

		int32_t viewport[4] =
		{
			0, 0, (int32_t)m_width, (int32_t)m_height
		};
		nglViewportScissor( m_bright_pass, viewport, viewport );
	}

	m_gen_mipmaps = new GenMipmaps();
	m_gen_mipmaps->Init( "bloom", m_shapes, m_bright_textures, m_width, m_height );

	// Fragment blur for every layers
	m_downsample_blurs.resize( m_layers, 0 );
	m_upsample_blurs.resize( m_layers, 0 );
	//m_blur_strength = KCL::Max( strength, 3u );
	KCL::uint32 blur_texture_size[3];

	//1st downsample is actually the bright pass!
	//1/4-->1/8 uses bright pass result
	GetLayerSize( 1, blur_texture_size );
	m_downsample_blurs[1] = new BloomDownsampleBlur();
	m_downsample_blurs[1]->Init( m_shapes, m_bright_textures[0], blur_texture_size[0], blur_texture_size[1], strength_down, m_texture_format, 1, "bloom_downsample" );

	for( KCL::uint32 i = 2; i < m_layers; i++ )
	{
		GetLayerSize( i, blur_texture_size );

		m_downsample_blurs[i] = new BloomDownsampleBlur();
		m_downsample_blurs[i]->Init( m_shapes, m_downsample_blurs[i - 1]->GetOutputTexture(), blur_texture_size[0], blur_texture_size[1], strength_down, m_texture_format, 1, "bloom_downsample" );
	}

	for( KCL::uint32 i = 1; i < m_layers-1; i++ )
	{
		GetLayerSize( m_layers-i-1, blur_texture_size );

		m_upsample_blurs[i] = new BloomUpsampleBlur();
		m_upsample_blurs[i]->Init( m_shapes, m_downsample_blurs[m_layers - i]->GetOutputTexture(), m_downsample_blurs[m_layers - i - 1]->GetOutputTexture(), blur_texture_size[0], blur_texture_size[1], strength_up, m_texture_format, 1, "bloom_upsample" );
	}

	GetLayerSize( 0, blur_texture_size );

	m_upsample_blurs[m_layers - 1] = new BloomUpsampleBlur();
	m_upsample_blurs[m_layers - 1]->Init( m_shapes, m_downsample_blurs[1]->GetOutputTexture(), m_bright_textures[0], blur_texture_size[0], blur_texture_size[1], strength_up, m_texture_format, 1, "bloom_upsample" );


	{ //set bright pass sampling kernel data

		// Resize the jobs
		m_stepuv.resize( 1 );

		m_stepuv[0].x = 1.0f / float( width );
		m_stepuv[0].y = 1.0f / float( height );
	}
}

KCL::uint32 BloomMobile::ExecuteUpsample(KCL::uint32 command_buffer, KCL::uint32 input_layer )
{
	return m_upsample_blurs[input_layer]->RenderBlurPass(command_buffer, 0 );
}

KCL::uint32 BloomMobile::ExecuteDownsample(KCL::uint32 command_buffer, KCL::uint32 input_layer )
{
	return m_downsample_blurs[input_layer]->RenderBlurPass(command_buffer, 0 );
}

void *BloomMobile::GetUniformBloomTexture( KCL::uint32 layer )
{
	return m_upsample_blurs[layer]->GetUniformOutputTexture();
}

KCL::uint32 BloomMobile::GetBloomTexture( KCL::uint32 layer ) const
{
	return m_upsample_blurs[layer]->GetOutputTexture();
}

void BloomMobile::DeletePipelines()
{
	nglDeletePipelines( m_bright_pass );

	m_gen_mipmaps->DeletePipelines();

	for( KCL::uint32 i = 0; i < m_layers; i++ )
	{
		if( m_downsample_blurs[i] )
		{
			m_downsample_blurs[i]->DeletePipelines();
		}
		if( m_upsample_blurs[i] )
		{
			m_upsample_blurs[i]->DeletePipelines();
		}
	}
}

void BloomMobile::Resize( KCL::uint32 blur_strength, KCL::uint32 width, KCL::uint32 height )
{
	m_width = width;
	m_height = height;

	KCL::uint32 texture_size[3];
	for( KCL::uint32 i = 0; i < m_layers; i++ )
	{
		GetLayerSize( i, texture_size );
		nglResizeTextures( 1, &m_bright_textures[i], texture_size );
	}

	for( KCL::uint32 i = 1; i < m_layers; i++ )
	{
		GetLayerSize( i, texture_size );
		m_downsample_blurs[i]->Resize( blur_strength, texture_size[0], texture_size[1] );
	}

	for( KCL::uint32 i = 1; i < m_layers-1; i++ )
	{
		GetLayerSize( m_layers - i - 1, texture_size );
		m_upsample_blurs[i]->Resize( blur_strength, texture_size[0], texture_size[1] );
	}

	KCL::int32 viewport[4] = { 0, 0, KCL::int32( m_width ), KCL::int32( m_height ) };
	nglViewportScissor( m_bright_pass, viewport, viewport );

	m_gen_mipmaps->Resize( width, height );


	// Resize the jobs
	m_stepuv.resize( 1 );

	m_stepuv[0].x = 1.0f / float( width );
	m_stepuv[0].y = 1.0f / float( height );
}

KCL::uint32 BloomMobile::ExecuteBrightPass( KCL::uint32 command_buffer, KCL::uint32 input_texture )
{
	Transitions &transitions = Transitions::Get()
		.TextureBarrier(input_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_bright_textures[0], NGL_COLOR_ATTACHMENT);
	transitions.Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_INPUT_TEXTURE] = &input_texture;
	p[UNIFORM_HDR_ABCD] = m_hdr->GetUniformTonemapperConstants1();
	p[UNIFORM_HDR_EFW_TAU] = m_hdr->GetUniformTonemapperConstants2();
	p[UNIFORM_HDR_TONEMAP_WHITE] = m_hdr->GetUniformTonemapWhite();
	p[UNIFORM_HDR_EXPOSURE] = m_hdr->GetUniformExposure();
	p[UNIFORM_BLOOM_PARAMETERS] = m_hdr->GetUniformBloomParameters();
	p[UNIFORM_COLOR_CORRECTION] = m_color_correction.v;

	uint32_t lod_level = 0;
	p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
	KCL::Vector2D inv_res = m_stepuv[lod_level];
	inv_res.x *= 0.25f;
	inv_res.y *= 0.25f;
	p[UNIFORM_INV_RESOLUTION] = inv_res.v;

	KCL::uint32 shader = m_bright_pass_manual_exposure_shader;
	if( m_hdr->GetExposureMode() != EXPOSURE_MANUAL )
	{
		shader = m_bright_pass_auto_exposure_shader;
	}

	nglBegin( m_bright_pass, command_buffer);
	nglDrawTwoSided( m_bright_pass, shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
	nglEnd( m_bright_pass );

	return m_bright_pass;
}
