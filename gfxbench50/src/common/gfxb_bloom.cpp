/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_bloom.h"
#include "gfxb_barrier.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_hdr.h"
#include "common/gfxb_gen_mipmaps.h"
#include "common/gfxb_fragment_blur.h"

#include <sstream>

using namespace GFXB;

Bloom::Bloom()
{
	m_shapes = nullptr;
	m_hdr = nullptr;

	m_width = 0;
	m_height = 0;

	m_layers = 0;
	m_texture_format = NGL_UNDEFINED;

	m_bright_pass = 0;
	m_bright_pass_manual_exposure_shader = 0;
	m_bright_pass_auto_exposure_shader = 0;

	m_blur_strength = 0;

	m_gen_mipmaps = nullptr;
}


Bloom::~Bloom()
{
	delete m_gen_mipmaps;
	for (size_t i = 0; i < m_blurs.size(); i++)
	{
		delete m_blurs[i];
	}
}


void Bloom::Init(Shapes *shapes, ComputeHDR *hdr, KCL::uint32 width, KCL::uint32 height, KCL::uint32 layers, KCL::uint32 gauss_strength, NGL_format texture_format)
{
	m_shapes = shapes;
	m_hdr = hdr;
	m_width = width;
	m_height = height;
	m_layers = layers;
	m_texture_format = texture_format;

	m_bright_pass_manual_exposure_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("bright_pass.vert", "bright_pass.frag").AddHeaderFile("tonemapper.h").AddDefine("EXPOSURE_MANUAL"));
	m_bright_pass_auto_exposure_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("bright_pass.vert", "bright_pass.frag").AddHeaderFile("tonemapper.h"));

	// Bright textures
	m_bright_textures.resize(m_layers, 0);
	for (KCL::uint32 i = 0; i < m_layers; i++)
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
		texture_layout.SetAllClearValue(0.0f);

		GetLayerSize(i, texture_layout.m_size);

		nglGenTexture(m_bright_textures[i], texture_layout, nullptr);
		Transitions::Get().Register(m_bright_textures[i], texture_layout);
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
		m_bright_pass = nglGenJob(rrd);

		int32_t viewport[4] =
		{
			0, 0, (int32_t)m_width, (int32_t)m_height
		};
		nglViewportScissor(m_bright_pass, viewport, viewport);
	}

	m_gen_mipmaps = new GenMipmaps();
	m_gen_mipmaps->Init("bloom", m_shapes, m_bright_textures, m_width, m_height);

	// Fragment blur for every layers
	m_blurs.resize(m_layers, 0);
	m_blur_strength = KCL::Max(gauss_strength, 3u);
	KCL::uint32 blur_texture_size[3];
	for (KCL::uint32 i = 0; i < m_layers; i++)
	{
		GetLayerSize(i, blur_texture_size);

		// PREC_TODO: half?
		m_blurs[i] = new FragmentBlur();
		m_blurs[i]->SetPrecision("float");
		m_blurs[i]->SetComponentCount(3);
		m_blurs[i]->Init("bloom", m_shapes, m_bright_textures[i], blur_texture_size[0], blur_texture_size[1], m_blur_strength, m_texture_format, 1);
	}
}


void Bloom::Resize(KCL::uint32 blur_strength, KCL::uint32 width, KCL::uint32 height)
{
	m_width = width;
	m_height = height;

	KCL::uint32 texture_size[3];
	for (KCL::uint32 i = 0; i < m_layers; i++)
	{
		GetLayerSize(i, texture_size);
		nglResizeTextures(1, &m_bright_textures[i], texture_size);

		m_blurs[i]->Resize(blur_strength, texture_size[0], texture_size[1]);
	}

	KCL::int32 viewport[4] = { 0, 0, KCL::int32(m_width), KCL::int32(m_height) };
	nglViewportScissor(m_bright_pass, viewport, viewport);

	m_gen_mipmaps->Resize(width, height);
}


void Bloom::DeletePipelines()
{
	nglDeletePipelines(m_bright_pass);

	m_gen_mipmaps->DeletePipelines();

	for (KCL::uint32 i = 0; i < m_layers; i++)
	{
		m_blurs[i]->DeletePipelines();
	}
}


void Bloom::SetColorCorrection(const KCL::Vector4D &color_correction)
{
	m_color_correction = color_correction;
}


void Bloom::ExecuteLumianceBufferBarrier(KCL::uint32 command_buffer)
{
	if (m_hdr->GetExposureMode() != EXPOSURE_MANUAL)
	{
        Transitions& transitions = Transitions::Get()
			.BufferBarrier(m_hdr->GetLuminanceBuffer(), NGL_SHADER_RESOURCE);
		transitions.Execute(command_buffer);
	}
}


KCL::uint32 Bloom::ExecuteBrightPass(KCL::uint32 command_buffer, KCL::uint32 input_texture)
{
	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_bright_textures[0], NGL_COLOR_ATTACHMENT)
		.TextureBarrier(input_texture, NGL_SHADER_RESOURCE);
	if (m_hdr->GetExposureMode() != EXPOSURE_MANUAL)
	{
		transitions.BufferBarrier(m_hdr->GetLuminanceBuffer(), NGL_SHADER_RESOURCE);
	}
	transitions.Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_INPUT_TEXTURE] = &input_texture;
	p[UNIFORM_HDR_ABCD] = m_hdr->GetUniformTonemapperConstants1();
	p[UNIFORM_HDR_EFW_TAU] = m_hdr->GetUniformTonemapperConstants2();
	p[UNIFORM_HDR_TONEMAP_WHITE] = m_hdr->GetUniformTonemapWhite();
	p[UNIFORM_HDR_EXPOSURE] = m_hdr->GetUniformExposure();
	p[UNIFORM_BLOOM_PARAMETERS] = m_hdr->GetUniformBloomParameters();
	p[UNIFORM_COLOR_CORRECTION] = m_color_correction.v;

	KCL::uint32 shader = m_bright_pass_manual_exposure_shader;
	if (m_hdr->GetExposureMode() != EXPOSURE_MANUAL)
	{
		shader = m_bright_pass_auto_exposure_shader;
	}

	nglBegin(m_bright_pass, command_buffer);
	nglDrawTwoSided(m_bright_pass, shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(m_bright_pass);

	return m_bright_pass;
}


KCL::uint32 Bloom::ExecuteDownsample(KCL::uint32 command_buffer, KCL::uint32 input_layer)
{
	return m_gen_mipmaps->GenerateMipmaps(command_buffer, input_layer);
}


KCL::uint32 Bloom::ExectureVerticalBlur(KCL::uint32 command_buffer, KCL::uint32 layer)
{
	return m_blurs[layer]->RenderVerticalPass(command_buffer, 0);
}


KCL::uint32 Bloom::ExectureHorizontalBlur(KCL::uint32 command_buffer, KCL::uint32 layer)
{
	return m_blurs[layer]->RenderHorizontalPass(command_buffer, 0);
}


KCL::uint32 Bloom::GetBrightTexture(KCL::uint32 layer) const
{
	return m_bright_textures[layer];
}


KCL::uint32 Bloom::GetBloomTexture(KCL::uint32 layer) const
{
	return m_blurs[layer]->GetOutputTexture();
}


void *Bloom::GetUniformBrightTexture(KCL::uint32 layer)
{
	return &m_bright_textures[layer];
}


void *Bloom::GetUniformBloomTexture(KCL::uint32 layer)
{
	return m_blurs[layer]->GetUniformOutputTexture();
}


KCL::uint32 Bloom::GetLayerCount() const
{
	return m_layers;
}


void Bloom::GetLayerSize(KCL::uint32 layer, KCL::uint32 texture_size[3])
{
	KCL::uint32 mipmap_width = m_width / (1 << (layer));
	KCL::uint32 mipmap_height = m_height / (1 << (layer));

	mipmap_width = KCL::Max(mipmap_width, 1u);
	mipmap_height = KCL::Max(mipmap_height, 1u);

	texture_size[0] = mipmap_width;
	texture_size[1] = mipmap_height;
	texture_size[2] = 1;
}
