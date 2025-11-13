/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DOF_BLUR_H
#define DOF_BLUR_H

//#define GFXB_DOF_MERGED_APPLY

#include "gfxb_separable_blur.h"
#include "gfxb_environment.h"
#include "gfxb_shader.h"

#include <vector>

namespace KCL
{
	class Camera2;
}

namespace GFXB
{
	class Shapes;

	class DOF
	{
	public:
		DOF();
		virtual ~DOF();

		void Init(Shapes *shapes, KCL::uint32 width, KCL::uint32 height, bool is_portrait, KCL::uint32 kernel_size, KCL::uint32 input_texture, std::vector<KCL::uint32> output_textures, bool half_res);

		void SetDOFValues(const DOFValues &values);
		void SetOutputBufferId(KCL::uint32 id);

		void Render(KCL::uint32 command_buffer, KCL::Camera2 *camera, KCL::uint32 depth_texture, float focus_distance);
		void SetKernelSize(KCL::uint32 size);
		void Resize(KCL::uint32 kernel_size, KCL::uint32 x, KCL::uint32 y, KCL::uint32 width, KCL::uint32 height);

		void DeletePipelines();

		bool IsHalfResMode() const;

		void SetInputTexture(KCL::uint32 texture);
		KCL::uint32 GetInputTexture() const;
		void *GetUniformInputTexture();

		KCL::uint32 GetDownsampledTexture() const;
		void *GetUniformDownsampledTexture();

		KCL::uint32 GetOuputTexture() const;
		void *GetUniformOutputTexture();

	private:
#ifdef GFXB_DOF_MERGED_APPLY
		class DOFHorizontalBlurFullRes : public SeparableBlur
		{
		protected:
			virtual void CreateShaderDescriptor( ShaderDescriptor &desc, KCL::uint32 kernel_size, KCL::uint32 res ) override
			{
				SeparableBlur::CreateShaderDescriptor( desc, kernel_size, res );

				desc.SetVSFile( "dof.vert" ).AddDefineInt( "DOF_PORTRAIT_MODE", DOF::is_portrait );
				desc.SetFSFile( "dof.frag" ).AddDefineInt( "DOF_HALF_RES", 0).AddDefineInt("DOF_MERGED_APPLY", 1);
			}

			virtual KCL::uint32 GetBlurShader( KCL::uint32 kernel_size ) override
			{
				KCL::uint32 res = 0;
				if( m_lod_levels > 1 )
				{
					res = 1;
				}
				else
				{
					res = m_direction == VERTICAL ? ( DOF::is_portrait ? m_width : m_height ) : ( DOF::is_portrait ? m_height : m_width );
				}

				std::pair<KCL::uint32, KCL::uint32> key( kernel_size, res );
				if( m_blur_shader_cache.find( key ) != m_blur_shader_cache.end() )
				{
					return m_blur_shader_cache.at( key );
				}

				ShaderDescriptor shader_desc;
				CreateShaderDescriptor( shader_desc, kernel_size, res );
				KCL::uint32 s = ShaderFactory::GetInstance()->AddDescriptor( shader_desc );
				m_blur_shader_cache[key] = s;
				return s;
			}

		};
#endif

		std::string m_name;

		GFXB::Shapes* m_shapes;

		bool m_half_res_mode;

		DOFValues m_dof_values;
		KCL::Vector4D m_dof_parameters;

		static bool is_portrait;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		KCL::uint32 m_kernel_size;
		KCL::uint32 m_input_texture;
		KCL::uint32 m_vertical_output_texture;
		KCL::uint32 m_horizontal_output_texture;
		KCL::uint32 m_output_texture;

		SeparableBlur *m_vertical_blur;
		SeparableBlur *m_horizontal_blur;
		KCL::uint32 m_apply_render;
		KCL::uint32 m_apply_shader;

		KCL::uint32 m_half_res_texture;
		KCL::uint32 m_downsample_shader;
		KCL::uint32 m_downsample_render;

		std::vector<KCL::uint32> m_output_textures;

		std::vector<SeparableBlur*> m_horizontal_blurs;
		std::vector<KCL::uint32> m_apply_renders;
	};
}

#endif