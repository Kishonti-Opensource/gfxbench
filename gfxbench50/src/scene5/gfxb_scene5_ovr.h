/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_SCENE_OVR_H
#define GFXB5_SCENE_OVR_H

#include "common/gfxb_scene_base.h"
#include "common/gfxb_factories.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_hdr.h"
//#include "common/gfxb_bloom.h"
#include "common/gfxb_bloom_mobile.h"
#include "gfxb_ovr_settings_manager.h"
#include "gfxb_ovr_mesh.h"
#include "gfxb_ovr_mesh_filters.h"

namespace GFXB
{
	class Shapes;
	class FrustumCull;
	class Scene5MainMeshFilter;
	
	class Scene5Ovr : public SceneBase
	{
	public:
		Scene5Ovr();
		~Scene5Ovr();

		KCL::KCL_Status Init() override;
		KCL::KCL_Status ReloadShaders() override;

		void Animate() override;
		void Render() override;
		void Resize( KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h) override;

		KCL::TextureFactory &TextureFactory() override;
		GFXB::Mesh3Factory &Mesh3Factory() override;

		void SetEye(int e) { m_eye = e; }
		void SetUp(KCL::Vector3D e) { m_up = e; }
		void SetWH(unsigned ww, unsigned hh) { m_w = ww; m_h = hh; }

		void ResizeShadowMap();

		GFXB::OvrSettingsManager& GetOvrSettingsManager()
		{
			return m_osm;
		}

	protected:
		Scheduler *CreateScheduler() override;

		void RenderShadow( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderGBuffer( KCL::uint32 job, const KCL::Camera2& cubemap_cam, const KCL::Camera2& cam, void** p );
		void RenderSSAO( KCL::uint32 job, KCL::Camera2& cam, void** p );
		void RenderSkybox( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderAlpha( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderLighting( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderHDR( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderBloom( KCL::uint32 job, const KCL::Camera2& cam, void** p );
		void RenderFinal( KCL::uint32 job, const KCL::Camera2& cam, void** p );

		virtual void GetCommandBufferConfiguration( KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count ) override
		{
			buffers_in_frame = 1;
			prerendered_frame_count = 3;
		}
		virtual void SetCommandBuffers( const std::vector<KCL::uint32> &buffers ) override
		{
			//nop
		}
		virtual KCL::uint32 GetLastCommandBuffer() override
		{
			return 0;
		}

	private:
		int m_eye;
		unsigned m_w, m_h;
		KCL::Vector3D m_up;

		KCL::uint32 m_cmd_buffer;

		GFXB::OvrSettingsManager m_osm;

		OvrMainMeshFilter* m_main_mesh_filter;
		FrustumCull* m_main_frustum_cull;

		GFXB::TextureFactory *m_texture_factory;
		GFXB::Mesh3Factory *m_mesh3_factory;
		GFXB::MaterialFactory *m_material_factory;
		GFXB::LightFactory *m_light_factory;
		//GFXB::ParticleSystemFactory *m_particlesystem_factory;
		GFXB::OvrMeshFactory *m_mesh_factory;

		ComputeHDR* m_hdr_computer;
		//Bloom* m_bloom;
		BloomMobile* m_bloom;
		KCL::Vector4D m_color_correction;

		Shapes *m_shapes;

#ifdef WITH_OVR_SDK
		std::vector<KCL::uint32> m_final_render;
#else
		std::vector<KCL::uint32> m_final_render_left;
		std::vector<KCL::uint32> m_final_render_right;
#endif
		KCL::uint32 m_lighting_render;
		KCL::uint32 m_deferred_render;
		KCL::uint32 m_ssao_render;

		KCL::uint32 m_gbuffer_color_texture;
		KCL::uint32 m_gbuffer_normal_texture;
		KCL::uint32 m_gbuffer_specular_texture;
		//KCL::uint32 m_gbuffer_emissive_texture;
		//KCL::uint32 m_gbuffer_velocity_texture;
		KCL::uint32 m_gbuffer_depth_texture;

		KCL::uint32 m_lighting_tex;
		//KCL::uint32 main_depth_tex;
		static const KCL::uint32 SSAO_DOWNSCALE = 2;
		static const KCL::uint32 SSAO_BLUR_STRENGTH = 5;
		KCL::uint32 m_ssao_texture;
		FragmentBlur* m_ssao_blur;
		KCL::uint32 m_ssao_blur_strength;

		static const KCL::uint32 LUT_TEXTURE_SIZE = 128;
		KCL::uint32 m_integrate_brdf_lut_texture;
		KCL::uint32 m_prefiltered_cubemap_texture;
		KCL::Vector3D m_sunlight_dir;
		KCL::Vector4D m_light_color;
		KCL::int32 m_shadow_texture_viewport[4];
		KCL::uint32 m_shadow_job;
		KCL::uint32 m_shadow_texture;
		KCL::uint32 m_shadow_shaders[3];
		KCL::Matrix4x4 m_shadow_matrix;
		KCL::Matrix4x4 m_bias_matrix;
		KCL::Camera2 m_shadow_camera;

		// Shaders
		KCL::uint32 m_skeletal_gbuffer_shader;
		KCL::uint32 m_gbuffer_shader;
		KCL::uint32 m_gbuffer_tessellated_shader;
		KCL::uint32 m_gbuffer_alpha_shader;
		KCL::uint32 m_lighting_shader;
		KCL::uint32 m_cubemap_shader;
		KCL::uint32 m_fullscreen_shader;
		KCL::uint32 m_ssao_shader;

		void InitShaders();

		void NormalizeEffectParameters();

		void InitOSMDefaults();
	};
}

#endif