/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_OPENGL_LIMITS_H
#define GLB_OPENGL_LIMITS_H

#include <kcl_math3d.h>
#include <kcl_base.h>
#include <sstream>
#include <vector>

class GraphicsContext;

namespace GLB
{
	class ContextLimits
	{
	public:		
		static const KCL::uint32 RES_UNIFORM_BLOCK = 0;
		static const KCL::uint32 RES_IMAGE_UNIFORM = 1;
		static const KCL::uint32 RES_TEXTURE_UNIT = 2;
		static const KCL::uint32 RES_ATOMIC_COUNTER_BUFFER = 3;
		static const KCL::uint32 RES_SHADER_STORAGE_BLOCK = 4;

		struct ShaderLimits
		{		
			KCL::uint32 m_max_resource[5];
		};
		
		// Program stage limits
		ShaderLimits m_vertex_shader_limits;
		ShaderLimits m_tess_control_shader_limits;
		ShaderLimits m_tess_eval_shader_limits;
		ShaderLimits m_geometry_shader_limits;
		ShaderLimits m_fragment_shader_limits;
		ShaderLimits m_compute_shader_limits;

		// Global limits
		KCL::uint32 m_max_combinded_resource[5];

		KCL::uint32 m_max_uniform_block_binding;
		KCL::uint32 m_max_shader_storage_block_binding;
		KCL::uint32 m_max_atomic_counter_binding;
		KCL::uint32 m_max_texture_binding;
		KCL::uint32 m_max_image_binding;

		KCL::uint32 m_max_uniform_block_size;

		KCL::uint32 m_max_compute_work_group_size_x;
		KCL::uint32 m_max_compute_work_group_size_y;
		KCL::uint32 m_max_compute_work_group_size_z;
		KCL::uint32 m_max_compute_work_group_invocations;

		std::string name;

		static ContextLimits *GetLimits(GraphicsContext *ctx);
		static ContextLimits *GetES31Limits();
		static ContextLimits *GetES31_AEP_Limits();

		virtual ~ContextLimits() {}

		KCL::KCL_Status ShaderConformanceTest(GraphicsContext *ctx, KCL::uint32 program);

		static bool IsImageType(KCL::int32 type);
		static bool IsSamplerType(KCL::int32 type);
	private:
		ContextLimits() {}

		struct InteraceQuery
		{
            std::vector<KCL::uint32> m_resource_indices; // The indices in the result of this query
			std::vector<KCL::uint32> m_bindings; // This is only valid for interfaces with binding points
			std::vector<KCL::uint32> m_buffer_sizes; // This is only valid for buffers

			KCL::uint32 m_vertex_shader_ref_count;
			KCL::uint32 m_tess_control_ref_count;
			KCL::uint32 m_tess_eval_shader_ref_count;
			KCL::uint32 m_geometry_shader_ref_count;
			KCL::uint32 m_fragment_shader_ref_count;
			KCL::uint32 m_compute_shader_ref_count;

            InteraceQuery()
            {
                clear();
            }

			void clear()
			{
                m_resource_indices.clear();
				m_bindings.clear();
				m_buffer_sizes.clear();

				m_vertex_shader_ref_count = 0;
				m_tess_control_ref_count = 0;
				m_tess_eval_shader_ref_count = 0;
				m_geometry_shader_ref_count = 0;
				m_fragment_shader_ref_count = 0;
				m_compute_shader_ref_count = 0;
			}
		};

		void QueryInteface(KCL::uint32 program, KCL::uint32 prg_interface, KCL::uint32 idx_count, bool has_tess_shader, bool has_geometry_shader, InteraceQuery &result);

		void QueryUniforms(KCL::uint32 program, KCL::uint32 uniform_count, bool has_tess_shader, bool has_geometry_shader, InteraceQuery &texture_result, InteraceQuery &image_result);

		std::string GetResourceName(KCL::uint32 program, KCL::uint32 prg_interface, KCL::uint32 idx);
		void CheckResourceReferences(KCL::uint32 res_type, InteraceQuery &query, KCL::KCL_Status &result, std::stringstream &sstream);

		const char *GetResourceTypeName(KCL::uint32 type);
		bool HasComputeShader(KCL::uint32 program);

		static void Init();
		static bool s_initialized;
		static ContextLimits s_es31_limits;
		static ContextLimits s_es31_aep_limits;
		static ContextLimits s_gl43_limits;
	};
};

#endif