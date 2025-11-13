/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_SCENE_H
#define GFXB5_SCENE_H

#include "common/gfxb_scene_base.h"
#include "common/gfxb_factories.h"
#include "common/gfxb_shader.h"

#include "job_statistic.h"

struct GI2;

namespace GFXB
{
	class Scene5MeshFactory;

	class Shapes;
	class Light;
	class FrustumCull;
	class Scene5MainMeshFilter;
	class Scene5ShadowMeshFilter;
	class Mesh3;
	class SunLight;
	class ComputeMotionBlur;
	class ShadowMap;
	class ComputeHDR;
	class Bloom;
	class BloomMobile;
	class DOF;
	class MeshFactory;
	class DebugRenderer;
	class FragmentBlur;
	class FontRenderer;
	class Environment;
	class LightshaftUtil;

	struct RenderOpts
	{
		enum DebugView
		{
			DEBUG_VIEW_DISABLED = 0,
			DEBUG_VIEW_ALBEDO,
			DEBUG_VIEW_ALPHA,
			DEBUG_VIEW_SPECULAR,
			DEBUG_VIEW_EMISSIVE,
			DEBUG_VIEW_GLOSSNESS,
			DEBUG_VIEW_ROUGHNESS,
			DEBUG_VIEW_NORMALS,
			DEBUG_VIEW_VELOCITY,
			DEBUG_VIEW_DEPTH,
			DEBUG_VIEW_DEPTH_LINEAR,
			DEBUG_VIEW_DEPTH_DOWNSAMPLE,
			DEBUG_VIEW_SSAO,
			DEBUG_VIEW_BRIGHT,
			DEBUG_VIEW_BLOOM,
			DEBUG_VIEW_NEIGHBOR_MAX,
			DEBUG_VIEW_MOTION_BLUR,
			DEBUG_VIEW_DOF,
			DEBUG_VIEW_DOF_INPUT,
			DEBUG_VIEW_HALF_RES_TRANSPARENT,
			DEBUG_VIEW_DIRECT_SHADOW,
			DEBUG_VIEW_INDIRECT_SHADOW,
			DEBUG_VIEW_ENERGY_CONSERVATIVITY_CHECK,
			DEBUG_VIEW_TEXTURE_DENSITY,
			DEBUG_VIEW_VISUALIZE_MIPMAP
		};

		enum Flag
		{
			FLAG_FP_RENDER_TARGETS = 0,
			FLAG_NORMAL_MAPPING,
			FLAG_RENDER_PROBES_ATLAS,
			FLAG_RENDER_PROBES_SH,
			FLAG_RENDER_IRRADIANCE_MESH,
			FLAG_DIRECT_SHADOWS,
			FLAG_SSAO,
			FLAG_MID_RANGE_SSAO,
			FLAG_SSAO_BLUR,
			FLAG_DIRECT_LIGHTING,
			FLAG_IRRADIANCE_LIGHTING,
			FLAG_LIGHTSHAFT,
			FLAG_IBL,
			FLAG_HDR,
			FLAG_BLOOM,
			FLAG_BLOOM_MOBILE,
			FLAG_MOTION_BLUR,
			FLAG_DOF,
			FLAG_DOF_HALF_RES,
			FLAG_PARTICLE_SYSTEMS,
			FLAG_BLUR_TRANSPARENTS,
			FLAG_SHARPEN_FILTER,
			FLAG_GAMMA_CORRECTION,
			FLAG_GAMMA_CORRECTION_FAST,
			FLAG_WIREFRAME,
			FLAG_WIREFRAME_SOLID,
			FLAG_COLORIZE_SHADOW_CASTERS,
			FLAG_COLORIZE_LOD_LEVELS,
			FLAG_FORCE_SHADOW_CASTER_ALL,
			FLAG_GI_USE_TEXTURE_SH_ATLAS,
			NUMBER_OF_FLAGS,
		};
	};

	enum TierLevel
	{
		TIER_INVALID = -1,
		TIER_NORMAL = 0,
		TIER_HIGH,
		NUMBER_OF_TIERS
	};

	class Scene5 : public SceneBase
	{
		static const KCL::uint32 LUT_TEXTURE_SIZE = 128;
		static const KCL::uint32 TRANSPARENT_DOWNSCALE = 2;
		static const KCL::uint32 SSAO_DOWNSCALE = 2;
		static const KCL::uint32 SSAO_DEPTH_MIP_LEVELS = 5;
		static const KCL::uint32 BLOOM_DOWNSCALE = 2; //bloom mobile should use 4!!!!

		static const KCL::uint32 HIGH_MOTION_BLUR_STRENGTH = 8;
		static const KCL::uint32 HIGH_TRANSPARENT_BLUR_STRENGTH = 8;
		static const KCL::uint32 HIGH_SSAO_BLUR_STRENGTH = 18;
		static const KCL::uint32 HIGH_BLOOM_STRENGTH = 9;
		static const KCL::uint32 HIGH_MAX_NUM_ENVPROBES = 64;

		static const KCL::uint32 NORMAL_MOTION_BLUR_STRENGTH = 8;
		static const KCL::uint32 NORMAL_TRANSPARENT_BLUR_STRENGTH = 8;
		static const KCL::uint32 NORMAL_SSAO_BLUR_STRENGTH = 9;
		static const KCL::uint32 NORMAL_BLOOM_STRENGTH = 9;
		static const KCL::uint32 NORMAL_MAX_NUM_ENVPROBES = 32;

	public:
		Scene5();
		~Scene5();

		KCL::KCL_Status Init() override;
		KCL::KCL_Status ReloadShaders() override;
		KCL::KCL_Status Warmup() override;

		void Animate() override;
		void Render() override;
		void Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h) override;

		void InitShadowMaps();

		KCL::TextureFactory &TextureFactory() override;
		GFXB::Mesh3Factory &Mesh3Factory() override;

		void SetViewportWidth(int ww) { m_viewport_width = ww; }
		void SetViewportHeight(int hh) { m_viewport_height = hh; }

		KCL::uint32 GetGBufferColor() const;
		KCL::uint32 GetGBufferSpecular() const;
		KCL::uint32 GetGBufferNormal() const;
		KCL::uint32 GetGBufferEmissive() const;
		KCL::uint32 GetGBufferDepth() const;

		Shapes *GetShapes() const;
		ComputeHDR *GetComputeHDR() const;
		FrustumCull *GetMainCameraFrustumCull() const;
		MeshFilter *GetShadowMeshFilter() const;
		DebugRenderer *GetDebugRenderer() const;
		std::vector<std::vector<KCL::Mesh*>> &GetPartitionMeshes();

		std::vector<ShadowMap*> GetShadowMaps();

		void BeginStatistics();
		void EndStatistics();

		void SetRenderFlag(RenderOpts::Flag flag, bool value);
		bool RenderFlagEnabled(RenderOpts::Flag flag) const;
		bool RenderFlagDisabled(RenderOpts::Flag flag) const;

		static void SetRenderFlag(KCL::uint32 flags, RenderOpts::Flag flag, bool value);
		static bool RenderFlagEnabled(KCL::uint32 flags, RenderOpts::Flag flag);

		void DebugFreezCamera(bool value);
		bool DebugHasFreezCamera();
		void DebugShowRoomPlanes();
		void SetDebugView(RenderOpts::DebugView debug_view);
		RenderOpts::DebugView GetDebugView() const;
		void SetDebugShadowMap(ShadowMap *shadow_map);

		KCL::Vector4D m_texture_density_square;

		std::vector<KCL::Vector3D> m_debug_nodes2;

		struct GI2Data : public KCL::Serializable
		{
			std::vector<KCL::Vector3D> m_position;
			std::vector<KCL::Vector3D> m_sampling_position;
			std::vector<KCL::Vector3D> m_half_extent;

			virtual void Serialize(JsonSerializer& s) override
			{
				s.Serialize("position", m_position);
				s.Serialize("sampling_pos", m_sampling_position);
				if (m_sampling_position.empty())
				{
					m_sampling_position = m_position;
				}
				s.Serialize("half_extent", m_half_extent);
				if (m_half_extent.empty())
				{
					m_half_extent.resize(m_position.size());
					std::fill(m_half_extent.begin(), m_half_extent.end(), KCL::Vector3D(5,5,5));
				}
			}
			virtual std::string GetParameterFilename() const override { return "probes.json"; };
		};

		GI2Data m_selected_probes;

		void UpdateGIStuff(KCL::uint32 command_buffer);
		void UpdateGIProbes(KCL::uint32 command_buffer);
		void RenderDebugProbes(KCL::uint32 command_buffer);

	protected:
		Scheduler *CreateScheduler() override;

		TierLevel m_tier;

		GFXB::TextureFactory *m_texture_factory;
		GFXB::Mesh3Factory *m_mesh3_factory;
		GFXB::MaterialFactory *m_material_factory;
		GFXB::LightFactory *m_light_factory;
		GFXB::ParticleSystemFactory *m_particlesystem_factory;
		GFXB::Scene5MeshFactory *m_mesh_factory;

		Shapes *m_shapes;
		Scene5MainMeshFilter *m_main_mesh_filter;
		Scene5ShadowMeshFilter *m_shadow_mesh_filter;
		FrustumCull *m_main_frustum_cull;
		ComputeHDR *m_compute_hdr;
		Bloom* m_bloom;
		BloomMobile* m_bloom_mobile;
		FragmentBlur *m_ssao_blur;
		ComputeMotionBlur *m_motion_blur;
		DOF *m_dof;
		GI2 *m_gi;
		LightshaftUtil *m_lightshaft_util;
		ParticleSystemManager *m_particlesystem_manager;
		FontRenderer *m_font_renderer;
		DebugRenderer *m_debug_renderer;

		float m_lod1_threshold2;
		float m_lod2_threshold2;

		std::vector<std::vector<KCL::Mesh*>> m_partition_meshes;
		std::vector<KCL::Mesh*> m_emissive_meshes;
		KCL::Actor *m_actor_tanya;
		KCL::Actor *m_actor_golem;
		std::vector<GFXB::ShadowMap*> m_shadow_maps;

		// Command buffer for initialization
		KCL::uint32 m_command_buffer_default;

		// Render jobs
		KCL::uint32 m_transparent_render;
		KCL::uint32 m_ssao_render;
		KCL::uint32 m_tonemapper_render;
		KCL::uint32 m_debug_view_render;
		std::vector<KCL::uint32> m_depth_downsample_renderers; // 1/2, 1/4, 1/8 ...

		// Render targets
		KCL::uint32 m_gbuffer_color_texture;
		KCL::uint32 m_gbuffer_normal_texture;
		KCL::uint32 m_gbuffer_specular_texture;
		KCL::uint32 m_gbuffer_emissive_texture;
		KCL::uint32 m_gbuffer_velocity_texture;
		KCL::uint32 m_gbuffer_depth_texture;
		KCL::uint32 m_depth_downsample_texture;
		KCL::uint32 m_ssao_texture;
		KCL::uint32 m_transparent_texture;
		KCL::uint32 m_lighting_texture;
		KCL::uint32 m_indirect_light_weight_texture;
		KCL::uint32 m_final_texture;

		// Debug render targets
		KCL::uint32 m_mip_texture;
		KCL::uint32 m_debug_texture;

		// Shaders
		KCL::uint32 m_forward_shader;
		KCL::uint32 m_irradiance_light_shader;
		KCL::uint32 m_normalize_irradiance_light_shader;
		KCL::uint32 m_ssao_shader;
		KCL::uint32 m_ssao_apply_shader;
		KCL::uint32 m_omni_light_shader;
		KCL::uint32 m_omni_light_shadow_shader;
		KCL::uint32 m_spot_light_shader;
		KCL::uint32 m_spot_light_shadow_shader;
		KCL::uint32 m_directional_light_shader;
		KCL::uint32 m_directional_light_shadow_shader;
		KCL::uint32 m_directional_light_cube_shader;
		KCL::uint32 m_directional_light_cube_shadow_shader;
		KCL::uint32 m_emissive_light_shader;
		KCL::uint32 m_ibl_shader;
		KCL::uint32 m_tonemapper_manual_exposure_shader;
		KCL::uint32 m_tonemapper_auto_exposure_shader;
		KCL::uint32 m_debug_view_shader;
		KCL::uint32 m_glow_shader;
		KCL::uint32 m_upsample_shader;
		KCL::uint32 m_downsample_depth_shader;
		KCL::uint32 m_downsample_depth_linearize_shader;

		// IBL
		GFXB::Texture *m_sky_texture;
		KCL::uint32 m_integrate_brdf_lut_texture;
		KCL::uint32 m_prefiltered_cubemap_texture;
		KCL::uint32 m_prefiltered_cubemap_size;

		// Engine materials
		KCL::Material *m_fire_material;
		KCL::Material *m_glow_material;

		// Engine meshes
		GFXB::Mesh3 *m_billboard_mesh;

		// Scene uniforms
		KCL::Vector4D m_inv_resolution;
		KCL::Vector4D m_view_pos;
		KCL::Vector4D m_view_dir;
		KCL::Matrix4x4 m_view_projection_inverse;
		KCL::Vector4D m_vs_corners[4];
		KCL::Vector4D m_ws_corners[4];
		KCL::Vector4D m_color_correction;
		KCL::Vector4D m_sharpen_filter;

		// Effect params
		KCL::uint32 m_motion_blur_strength;
		KCL::uint32 m_ssao_blur_strength;
		KCL::uint32 m_dof_strength;
		KCL::uint32 m_bloom_strength;
		KCL::uint32 m_transparent_blur_strength;
		KCL::uint32 m_max_num_envprobes;

		KCL::uint32 m_ssao_blur_normalized_strength;
		KCL::uint32 m_dof_normalized_strength;
		KCL::uint32 m_bloom_normalized_strength;
		KCL::uint32 m_transparent_normalized_blur_strength;

		KCL::uint32 m_render_flags;
		bool m_half_res_transparents_enabled;

		// Debug
		KCL::uint32 m_debug_show_room_planes;
		KCL::Camera2 *m_debug_freezed_camera;
		RenderOpts::DebugView m_debug_view;
		ShadowMap *m_debug_shadow_map;
		float m_wireframe_uniform;

		GFXB::FragmentBlur* m_transparent_blur;

		virtual KCL::KCL_Status OnParamsLoaded() override;

		KCL::KCL_Status InitTierLevel();
		virtual void InitShaders();
		virtual void InitShaderFactory(ShaderFactory *shader_factory);
		void InitDOF();
		void InitIBL();
		void IntegrateBRDFTexture(KCL::uint32 command_buffer);

		virtual void RenderPipeline() = 0;
		void RenderShadow(KCL::uint32 command_buffer);
		void RenderGIProbes(KCL::uint32 command_buffer);
		void RenderCamera(KCL::uint32 job, KCL::Camera2 *camera, std::vector<KCL::Mesh*> &meshes, KCL::uint32 custom_shader = -1);
		void RenderTransparents(KCL::uint32 renderer);
		void RenderTransparentUpsamplePass(KCL::uint32 renderer);
		void RenderDirectLights(KCL::uint32 renderer);
		void RenderIrradianceLights(KCL::uint32 renderer);
		void RenderEmissiveLights(KCL::uint32 renderer);
		void RenderIBL(KCL::uint32 renderer);
		void NormalizeIndirectLighting(KCL::uint32 renderer);
		void DownsampleDepthBuffer(KCL::uint32 command_buffer);
		void RenderHalfResTransparents(KCL::uint32 command_buffer);
		void RenderLightshafts(KCL::uint32 renderer);
		void RenderParticles(KCL::uint32 renderer);
		void RenderSSAO(KCL::uint32 command_buffer);
		void RenderApplySSAO(KCL::uint32 renderer);
		void DispatchHDR(KCL::uint32 command_buffer);
		void RenderBloom(KCL::uint32 command_buffer);
		void RenderTonemapperPass(KCL::uint32 command_buffer);
		void DispatchMotionBlur(KCL::uint32 command_buffer);
		void RenderMotionBlur(KCL::uint32 command_buffer);
		void RenderDOF(KCL::uint32 command_buffer);
		void RenderDebugView(KCL::uint32 command_buffer);
		virtual void BindGBuffer(const void **p) = 0;

		void CreateGIStuff(KCL::uint32 command_buffer);
		void NormalizeEffectParameters();

		void GenRenderTarget(KCL::uint32 &texture, NGL_texture_descriptor &descriptor, std::vector<std::vector<uint8_t> > *datas);
	};
}

#endif