/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_COMPUTE_MOTION_BLUR_H
#define GFXB_COMPUTE_MOTION_BLUR_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <vector>
#include <string>


#define DEBUG_MB_TILE_MAX_UV 0
#define DEBUG_MB_TILE_MAX_BUFFER_INFO 0


namespace GFXB
{
	class Shapes;
	class WarmupHelper;
	class SceneBase;

	class ComputeMotionBlur
	{
		static const KCL::uint32 VELOCITY_BUFFER_DOWNSAMPLE = 2;
	public:
		static const std::string MB_NEIGHBOR_MAX_WGS_NAME;
		static const std::string MB_TILE_MAX_WGS_NAME;

		enum AlgorithmMode
		{
			Adaptive = 0,
			VectorizedAdaptive = 1,
			Vectorized = 2
		};

		ComputeMotionBlur();
		virtual ~ComputeMotionBlur();

		void Init(Shapes *shapes, KCL::uint32 width, KCL::uint32 height, KCL::uint32 sample_count, AlgorithmMode mode);

		void SetInputTextures(KCL::uint32 color_texture, KCL::uint32 velocity_texture, KCL::uint32 depth_texture);
		KCL::uint32 &GetOutputTexture();

		KCL::uint32 ExecuteReduction(KCL::uint32 command_buffer);
		KCL::uint32 ExecuteNeighborMax(KCL::uint32 command_buffer);
		KCL::uint32 RenderMotionBlur(KCL::uint32 command_buffer, KCL::Camera2 *camera);

		void DeletePipelines();
		void Resize(KCL::uint32 width, KCL::uint32 height);

		void WarmupTilemaxCompute(GFXB::WarmupHelper* warmup_helper, KCL::uint32 command_buffer);
		void WarmupNeighbormaxCompute(GFXB::WarmupHelper* warmup_helper, KCL::uint32 command_buffer);
		void InitSceneForWarmup(GFXB::SceneBase* scene, KCL::uint32 time);

		void SetTilemaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper, KCL::uint32 wg_size);
		void SetNeighbormaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper, KCL::uint32 wg_size);
		KCL::uint32 GetTilemaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper) const;
		KCL::uint32 GetNeighbormaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper) const;

		KCL::uint32 GetReductionJob() const;
		KCL::uint32 GetNeighborMaxJob() const;
		KCL::uint32 GetBlurJob() const;
		KCL::uint32 &GetNeighborMaxTexture();
		KCL::Vector4D &GetVelocityParams();
		void SetEnabled(bool enabled);
		bool IsEnabled() const;

	private:
		AlgorithmMode m_mode;

		void LoadTilemaxComputeShader(KCL::uint32 wg_size);
		void LoadNeighbormaxComputeShader(KCL::uint32 wg_size);

		bool m_enabled;

		Shapes *m_shapes;
		KCL::Vector4D m_velocity_params;

		KCL::uint32 m_width;
		KCL::uint32 m_height;

		KCL::uint32 m_k;
		KCL::uint32 m_original_sample_count;
		KCL::uint32 m_sample_count;

		std::vector<KCL::Vector4D> m_tap_offsets;

		KCL::uint32 m_tilemax_buffer;

		KCL::uint32 m_neighbormax_texture_width;
		KCL::uint32 m_neighbormax_texture_height;
		KCL::uint32 m_neighbormax_texture;

		KCL::uint32 m_reduction_workgroup_size;
		KCL::uint32 m_reduction_dispatch_x;
		KCL::uint32 m_reduction_dispatch_y;

		KCL::uint32 m_neighbor_max_workgroup_size;
		KCL::uint32 m_neighbor_max_dispatch_x;
		KCL::uint32 m_neighbor_max_dispatch_y;

		KCL::uint32 m_reduction_job;
		KCL::uint32 m_neighbor_max_job;
		KCL::uint32 m_blur_job;

		KCL::uint32 m_reduction_shader;
		KCL::uint32 m_neighbor_max_shader;
		KCL::uint32 m_blur_shader;

		KCL::uint32 m_input_color_texture;
		KCL::uint32 m_input_velocity_texture;
		KCL::uint32 m_input_depth_texture;
		KCL::uint32 m_output_texture;

		// DEBUG members
#if DEBUG_MB_TILE_MAX_UV
	public:
		KCL::uint32 m_test_texture;
	private:
		KCL::uint32 m_test_texture_clear_job;
		void ResizeTileUVDebug();
#endif

#if DEBUG_MB_TILE_MAX_BUFFER_INFO
		void DumpTileMaxBufferInfo();
#endif

	};
}

#endif
