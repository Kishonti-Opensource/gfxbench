/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_fragment_blur.h"
#include "gfxb_barrier.h"

#include <assert.h>

using namespace GFXB;


FragmentBlur::FragmentBlur()
{
	m_temp_texture = 0;
	m_output_texture = 0;

	m_vertical_blur = new SeparableBlur();
	m_horizontal_blur = new SeparableBlur();

	m_component_count = 0;
	m_precision = "";
}


FragmentBlur::~FragmentBlur()
{
	delete m_vertical_blur;
	delete m_horizontal_blur;
}


void FragmentBlur::Init(const char* name, Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels)
{
	assert(m_precision.length());
	assert(m_component_count != 0);

	m_name = name;

	NGL_texture_descriptor desc;
	desc.m_type = NGL_TEXTURE_2D;
	desc.m_filter = lod_levels > 1 ? NGL_LINEAR_MIPMAPPED : NGL_LINEAR;
	desc.m_wrap_mode = NGL_CLAMP_ALL;

	desc.m_size[0] = width;
	desc.m_size[1] = height;
	desc.m_format = texture_format;
	desc.m_is_renderable = true;
	desc.m_num_levels = lod_levels;
	desc.SetAllClearValue(0.0f);

	{
		desc.m_name = m_name + "::blur_v";
		nglGenTexture(m_temp_texture, desc, nullptr);
		Transitions::Get().Register(m_temp_texture, desc);

		m_vertical_blur->SetPrecision(m_precision.c_str());
		m_vertical_blur->SetComponentCount(m_component_count);
		m_vertical_blur->Init(desc.m_name.c_str(), SeparableBlur::VERTICAL, shapes, input_texture, m_temp_texture, width, height, blur_kernel_size, texture_format, lod_levels);
	}

	{
		desc.m_name = m_name + "::blur_h";
		nglGenTexture(m_output_texture, desc, nullptr);
		Transitions::Get().Register(m_output_texture, desc);

		m_horizontal_blur->SetPrecision(m_precision.c_str());
		m_horizontal_blur->SetComponentCount(m_component_count);
		m_horizontal_blur->Init(desc.m_name.c_str(), SeparableBlur::HORIZONTAL, shapes, m_temp_texture, m_output_texture, width, height, blur_kernel_size, texture_format, lod_levels);
	}
}


void FragmentBlur::Resize(KCL::uint32 blur_kernel_size, KCL::uint32 width, KCL::uint32 height)
{
	KCL::uint32 texture_size[3] = { width, height, 0 };
	KCL::uint32 textures[2] = { m_temp_texture, m_output_texture };

	nglResizeTextures(2, textures, texture_size);

	m_vertical_blur->Resize(blur_kernel_size, 0, 0, width, height);
	m_horizontal_blur->Resize(blur_kernel_size, 0, 0, width, height);
}


void FragmentBlur::DeletePipelines()
{
	m_vertical_blur->DeletePipelines();
	m_horizontal_blur->DeletePipelines();
}


void FragmentBlur::SetKernelSize(KCL::uint32 blur_kernel_size)
{
	m_vertical_blur->SetKernelSize(blur_kernel_size);
	m_horizontal_blur->SetKernelSize(blur_kernel_size);
}


KCL::uint32 FragmentBlur::RenderVerticalPass(KCL::uint32 command_buffer, KCL::uint32 lod_level, bool end_job)
{
	return m_vertical_blur->Render(command_buffer, lod_level, end_job);
}


KCL::uint32 FragmentBlur::RenderHorizontalPass(KCL::uint32 command_buffer, KCL::uint32 lod_level, bool end_job)
{
	return m_horizontal_blur->Render(command_buffer, lod_level, end_job);
}


void FragmentBlur::SetInputTexture(KCL::uint32 in_tex)
{
	m_vertical_blur->SetInputTexture(in_tex);
}


KCL::uint32 FragmentBlur::GetOutputTexture() const
{
	return m_horizontal_blur->GetOutputTexture();
}


void *FragmentBlur::GetUniformOutputTexture()
{
	return m_horizontal_blur->GetUniformOutputTexture();
}


KCL::uint32 FragmentBlur::GetVerticalJob(KCL::uint32 lod_level) const
{
	return m_vertical_blur->GetJob(lod_level);
}


KCL::uint32 FragmentBlur::GetHorizontalJob(KCL::uint32 lod_level) const
{
	return m_horizontal_blur->GetJob(lod_level);
}
