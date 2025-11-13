/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SEPARABLE_BLUR_H
#define GFXB_SEPARABLE_BLUR_H

#include <ngl.h>
#include <kcl_base.h>

namespace GFXB
{
	class Shapes;
	class ShaderDescriptor;

	class SeparableBlur
	{
	public:
		const static KCL::uint32 MAX_KS = 64;

		enum Direction
		{
			HORIZONTAL = 0,
			VERTICAL
		};

		SeparableBlur();
		virtual ~SeparableBlur();

		void SetComponentCount(KCL::uint32 component_count)
		{
			m_component_count = component_count;
		}

		void SetPrecision(const std::string &prec)
		{
			m_precision = prec;
		}

		void Init(const char* name, Direction dir, Shapes *shapes, KCL::uint32 input_texture, KCL::uint32 output_texture, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, NGL_format texture_format, KCL::uint32 lod_levels );

		void Resize(KCL::uint32 blur_kernel_size, KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height);
		void DeletePipelines();

		void SetKernelSize(KCL::uint32 blur_kernel_size);
		KCL::uint32 Render(KCL::uint32 command_buffer, KCL::uint32 lod_level = 0u, bool end_job = true);
		KCL::uint32 Render(KCL::uint32 command_buffer, const void **p, KCL::uint32 lod_level = 0u, bool end_job = true);

		Direction GetDirection() const;

		void SetInputTexture(KCL::uint32 in_tex);
		KCL::uint32 GetInputTexture() const;
		void *GetUniformInputTexture();

		KCL::uint32 GetOutputTexture() const;
		void *GetUniformOutputTexture();

		KCL::uint32 GetJob(KCL::uint32 lod_level = 0) const;
		static KCL::uint32 GetTextureComponentCount(KCL::uint32 texture_type);

	protected:
		virtual void CreateShaderDescriptor(ShaderDescriptor &desc, KCL::uint32 kernel_size, KCL::uint32 res);
		virtual void CreateJobDescriptor(NGL_job_descriptor &desc) {}

		virtual KCL::uint32 GetBlurShader(KCL::uint32 kernel_size);

		void ResizeJobs();

		std::string m_name;

		Direction m_direction;

		Shapes *m_shapes;

		KCL::uint32 m_component_count;
		std::string m_precision;
		std::string m_input_mask;
		std::string m_output_mask;
		std::string m_texture_type;
		std::string m_blur_type;

		KCL::uint32 m_x;
		KCL::uint32 m_y;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		KCL::uint32 m_lod_levels;
		KCL::uint32 m_input_texture;
		KCL::uint32 m_output_texture;
		NGL_format m_texture_format;

		KCL::uint32 m_blur_shader;
		std::vector<KCL::uint32> m_blur_jobs;

		std::map<std::pair<KCL::uint32, KCL::uint32>, KCL::uint32 > m_blur_shader_cache;

		// Gauss components
		std::vector<KCL::Vector2D> m_stepuv;
		KCL::uint32 m_blur_kernel_size;
	};
}

#endif