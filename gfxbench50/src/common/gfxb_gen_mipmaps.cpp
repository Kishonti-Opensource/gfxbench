/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_gen_mipmaps.h"
#include "gfxb_shader.h"
#include "gfxb_shapes.h"
#include "gfxb_barrier.h"

#include <ngl.h>
#include <kcl_math3d.h>
#include <sstream>

using namespace GFXB;

GenMipmaps::GenMipmaps()
{
	m_width = 0;
	m_height = 0;
	m_levels = 0;
	m_shader = 0;
}


GenMipmaps::~GenMipmaps()
{
	DeletePipelines();
}


void GenMipmaps::Init(const char *name, Shapes *shapes, KCL::uint32 texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 levels)
{
	m_lod_mode = true;
	m_name = name;
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_levels = levels;

	m_textures.resize(m_levels, texture);

	InitRenderers();
}


void GenMipmaps::Init(const char *name, Shapes *shapes, std::vector<KCL::uint32> textures, KCL::uint32 width, KCL::uint32 height)
{
	m_lod_mode = false;
	m_name = name;
	m_shapes = shapes;
	m_width = width;
	m_height = height;
	m_levels = KCL::uint32(textures.size());

	m_textures = textures;

	InitRenderers();
}


void GenMipmaps::InitRenderers()
{
	m_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("downsample.vert", "downsample.frag").AddDefineInt("LOD_MODE", m_lod_mode));

	for (KCL::uint32 i = 1; i < m_levels; i++)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_textures[i];
			ad.m_attachment.m_level = m_lod_mode ? i : 0;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			std::stringstream sstream;
			sstream << m_name << "::gen_mips_" << i;

			NGL_subpass sp;
			sp.m_name = sstream.str();
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;

		KCL::uint32 job = nglGenJob(rrd);
		m_jobs.push_back(job);

		nglDepthState(job, NGL_DEPTH_DISABLED, false);
	}

	Resize(m_width, m_height);
}


void GenMipmaps::Resize(KCL::uint32 width, KCL::uint32 height)
{
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		KCL::uint32 mipmap_width = width / (1 << (i + 1));
		KCL::uint32 mipmap_height = height / (1 << (i + 1));

		mipmap_width = KCL::Max(mipmap_width, 1u);
		mipmap_height = KCL::Max(mipmap_height, 1u);

		KCL::int32 vp[4] = { 0, 0, KCL::int32(mipmap_width), KCL::int32(mipmap_height) };
		nglViewportScissor(m_jobs[i], vp, vp);
	}
}


void GenMipmaps::DeletePipelines()
{
	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		nglDeletePipelines(m_jobs[i]);
	}
}


KCL::uint32 GenMipmaps::GenerateMipmaps(KCL::uint32 command_buffer, KCL::uint32 input_level)
{
	if (m_lod_mode)
	{
		Transitions::Get()
			.TextureMipLevelBarrier(m_textures[input_level], input_level, NGL_SHADER_RESOURCE)
			.TextureMipLevelBarrier(m_textures[input_level + 1], input_level + 1, NGL_COLOR_ATTACHMENT)
			.Execute(command_buffer);
	}
	else
	{
		Transitions::Get()
			.TextureBarrier(m_textures[input_level], NGL_SHADER_RESOURCE)
			.TextureBarrier(m_textures[input_level + 1], NGL_COLOR_ATTACHMENT)
			.Execute(command_buffer);
	}

	float lod_level = float(input_level);

	const void *p[UNIFORM_MAX];
	p[UNIFORM_INPUT_TEXTURE] = &m_textures[input_level];
	p[UNIFORM_LOD_LEVEL] = &lod_level;

	KCL::uint32 job = m_jobs[input_level];
	nglBegin(job, command_buffer);
	nglDrawTwoSided(job, m_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(job);

	return job;
}


KCL::uint32 GenMipmaps::GetLevels() const
{
	return m_levels;
}


KCL::uint32 GenMipmaps::GetTexture(KCL::uint32 level) const
{
	return m_textures[level];
}
