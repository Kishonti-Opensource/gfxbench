/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "gfxb_shader.h"
#include "gfxb_barrier.h"
#include "common/gfxb_separable_blur.h"
#include "common/gfxb_gauss_blur_helper.h"
#include "common/gfxb_shapes.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip> // std::setprecision

//#define GFXB_DISABLE_SEPARABLE_BLUR_LOOP_UNROLL

using namespace GFXB;

SeparableBlur::SeparableBlur()
{
	m_direction = HORIZONTAL;

	m_x = 0;
	m_y = 0;
	m_width = 0;
	m_height = 0;

	m_lod_levels = 0;
	m_input_texture = 0;
	m_output_texture = 0;
	m_texture_format = NGL_UNDEFINED;

	m_blur_shader = 0;
	m_blur_kernel_size = 0;

	m_component_count = 0;
	m_precision = "";

	m_blur_shader_cache.clear();
}


SeparableBlur::~SeparableBlur()
{
}


void SeparableBlur::Init(const char* name, Direction dir, Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 output_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels)
{
	assert(!(lod_levels == 0));

	//make sure swizzle masks are set!
	assert( m_precision.length() );
	assert(m_component_count != 0);

	m_name = name;
	m_direction = dir;
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_texture_format = texture_format;
	m_lod_levels = lod_levels;
	m_input_texture = input_texture;
	m_output_texture = output_texture;

	m_input_mask = "";
	m_blur_type = m_precision;
	switch (m_component_count)
	{
		case 1:
			m_input_mask = "V.x";
			break;
		case 2:
			m_input_mask = "V.xy";
			m_blur_type += '2';
			break;
		case 3:
			m_input_mask = "V.xyz";
			m_blur_type += '3';
			break;
		case 4:
			m_input_mask = "V";
			m_blur_type += '4';
			break;
		default:
			assert(0);
			break;
	}

	KCL::uint32 texture_component_count = GetTextureComponentCount(m_texture_format);
	assert(m_component_count <= texture_component_count);

	m_output_mask = (m_component_count == texture_component_count) ? "V" : m_input_mask;
	{
		std::stringstream t;
		t << m_precision;
		if (texture_component_count != 1) t << texture_component_count;
		m_texture_type = t.str();
	}

	SetKernelSize(blur_kernel_size);

	// Compile the shader
	m_blur_shader = GetBlurShader(m_blur_kernel_size);

	// Create the jobs
	for (uint32_t i = 0; i < m_lod_levels; ++i)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_output_texture;
			ad.m_attachment.m_level = i;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = m_name;
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_blur_jobs.push_back(nglGenJob(rrd));
	}

	ResizeJobs();
}


KCL::uint32 SeparableBlur::GetBlurShader(KCL::uint32 kernel_size)
{
	KCL::uint32 res = 0;
	if (m_lod_levels > 1)
	{
		res = 1;
	}
	else
	{
		res = m_direction == VERTICAL ? m_height : m_width;
	}

	std::pair<KCL::uint32, KCL::uint32> key(kernel_size, res);
	if (m_blur_shader_cache.find(key) != m_blur_shader_cache.end())
	{
		return m_blur_shader_cache.at(key);
	}

	ShaderDescriptor shader_desc;
	CreateShaderDescriptor(shader_desc, kernel_size, res);
	KCL::uint32 s = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
	m_blur_shader_cache[key] = s;
	return s;
}


void SeparableBlur::CreateShaderDescriptor(ShaderDescriptor &shader_desc, KCL::uint32 kernel_size, KCL::uint32 res)
{
	assert(m_blur_type.length() > 0);
	assert(m_precision.length() > 0);
	assert(m_input_mask.length() > 0);
	assert(m_output_mask.length() > 0);
	assert(m_texture_type.length() > 0);

	// Generate the shader constants
	std::vector<float> gauss_weights = GaussBlurHelper::GetGaussWeights(m_blur_kernel_size, true);
	std::vector<float> packed_weights = GaussBlurHelper::CalcPackedWeights(gauss_weights);
	std::vector<float> packed_offsets = GaussBlurHelper::CalcPackedOffsets(res, m_blur_kernel_size, gauss_weights);

	bool is_half = (m_precision == "half");

	shader_desc.AddDefine(m_direction == VERTICAL ? "VERTICAL" : "HORIZONTAL");
	shader_desc.AddDefineInt("LOD_LEVEL_COUNT", m_lod_levels);
	shader_desc.AddDefineInt("MAX_KS", MAX_KS);
	shader_desc.AddDefineInt("KS", kernel_size+1);

#ifndef GFXB_DISABLE_SEPARABLE_BLUR_LOOP_UNROLL
	std::ostringstream unrolled_loop;
	unrolled_loop << std::fixed << std::setprecision(10); // same format as in GaussBlurHelper::GaussFloatListToString()
	std::string precision_qualifier = is_half ? "h" : "";

	for (KCL::uint32 i = 0; i <= kernel_size; i++)
	{
		if (m_lod_levels > 1)
		{
			if (m_direction == VERTICAL)
			{
				unrolled_loop << "color += ";
				unrolled_loop << packed_weights[i] << precision_qualifier;
				unrolled_loop << " * textureLod( texture_unit0, texcoord + float2(0.0, inv_resolution.y * ";
				unrolled_loop << packed_offsets[i];
				unrolled_loop << "), float(gauss_lod_level)).INPUT_SWIZZLE; ";
			}
			else
			{
				unrolled_loop << "color += ";
				unrolled_loop << packed_weights[i] << precision_qualifier;
				unrolled_loop << " * textureLod( texture_unit0, texcoord + float2(inv_resolution.x * ";
				unrolled_loop << packed_offsets[i];
				unrolled_loop << ", 0.0), float(gauss_lod_level)).INPUT_SWIZZLE; ";
			}
		}
		else
		{
			if (m_direction == VERTICAL)
			{
				unrolled_loop << "s += ";
				unrolled_loop << packed_weights[i] << precision_qualifier;
				unrolled_loop << " * INPUT_MASK(texture( texture_unit0, texcoord + float2(0.0, ";
				unrolled_loop << packed_offsets[i];
				unrolled_loop << "))); ";
			}
			else
			{
				unrolled_loop << "s += ";
				unrolled_loop << packed_weights[i] << precision_qualifier;
				unrolled_loop << " * INPUT_MASK(texture( texture_unit0, texcoord + float2(";
				unrolled_loop << packed_offsets[i];
				unrolled_loop << ", 0.0))); ";
			}
		}
	}

	shader_desc.AddDefineString("LOOP_UNROLL", "1");
	shader_desc.AddDefineString("UNROLLED_BLUR", unrolled_loop.str().c_str());
#else
	shader_desc.AddDefineString("LOOP_UNROLL", "0");
	shader_desc.AddDefineString("PACKED_GAUSS_WEIGHTS", GaussBlurHelper::GaussFloatListToString(packed_weights, is_half).c_str());
	shader_desc.AddDefineString("PACKED_GAUSS_OFFSETS", GaussBlurHelper::GaussFloatListToString(packed_offsets, false).c_str());
#endif

	shader_desc.SetVSFile("gauss_blur_fs.vert");
	shader_desc.SetFSFile("gauss_blur_fs.frag")
		.AddDefineString("BLUR_TYPE", m_blur_type.c_str())
		.AddDefineString("PREC_TYPE", m_precision.c_str());

	shader_desc.AddDefineString("INPUT_MASK(V)", m_input_mask.c_str());
	shader_desc.AddDefineString("OUTPUT_MASK(V)", m_output_mask.c_str());
	shader_desc.AddDefineString("TEXTURE_TYPE",m_texture_type.c_str());

	shader_desc.AddHeaderFile(m_lod_levels > 1 ? "gauss_blur_fs_lod.h" : "gauss_blur_fs.h");
}


void SeparableBlur::Resize(KCL::uint32 blur_kernel_size, KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height)
{
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;

	SetKernelSize(blur_kernel_size);

	ResizeJobs();
}


void SeparableBlur::ResizeJobs()
{
	// Resize the jobs
	m_stepuv.resize(m_lod_levels);
	for (KCL::uint32 i = 0; i < m_lod_levels; i++)
	{
		KCL::uint32 width = m_width / (1u << i);
		KCL::uint32 height = m_height / (1u << i);

		width = KCL::Max(width, 1u);
		height = KCL::Max(height, 1u);

		KCL::int32 viewport[4] =
		{
			KCL::int32(m_x), KCL::int32(m_y), KCL::int32(width), KCL::int32(height)
		};

		nglViewportScissor(m_blur_jobs[i], viewport, viewport);

		m_stepuv[i].x = 1.0f / float(width);
		m_stepuv[i].y = 1.0f / float(height);
	}
}


void SeparableBlur::SetKernelSize(KCL::uint32 blur_kernel_size)
{
	blur_kernel_size = KCL::Max(blur_kernel_size, 2u);

	if (m_blur_kernel_size == blur_kernel_size)
	{
		return;
	}

	m_blur_kernel_size = blur_kernel_size;
	m_blur_shader = GetBlurShader(blur_kernel_size);
}


void SeparableBlur::DeletePipelines()
{
	for (KCL::uint32 i = 0; i < m_blur_jobs.size(); ++i)
	{
		nglDeletePipelines(m_blur_jobs[i]);
	}
}


KCL::uint32 SeparableBlur::Render(KCL::uint32 command_buffer, KCL::uint32 lod_level, bool end_job)
{
	const void *p[UNIFORM_MAX];
	return Render(command_buffer, p, lod_level, end_job);
}


KCL::uint32 SeparableBlur::Render(KCL::uint32 command_buffer, const void **p, KCL::uint32 lod_level, bool end_job)
{
	assert(m_input_texture != 0);

	Transitions::Get()
		.TextureMipLevelBarrier(m_input_texture, lod_level, NGL_SHADER_RESOURCE)
		.TextureMipLevelBarrier(m_output_texture, lod_level, NGL_COLOR_ATTACHMENT)
		.Execute(command_buffer);

	KCL::uint32 job = m_blur_jobs[lod_level];

	nglBlendState(job, 0, NGL_BLEND_DISABLED, NGL_CHANNEL_ALL);

	p[UNIFORM_TEXTURE_UNIT0] = &m_input_texture;

	if (m_lod_levels > 1)
	{
		p[UNIFORM_GAUSS_LOD_LEVEL] = &lod_level;
		p[UNIFORM_INV_RESOLUTION] = m_stepuv[lod_level].v;
	}

	nglBegin(job, command_buffer);
	nglDrawTwoSided(job, m_blur_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	if (end_job)
	{
		nglEnd(job);
	}

	return job;
}


SeparableBlur::Direction SeparableBlur::GetDirection() const
{
	return m_direction;
}


void SeparableBlur::SetInputTexture(KCL::uint32 in_tex)
{
	m_input_texture = in_tex;
}

KCL::uint32 SeparableBlur::GetInputTexture() const
{
	return m_input_texture;
}


void *SeparableBlur::GetUniformInputTexture()
{
	return &m_input_texture;
}


KCL::uint32 SeparableBlur::GetOutputTexture() const
{
	return m_output_texture;
}


void *SeparableBlur::GetUniformOutputTexture()
{
	return &m_output_texture;
}


KCL::uint32 SeparableBlur::GetJob(KCL::uint32 lod_level) const
{
	return m_blur_jobs[lod_level];
}


KCL::uint32 SeparableBlur::GetTextureComponentCount(KCL::uint32 texture_type)
{
	switch (texture_type)
	{
	case NGL_R8_G8_B8_A8_UNORM:
	case NGL_R16_G16_B16_A16_FLOAT:
		return 4;
	
	case NGL_R8_UNORM:
		return 1;
	
	default:
		assert(0);
		break;
	}
	return 0;
}
