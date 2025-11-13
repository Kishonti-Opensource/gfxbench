/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_dof.h"
#include "gfxb_shader.h"
#include "gfxb_barrier.h"
#include "common/gfxb_shapes.h"

#include <kcl_camera2.h>

using namespace GFXB;

bool DOF::is_portrait = false;

DOF::DOF()
{
	m_name = "DOF";

	m_shapes = nullptr;

	m_half_res_mode = false;

	m_width = 0;
	m_height = 0;
	m_kernel_size = 0;
	m_input_texture = 0;
	m_vertical_output_texture = 0;
	m_horizontal_output_texture = 0;

	m_apply_render = 0;
	m_apply_shader = 0;

	m_half_res_texture = 0;
	m_downsample_shader = 0;
	m_downsample_render = 0;

	m_vertical_blur = nullptr;
	m_horizontal_blur = nullptr;
}


DOF::~DOF()
{
	DeletePipelines();

	delete m_vertical_blur;
    delete m_horizontal_blur;

	for (size_t i = 0; i < m_horizontal_blurs.size(); i++)
	{
		delete m_horizontal_blurs[i];
	}
}


void DOF::Init(Shapes *shapes, KCL::uint32 width, KCL::uint32 height, bool is_port, KCL::uint32 kernel_size, KCL::uint32 input_texture, std::vector<KCL::uint32> output_textures, bool half_res)
{
	is_portrait = is_port;
	m_shapes = shapes;
	m_kernel_size = kernel_size;
	m_width = width;
	m_height = height;
	m_input_texture = input_texture;
	m_output_textures = output_textures;
	m_half_res_mode = half_res;

	if (m_half_res_mode)
	{
		m_width = KCL::Max(m_width / 2u, 1u);
		m_height = KCL::Max(m_height / 2u, 1u);
	}

	NGL_texture_descriptor desc;
	NGL_format texture_format = NGL_R8_G8_B8_A8_UNORM;
	{
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_LINEAR;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = is_portrait ? m_height : m_width;
		desc.m_size[1] = is_portrait ? m_width : m_height;
		desc.m_format = texture_format;
		desc.m_is_renderable = true;
		desc.m_num_levels = 1;
		desc.SetAllClearValue(0.0f);
	}

	{
		// Output texture of vertical blur
		desc.m_name = m_name + "::blur_v";
		nglGenTexture(m_vertical_output_texture, desc, nullptr);
		Transitions::Get().Register(m_vertical_output_texture, desc);

		// Common vertical blur
		m_vertical_blur = new SeparableBlur();
		m_vertical_blur->SetPrecision("half");
		m_vertical_blur->SetComponentCount(3);
		m_vertical_blur->Init(desc.m_name.c_str(), SeparableBlur::VERTICAL, shapes, 0, m_vertical_output_texture, is_portrait ? m_height : m_width, is_portrait ? m_width : m_height, kernel_size, texture_format, 1);
	}

#ifdef GFXB_DOF_MERGED_APPLY
	if (!m_half_res_mode)
	{
		desc.m_size[0] = m_width;
		desc.m_size[1] = m_height;

		// Horizontal + apply in one pass
		for (KCL::uint32 i = 0; i < output_textures.size(); i++)
		{
			std::string name = m_name + "::blur_h_apply";

			SeparableBlur *blur = new DOFHorizontalBlurFullRes();
			blur->SetPrecision("half");
			blur->SetComponentCount(3);
			blur->Init(name.c_str(), SeparableBlur::HORIZONTAL, shapes, m_vertical_output_texture, output_textures[i], m_width, m_height, kernel_size, texture_format, 1);
			m_horizontal_blurs.push_back(blur);
		}

		return;
	}
#endif

	{
		// Output texture of horizontal blur
		desc.m_name = m_name + "::blur_h";
		nglGenTexture(m_horizontal_output_texture, desc, nullptr);
		Transitions::Get().Register(m_horizontal_output_texture, desc);

		// Common horizontal blur
		m_horizontal_blur = new SeparableBlur();
		m_horizontal_blur->SetPrecision("half");
		m_horizontal_blur->SetComponentCount(3);
		m_horizontal_blur->Init(desc.m_name.c_str(), SeparableBlur::HORIZONTAL, shapes, m_vertical_output_texture, m_horizontal_output_texture, is_portrait ? m_height : m_width, is_portrait ? m_width : m_height, kernel_size, texture_format, 1);
	}

	{
		// Apply jobs
		m_apply_renders.resize(output_textures.size());
		for (KCL::uint32 i = 0; i < output_textures.size(); i++)
		{
			NGL_job_descriptor rrd;
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = output_textures[i];
				ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				NGL_subpass sp;
				sp.m_name = "dof_apply";
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
				rrd.m_subpasses.push_back(sp);
			}

			rrd.m_load_shader_callback = LoadShader;
			m_apply_renders[i] = nglGenJob(rrd);

			KCL::int32 viewport[4] =
			{
				0, 0, KCL::int32(width), KCL::int32(height) // (width x height) is the original (not halved) resolution
			};
			nglViewportScissor(m_apply_renders[i], viewport, viewport);
		}
	}

	if (m_half_res_mode)
	{
		// Apply shader
		{
			ShaderDescriptor shader_desc;
			shader_desc.SetVSFile("dof.vert").AddDefineInt("DOF_PORTRAIT_MODE", is_portrait);
			shader_desc.SetFSFile("dof.frag").AddDefineInt("DOF_HALF_RES", 1);
			m_apply_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
		}

		// Downsampled input texture
		desc.m_name = m_name + "::half_res";
		nglGenTexture(m_half_res_texture, desc, nullptr);
		Transitions::Get().Register(m_half_res_texture, desc);

		// The halved texture is the input of the vertical blur
		m_vertical_blur->SetInputTexture(m_half_res_texture);

		// Downsample shader
		ShaderDescriptor shader_desc;
		shader_desc.SetVSFile("downsample.vert");
		shader_desc.SetFSFile("downsample.frag");
		m_downsample_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);

		// Downsample renderer
		{
			NGL_job_descriptor rrd;
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = m_half_res_texture;
				ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				NGL_subpass sp;
				sp.m_name = m_name + "::downsample";
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
				rrd.m_subpasses.push_back(sp);
			}

			rrd.m_load_shader_callback = LoadShader;
			m_downsample_render = nglGenJob(rrd);

			KCL::int32 viewport[4] =
			{
				0, 0, KCL::int32(is_portrait ? m_height : m_width), KCL::int32(is_portrait ? m_width : m_height)
			};
			nglViewportScissor(m_downsample_render, viewport, viewport);
		}
	}
	else
	{
		// Apply shader
		{
			ShaderDescriptor shader_desc;
			shader_desc.SetVSFile("dof.vert").AddDefineInt("DOF_PORTRAIT_MODE", is_portrait);
			shader_desc.SetFSFile("dof.frag");
			m_apply_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
		}
	}

	SetOutputBufferId(0);
}


void DOF::Render(KCL::uint32 command_buffer, KCL::Camera2 *camera, KCL::uint32 depth_texture, float focus_distance)
{
	assert(m_input_texture != 0);

	m_dof_parameters.x = focus_distance + m_dof_values.m_focus_range;
	m_dof_parameters.z = KCL::Max(m_dof_values.m_range, 0.01f);
	m_dof_parameters.w = KCL::Max(m_dof_values.m_function, 0.01f);

	m_vertical_blur->SetKernelSize(m_kernel_size);
	m_horizontal_blur->SetKernelSize(m_kernel_size);

	NGL_texture_subresource depth_subresource(depth_texture);

	const void *p[UNIFORM_MAX];

#ifdef GFXB_DOF_MERGED_APPLY
	if (!m_half_res_mode)
	{
		m_vertical_blur->SetInputTexture(m_input_texture);
		m_vertical_blur->Render(command_buffer);

		p[UNIFORM_DOF_INPUT_TEXTURE] = &m_input_texture;  // Original texture without blur
		p[UNIFORM_GBUFFER_DEPTH_TEX] = &depth_texture;

		// Camera uniforms
		p[UNIFORM_DEPTH_PARAMETERS] = camera->m_depth_linearize_factors.v;

		// DOF values
		p[UNIFORM_DOF_PARAMETERS] = m_dof_parameters.v;

		// These transitions are executed by the horziontal blur
		Transitions::Get()
			.TextureBarrier(m_input_texture, NGL_SHADER_RESOURCE)
			.TextureBarrier(depth_texture, NGL_SHADER_RESOURCE);

		m_horizontal_blur->Render(command_buffer, p);

		return;
	}
#endif

	if (m_half_res_mode)
	{
		// Downsampling
		{
			p[UNIFORM_INPUT_TEXTURE] = &m_input_texture;

			Transitions::Get()
				.TextureBarrier(m_half_res_texture, NGL_COLOR_ATTACHMENT)
				.TextureBarrier(m_input_texture, NGL_SHADER_RESOURCE)
				.Execute(command_buffer);

			nglBegin(m_downsample_render, command_buffer);
			nglDrawTwoSided(m_downsample_render, m_downsample_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
			nglEnd(m_downsample_render);
		}
	}
	else
	{
		m_vertical_blur->SetInputTexture(m_input_texture);
	}

	{
		// Blur
		{
			assert(m_half_res_mode || (m_vertical_blur->GetInputTexture() == m_input_texture));
			assert(!m_half_res_mode || (m_vertical_blur->GetInputTexture() == m_half_res_texture));

			m_vertical_blur->Render(command_buffer);
			m_horizontal_blur->Render(command_buffer);
		}

		// Apply DoF
		{
			p[UNIFORM_DOF_INPUT_TEXTURE] = &m_input_texture; // Original texture without blur
			p[UNIFORM_INPUT_TEXTURE] = m_horizontal_blur->GetUniformOutputTexture(); // Blurred image
			p[UNIFORM_GBUFFER_DEPTH_TEX] = &depth_subresource;

			// Camera uniforms
			p[UNIFORM_DEPTH_PARAMETERS] = camera->m_depth_linearize_factors.v;

			// DoF values
			p[UNIFORM_DOF_PARAMETERS] = m_dof_parameters.v;

			Transitions::Get()
				.TextureMipLevelBarrier(depth_subresource.m_idx, 0, NGL_SHADER_RESOURCE)
				.TextureBarrier(m_input_texture, NGL_SHADER_RESOURCE)
				.TextureBarrier(m_horizontal_blur->GetOutputTexture(), NGL_SHADER_RESOURCE)
				.TextureBarrier(m_output_texture, NGL_COLOR_ATTACHMENT)
				.Execute(command_buffer);

			nglBegin(m_apply_render, command_buffer);
			nglDrawTwoSided(m_apply_render, m_apply_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
			nglEnd(m_apply_render);
		}
	}
}


void DOF::SetDOFValues(const DOFValues &values)
{
	m_dof_values = values;
}


void DOF::SetOutputBufferId(KCL::uint32 id)
{
	m_output_texture = m_output_textures[id];

#ifdef GFXB_DOF_MERGED_APPLY
	if (!m_half_res_mode)
	{
		m_horizontal_blur = m_horizontal_blurs[id];
		return;
	}
#endif

	m_apply_render = m_apply_renders[id];
}


void DOF::SetKernelSize(KCL::uint32 size)
{
	m_kernel_size = size;
}


void DOF::Resize(KCL::uint32 kernel_size, KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height)
{
	SetKernelSize(kernel_size);

	m_width = width;
	m_height = height;

	if (m_half_res_mode)
	{
		m_width = KCL::Max(m_width / 2u, 1u);
		m_height = KCL::Max(m_height / 2u, 1u);
	}

	KCL::uint32 texture_size[3] = { m_width, m_height, 0 };
	nglResizeTextures(1, &m_vertical_output_texture, texture_size);
	m_vertical_blur->Resize(kernel_size, 0, 0, m_width, m_height);

#ifdef GFXB_DOF_MERGED_APPLY
	if (!m_half_res_mode)
	{
		for (size_t i = 0; i < m_horizontal_blurs.size(); i++)
		{
			m_horizontal_blurs[i]->Resize(kernel_size, x, y, m_width, m_height);
		}

		return;
	}
#endif

	nglResizeTextures(1, &m_horizontal_output_texture, texture_size);
	m_horizontal_blur->Resize(kernel_size, 0, 0, m_width, m_height);

	if (m_half_res_mode)
	{
		nglResizeTextures(1, &m_half_res_texture, texture_size);

		int32_t half_viewport[4] = { (int32_t)x, (int32_t)y, (int32_t)m_width, (int32_t)m_height };
		nglViewportScissor(m_downsample_render, half_viewport, half_viewport);
	}

	{
		int32_t viewport[4] = {(int32_t)x, (int32_t)y, (int32_t)width, (int32_t)height};  // (width x height) is the original (not halved) resolution
		for (size_t i = 0; i < m_apply_renders.size(); i++)
		{
			nglViewportScissor(m_apply_renders[i], viewport, viewport);
		}
	}
}


void DOF::DeletePipelines()
{
	m_vertical_blur->DeletePipelines();

	for (size_t i = 0; i < m_horizontal_blurs.size(); i++)
	{
		m_horizontal_blurs[i]->DeletePipelines();
	}

	for(size_t i = 0; i < m_apply_renders.size(); i++)
	{
		nglDeletePipelines(m_apply_renders[i]);
	}

	if (m_downsample_render)
	{
		nglDeletePipelines(m_downsample_render);
	}
}


bool DOF::IsHalfResMode() const
{
	return m_half_res_mode;
}


void DOF::SetInputTexture(KCL::uint32 texture)
{
	m_input_texture = texture;
}


KCL::uint32 DOF::GetInputTexture() const
{
	return m_input_texture;
}


void *DOF::GetUniformInputTexture()
{
	return &m_input_texture;
}


KCL::uint32 DOF::GetDownsampledTexture() const
{
	return m_half_res_texture;
}


void *DOF::GetUniformDownsampledTexture()
{
	return &m_half_res_texture;
}


KCL::uint32 DOF::GetOuputTexture() const
{
	return m_horizontal_blur->GetOutputTexture();
}


void *DOF::GetUniformOutputTexture()
{
	return m_horizontal_blur->GetUniformOutputTexture();
}
