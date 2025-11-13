/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FRAGMENT_BLUR_H
#define FRAGMENT_BLUR_H

#include "gfxb_separable_blur.h"

namespace GFXB
{
	class Shapes;

	class FragmentBlur
	{
	public:
		FragmentBlur();
		virtual ~FragmentBlur();

		void SetComponentCount(KCL::uint32 component_count)
		{
			m_component_count = component_count;
		}

		void SetPrecision(const std::string &prec)
		{
			m_precision = prec;
		}

		void Init(const char* name, Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels);

		virtual void Resize(KCL::uint32 blur_kernel_size, KCL::uint32 width, KCL::uint32 height);
		void DeletePipelines();

		KCL::uint32 RenderVerticalPass(KCL::uint32 command_buffer, KCL::uint32 lod_level = 0u, bool end_job = true);
		KCL::uint32 RenderHorizontalPass(KCL::uint32 command_buffer, KCL::uint32 lod_level = 0u, bool end_job = true);

		void SetInputTexture(KCL::uint32 in_tex);
		void SetKernelSize(KCL::uint32 blur_kernel_size);

		KCL::uint32 GetOutputTexture() const;
		void *GetUniformOutputTexture();

		KCL::uint32 GetVerticalJob(KCL::uint32 lod_level = 0u) const;
		KCL::uint32 GetHorizontalJob(KCL::uint32 lod_level = 0u) const;

	protected:
		std::string m_precision;
		KCL::uint32 m_component_count;

		std::string m_name;

		KCL::uint32 m_temp_texture;
		KCL::uint32 m_output_texture;

		SeparableBlur *m_vertical_blur;
		SeparableBlur *m_horizontal_blur;
	};

}

#endif
