/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5.h"
#include "gfxb_scene5_tools.h"
#include "gfxb_scene5_mesh.h"
#include "gfxb_scene5_mesh_filters.h"
#include "gfxb_room_setup.h"
#include "gfxb_sun_light.h"
#include "gfxb_room_setup.h"
#include "gfxb_lightshaft_util.h"
#include "gfxb_font_renderer.h"
#include "gfxb_barrier.h"

#include "gi/gi.h"

#include "common/gfxb_factories.h"
#include "common/gfxb_light.h"
#include "common/gfxb_lightshaft.h"
#include "common/gfxb_texture.h"
#include "common/gfxb_material.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_mesh_shape.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_cubemap.h"
#include "common/gfxb_environment.h"
#include "common/gfxb_debug_renderer.h"
#include "common/gfxb_hdr.h"
#include "common/gfxb_bloom.h"
#include "common/gfxb_bloom_mobile.h"
#include "common/gfxb_shadow_map.h"
#include "common/gfxb_frustum_cull.h"
#include "common/gfxb_tools.h"
#include "common/gfxb_fragment_blur.h"
#include "common/gfxb_gen_mipmaps.h"
#include "common/gfxb_particlesystem.h"
#include "common/gfxb_compute_motion_blur.h"
#include "common/gfxb_dof.h"
#include "common/gfxb_shot_handler.h"
#include "common/gfxb_bilateral_fragment_blur.h"
#include <ngl.h>

#include <kcl_envprobe.h>

#include <etc1.h>

//#define OVR_GEN
//#include "gfxb_ovr_gen.h"

// This pushes the sky out of the reach of mid-range ssao
#define GFXB_SKY_ZFAR_OFFSET 2.0f

using namespace GFXB;

Scene5::Scene5() : SceneBase(KCL::SV_50)
{
	m_particlesystem_manager = new GFXB::ParticleSystemManager();

	// Create the factories
	m_texture_factory = new GFXB::TextureFactory();
	m_mesh3_factory = new GFXB::Mesh3Factory();
	m_material_factory = new GFXB::MaterialFactory();
	m_light_factory = new GFXB::LightFactory(this);
	m_particlesystem_factory = new GFXB::ParticleSystemFactory(m_particlesystem_manager);
	m_mesh_factory = new Scene5MeshFactory();

	// Register KCL::Object factories
	GetFactoryInstance().RegisterFactory(m_material_factory, KCL::MATERIAL);
	GetFactoryInstance().RegisterFactory(m_light_factory, KCL::LIGHT);
	GetFactoryInstance().RegisterFactory(m_particlesystem_factory, KCL::EMITTER5);
	GetFactoryInstance().RegisterFactory(m_mesh_factory, KCL::MESH);

	if (IsEditorMode())
	{
		INFO("Anisotropic filtering enabled");
		m_texture_factory->SetAnisotropyValue(4);
	}

	m_tier = TIER_INVALID;

	// Render targets
	m_gbuffer_color_texture = 0;
	m_gbuffer_normal_texture = 0;
	m_gbuffer_specular_texture = 0;
	m_gbuffer_emissive_texture = 0;
	m_gbuffer_velocity_texture = 0;
	m_gbuffer_depth_texture = 0;
	m_depth_downsample_texture = 0;
	m_ssao_texture = 0;
	m_lighting_texture = 0;
	m_transparent_texture = 0;
	m_mip_texture = 0;
	m_final_texture = 0;
	m_indirect_light_weight_texture = 0;

	m_sky_texture = nullptr;
	m_integrate_brdf_lut_texture = 0;
	m_prefiltered_cubemap_texture = 0;
	m_prefiltered_cubemap_size = 0;

	// Command buffers
	m_command_buffer_default = 0;

	// Render jobs
	m_ssao_render = 0;
	m_transparent_render = 0;
	m_tonemapper_render = 0;
	m_debug_view_render = 0;

	// Shaders
	m_forward_shader = 0;
	m_irradiance_light_shader = 0;
	m_normalize_irradiance_light_shader = 0;
	m_ssao_shader = 0;
	m_ssao_apply_shader = 0;
	m_omni_light_shader = 0;
	m_omni_light_shadow_shader = 0;
	m_spot_light_shader = 0;
	m_spot_light_shadow_shader = 0;
	m_directional_light_shader = 0;
	m_directional_light_shadow_shader = 0;
	m_directional_light_cube_shader = 0;
	m_directional_light_cube_shadow_shader = 0;
	m_emissive_light_shader = 0;
	m_ibl_shader = 0;
	m_tonemapper_manual_exposure_shader = 0;
	m_tonemapper_auto_exposure_shader = 0;
	m_debug_view_shader = 0;
	m_glow_shader = 0;
	m_upsample_shader = 0;
	m_downsample_depth_shader = 0;
	m_downsample_depth_linearize_shader = 0;

	// Scene components
	m_main_mesh_filter = nullptr;
	m_main_frustum_cull = nullptr;
	m_shadow_mesh_filter = nullptr;
	m_shapes = nullptr;
	m_compute_hdr = nullptr;
	m_bloom = nullptr;
	m_bloom_mobile = nullptr;
	m_transparent_blur = nullptr;
	m_motion_blur = nullptr;
	m_dof = nullptr;
	m_gi = nullptr;
	m_ssao_blur = nullptr;
	m_lightshaft_util = nullptr;

	// Effect params
	m_motion_blur_strength = 0;
	m_dof_strength = 0;
	m_bloom_strength = 0;
	m_transparent_blur_strength = 0;
	m_ssao_blur_strength = 0;
	m_max_num_envprobes = 0;

	m_ssao_blur_normalized_strength = 0;
	m_dof_normalized_strength = 0;
	m_bloom_normalized_strength = 0;
	m_transparent_normalized_blur_strength = 0;

	m_actor_tanya = nullptr;
	m_actor_golem = nullptr;
	m_fire_material = nullptr;
	m_glow_material = nullptr;
	m_billboard_mesh = nullptr;

	// Debug
	m_debug_view = RenderOpts::DEBUG_VIEW_DISABLED;
	m_debug_renderer = nullptr;
	m_debug_texture = 0;
	m_debug_freezed_camera = nullptr;
	m_debug_shadow_map = nullptr;
	m_debug_show_room_planes = 0;
	m_wireframe_uniform = 0.0f;

	m_font_renderer = nullptr;

	m_lod1_threshold2 = 0.0f;
	m_lod2_threshold2 = 0.0f;

	m_render_flags = KCL::uint32(~0);
	SetRenderFlag(RenderOpts::FLAG_GAMMA_CORRECTION_FAST, true);
	SetRenderFlag(RenderOpts::FLAG_FORCE_SHADOW_CASTER_ALL, false);

	SetRenderFlag(RenderOpts::FLAG_BLOOM_MOBILE, false);
	SetRenderFlag(RenderOpts::FLAG_DOF_HALF_RES, false);

	// Enable/disable direct lights
	SetRenderFlag(RenderOpts::FLAG_DIRECT_LIGHTING, true);

	// Enable/disable image based lighting
	SetRenderFlag(RenderOpts::FLAG_IBL, false);

	// Enable/disable irradiance lighting
	SetRenderFlag(RenderOpts::FLAG_IRRADIANCE_LIGHTING, true);

	// Enable/disable shadows
	SetRenderFlag(RenderOpts::FLAG_DIRECT_SHADOWS, true);

	// Enable/disable SSAO
	SetRenderFlag(RenderOpts::FLAG_SSAO, true);

	// Enable/disable secondary, mid-range SSAO
	SetRenderFlag(RenderOpts::FLAG_MID_RANGE_SSAO, true);

	// Enable/disable SSAO blur
	SetRenderFlag(RenderOpts::FLAG_SSAO_BLUR, true);

	// Enable/disable paritcle system
	SetRenderFlag(RenderOpts::FLAG_PARTICLE_SYSTEMS, true);

	// Enable/disable lightshafts
	SetRenderFlag(RenderOpts::FLAG_LIGHTSHAFT, true);

	// Enable/disable transparent blur
	SetRenderFlag(RenderOpts::FLAG_BLUR_TRANSPARENTS, true);

	// Enable/disable motion blur
	SetRenderFlag(RenderOpts::FLAG_MOTION_BLUR, true);

	// Enable/disable depth of field
	SetRenderFlag(RenderOpts::FLAG_DOF, true);

	// Enable/disable irradiance probe debug rendering
	SetRenderFlag(RenderOpts::FLAG_RENDER_PROBES_ATLAS, false);
	SetRenderFlag(RenderOpts::FLAG_RENDER_PROBES_SH, false);

	// Enable/disable wireframe render
	SetRenderFlag(RenderOpts::FLAG_WIREFRAME, false);
	SetRenderFlag(RenderOpts::FLAG_WIREFRAME_SOLID, false);

	// Colorize shadow casters per light frustums
	SetRenderFlag(RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS, false);

	// Colorize mesh lod levels
	SetRenderFlag(RenderOpts::FLAG_COLORIZE_LOD_LEVELS, false);

	// Enable/disable irradiance mesh render
	SetRenderFlag(RenderOpts::FLAG_RENDER_IRRADIANCE_MESH, false);

	m_texture_density_square.x = 64 * 64;
	m_texture_density_square.y = 128 * 128;
	m_texture_density_square.z = 512 * 512;

	//ovr_gen::constructor(this);
}


Scene5::~Scene5()
{
    if (m_compute_hdr != nullptr)
    {
        m_compute_hdr->SaveLuminanceValues(GetPredefinedLuminanceFilename(m_tier_level_name));
    }

	ShaderFactory::Release();

	RoomSetup::CleanUp(this);

	delete m_texture_factory;
	delete m_mesh3_factory;
	delete m_material_factory;
	delete m_light_factory;
	delete m_particlesystem_factory;
	delete m_mesh_factory;

	m_particlesystem_manager->UnregisterAllParticleEmitters();
	delete m_particlesystem_manager;

	delete m_lightshaft_util;

	delete m_main_mesh_filter;
	delete m_main_frustum_cull;
	delete m_shapes;
	delete m_shadow_mesh_filter;
	delete m_compute_hdr;
	delete m_bloom;
	delete m_bloom_mobile;
	delete m_gi;
	delete m_transparent_blur;
	delete m_ssao_blur;
	delete m_motion_blur;
	delete m_dof;
	delete m_font_renderer;

	delete m_debug_renderer;
	delete m_debug_freezed_camera;

	for (size_t i = 0; i < m_shadow_maps.size(); i++)
	{
		delete m_shadow_maps[i];
	}

	for (size_t i = 0; i < m_partition_meshes.size(); i++)
	{
		for (size_t j = 0; j < m_partition_meshes[i].size(); j++)
		{
			delete m_partition_meshes[i][j];
		}
	}
}


void Scene5::Animate()
{
	if (RenderFlagEnabled(RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS))
	{
		for (size_t r = 0; r < m_rooms.size(); r++)
		{
			for (size_t m = 0; m < m_rooms[r]->m_meshes.size(); m++)
			{
				((Mesh*)m_rooms[r]->m_meshes[m])->ClearFrustumIds();
			}
		}

		for (size_t a = 0; a < m_actors.size(); a++)
		{
			for (size_t m = 0; m < m_actors[a]->m_meshes.size(); m++)
			{
				((Mesh*)m_actors[a]->m_meshes[m])->ClearFrustumIds();
			}
		}
	}

	SceneBase::Animate();

	const Environment::Values &env_values = GetEnvironment()->GetValues();

	//ovr_gen::animate(m_fps_camera, &m_active_camera);

	m_main_mesh_filter->SetAnimationTime(m_animation_time);

	// Place actors to rooms
	{
		KCL::XRoom *room;
		KCL::Actor *actor;
		for (size_t j = 0; j < m_rooms.size(); j++)
		{
			room = m_rooms[j];
			if (room->m_planes.empty())
			{
				continue;
			}

			room->m_actors.clear();
			for (size_t i = 0; i < m_actors.size(); i++)
			{
				actor = m_actors[i];
				if (KCL::XRoom::OVERLAP(room->m_planes.data(), (KCL::uint32)room->m_planes.size(), &actor->m_aabb) != KCL::OVERLAP_OUTSIDE)
				{
					room->m_actors.push_back(actor);
				}
			}
		}
	}

	// Main camera frustum cull
	if (m_debug_freezed_camera)
	{
		m_main_frustum_cull->Cull(m_debug_freezed_camera);
		m_active_camera->SetNearFar(0.1f, 10000.0f);
	}
	else
	{
		m_main_frustum_cull->Cull(m_active_camera);
		// push back the far plane to have some distance between the sky and the farthest drawn geometry
		m_active_camera->SetNearFar(m_main_frustum_cull->GetNear(), m_main_frustum_cull->GetFar() + GFXB_SKY_ZFAR_OFFSET);
	}

	m_active_camera->Update();

	if (RenderFlagEnabled(RenderOpts::FLAG_RENDER_IRRADIANCE_MESH))
	{
		std::vector<KCL::Mesh*> &dest = m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE];
		dest.insert(dest.begin(), m_irradiance_meshes.begin(), m_irradiance_meshes.end());
	}

	if (m_debug_show_room_planes)
	{
		m_main_frustum_cull->m_visible_meshes[0].clear();
		m_main_frustum_cull->m_visible_meshes[0].insert(m_main_frustum_cull->m_visible_meshes[0].end(), m_partition_meshes[m_debug_show_room_planes].begin(),
			m_partition_meshes[m_debug_show_room_planes].end());
	}

	// Calculate scene uniforms
	{
		//m_sun_light->Update();

		KCL::Matrix4x4::Invert4x4(m_active_camera->GetViewProjection(), m_view_projection_inverse);

		m_view_pos.set(m_active_camera->GetEye(), 1.0f);

		KCL::Vector3D camera_dir(-m_active_camera->GetView().v[2], -m_active_camera->GetView().v[6], -m_active_camera->GetView().v[10]);
		camera_dir.normalize();
		m_view_dir.set(camera_dir, 0.0f);

		KCL::Vector3D ws_corners[4];
		//NOTE: CalculateRaysToFullscreenBillboard(..., true) has bugs!
		m_active_camera->CalculateRaysToFullscreenBillboard(ws_corners, false);
		for (KCL::uint32 i = 0; i < 4; i++)
		{
			m_ws_corners[i].set(ws_corners[i], 0.0f);
		}

		m_inv_resolution.set(float(1.0f) / float(m_viewport_width), float(1.0f) / float(m_viewport_height), float(m_viewport_width), float(m_viewport_height));

		m_active_camera->CalculateVSRaysToFullscreenBillboard(m_vs_corners);

		// LOD^2 distances for the main camera
		const LODValues &lod_values = env_values.m_lod_values;
		m_lod1_threshold2 = lod_values.m_lod1_distance * lod_values.m_lod1_distance;
		m_lod2_threshold2 = lod_values.m_lod2_distance * lod_values.m_lod2_distance;

		{
			const ColorCorrectionValues &cc_values = env_values.m_color_correction_values;
			// Saturation
			m_color_correction.x = cc_values.m_saturation;

			// Color scale
			m_color_correction.y = cc_values.m_contrast;

			// Color bias
			float complementer = 1.0f - cc_values.m_contrast;
			m_color_correction.z = KCL::Math::interpolate(0.0f, complementer, cc_values.m_contrast_center);

			const SharpenValues &sharpen_values = env_values.m_sharpen_values;
			m_sharpen_filter.x = sharpen_values.m_strength;
			m_sharpen_filter.y = sharpen_values.m_strength * 0.25f;
			m_sharpen_filter.z = sharpen_values.m_limit;
			//ovr_gen::set_sharpen_values(m_sharpen_filter);
		}

		NormalizeEffectParameters();
	}

	{
		m_half_res_transparents_enabled = RenderFlagEnabled(RenderOpts::FLAG_LIGHTSHAFT) || RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS);
	}

	if (RenderFlagEnabled(RenderOpts::FLAG_HDR))
	{
		HDRValues hdr_values = env_values.m_hdr_values;

		if (m_hdr_adaptation_mode == ADAPTATION_PREDEFINED)
		{
			hdr_values.m_exposure_mode = EXPOSURE_PREDEFINED;
		}
		else if (m_hdr_adaptation_mode == ADAPTATION_DISABLED
			&& env_values.m_hdr_values.m_exposure_mode == EXPOSURE_ADAPTIVE)
		{
			hdr_values.m_exposure_mode = EXPOSURE_AUTO;
		}

		m_compute_hdr->Animate(hdr_values, m_animation_time);
	}

	// Shadow
	m_shadow_mesh_filter->SetForceShadowCasterAll(RenderFlagEnabled(RenderOpts::FLAG_FORCE_SHADOW_CASTER_ALL));

	// GI
	if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		{
			m_gi->m_lights.clear();

			for (size_t i = 0; i < m_actors.size(); i++)
			{
				for (size_t j = 0; j < m_actors[i]->m_lights.size(); j++)
				{
					KCL::Light *light = m_actors[i]->m_lights[j];
					if (light->m_visible)
					{
						m_gi->m_lights.push_back(light);
					}
				}
			}
		}

		m_gi->FrustumCull(m_active_camera);

		m_gi->m_render_shadows = RenderFlagEnabled(RenderOpts::FLAG_DIRECT_SHADOWS);
		m_gi->m_sky_color = KCL::Vector4D(env_values.m_gi_values.m_sky_color, 0.0f) * env_values.m_gi_values.m_sky_intensity;
		m_gi->m_indirect_lighting_factor = env_values.m_gi_values.m_indirect_lighting_factor;
	}

	if (RenderFlagEnabled(RenderOpts::FLAG_MOTION_BLUR))
	{
		bool motion_blur_enabled = true;

		if (m_animation_time < PREV_MVP_INTERVAL)
		{
			// Disable motion blur for the first PREV_MVP_INTERVAL seconds, as we don't have correct "prev mvp" matrices
			motion_blur_enabled = false;
		}
		else if ((GetSingleFrame() == -1 && GetCameraShotHandler()->IsCameraShotChanged()) || GetCameraShotHandler()->GetCurrentCameraShotIndex() != GetCameraShotHandler()->GetShotId(m_animation_time - PREV_MVP_INTERVAL))
		{
			// Disable motion blur on camera changes
			motion_blur_enabled = false;
		}

		// Force motion blur during warmup
		motion_blur_enabled |= IsWarmup();

		m_motion_blur->SetEnabled(motion_blur_enabled);
	}

	// Particle Emitters
	if (RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS))
	{
		m_particlesystem_manager->AnimateAll(m_animation_time, m_particle_save_frames);
	}

	for (size_t i = 0; i < m_shadow_maps.size(); i++)
	{
		m_shadow_maps[i]->Animate(m_animation_time);
	}
}


void Scene5::Render()
{
	BeginStatistics();

	RenderPipeline();

	EndStatistics();

	if (IsEditorMode())
	{
		RenderDebugView(GetLastCommandBuffer());
	}

	//ovr_gen::write_to_file(m_shapes);
}


void Scene5::RenderShadow(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_DIRECT_SHADOWS) || IsWarmup())
	{
		return;
	}

	for (size_t i = 0; i < m_main_frustum_cull->m_visible_lights.size(); i++)
	{
		Light *light = (Light*)m_main_frustum_cull->m_visible_lights[i];
		if (light->m_light_shape->m_is_shadow_caster)
		{
			light->GetShadowMap()->IncludeActorsOnUpdate();
		}
	}

	for (size_t i = 0; i < m_actors.size(); i++)
	{
		for (size_t j = 0; j < m_actors[i]->m_lights.size(); j++)
		{
			Light* light = (Light*)(m_actors[i]->m_lights[j]);
			if (light->m_visible && light->m_light_shape->m_is_shadow_caster)
			{
				light->GetShadowMap()->Render(command_buffer);
			}
		}
	}
}


void Scene5::RenderGIProbes(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		return;
	}

	m_gi->direct_light_evaluation(command_buffer);

	m_gi->envprobe_lightmap_atlas_transfer(command_buffer, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, m_main_frustum_cull->m_visible_probes);

	m_gi->envprobe_generate_sh(command_buffer, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, m_main_frustum_cull->m_visible_probes);
}


void Scene5::DownsampleDepthBuffer(KCL::uint32 command_buffer)
{
	const bool ssao_enabled = RenderFlagEnabled(RenderOpts::FLAG_SSAO);
	const bool dof_half_res = RenderFlagEnabled(RenderOpts::FLAG_DOF_HALF_RES);

	if (!m_half_res_transparents_enabled && !ssao_enabled && !dof_half_res)
	{
		return;
	}

	NGL_api api = nglGetApi();
	bool bind_lod_zero = (api == NGL_DIRECT3D_11) || (api == NGL_DIRECT3D_12);

	// Downsample and linearize to the first mipmap level
	KCL::Vector4D offsets = KCL::Vector4D(0.0f, 0.0f, 0.0f, 0.0f);
	{
		Transitions &transitions = Transitions::Get()
			.TextureBarrier(m_gbuffer_depth_texture, NGL_SHADER_RESOURCE)
			.TextureMipLevelBarrier(m_depth_downsample_texture, 0, NGL_COLOR_ATTACHMENT);
		if (ssao_enabled)
		{
			for (KCL::uint32 i = 1; i < SSAO_DEPTH_MIP_LEVELS; i++)
			{
				transitions.TextureMipLevelBarrier(m_depth_downsample_texture, i, NGL_COLOR_ATTACHMENT);
			}
		}
		transitions.Execute(command_buffer);

		offsets.x = 1.0f / float(m_viewport_width);
		offsets.y = 1.0f / float(m_viewport_height);
		float lod_level = 0.0f;

		NGL_texture_subresource subresource(m_gbuffer_depth_texture);

		const void *p[UNIFORM_MAX];
		p[UNIFORM_INPUT_TEXTURE_LOD] = &subresource;
		p[UNIFORM_TAP_OFFSETS] = offsets.v;
		p[UNIFORM_LOD_LEVEL] = &lod_level;
		p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;

		nglBegin(m_depth_downsample_renderers[0], command_buffer);
		nglDraw(m_depth_downsample_renderers[0], NGL_TRIANGLES, m_downsample_depth_linearize_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p);
		nglEnd(m_depth_downsample_renderers[0]);
	}

	// Create the other mipmap levels
	if (ssao_enabled)
	{
		NGL_texture_subresource subresource(m_depth_downsample_texture);

		float lod_level;

		const void *p[UNIFORM_MAX];
		p[UNIFORM_INPUT_TEXTURE_LOD] = &subresource;
		p[UNIFORM_TAP_OFFSETS] = offsets.v;
		p[UNIFORM_LOD_LEVEL] = &lod_level;

		for (KCL::uint32 level = 1; level < SSAO_DEPTH_MIP_LEVELS; level++)
		{
			KCL::uint32 input_level = level - 1;

			KCL::uint32 width = m_viewport_width / (1 << level);
			KCL::uint32 height = m_viewport_height / (1 << level);
			width = KCL::Max(width, 1u);
			height = KCL::Max(height, 1u);

			subresource.m_level = input_level;

			lod_level = (float)input_level;
			if (bind_lod_zero)
			{
				lod_level = 0.0f;
			}

			Transitions::Get()
				.TextureMipLevelBarrier(m_depth_downsample_texture, input_level, NGL_SHADER_RESOURCE)
				.Execute(command_buffer);

			nglBegin(m_depth_downsample_renderers[level], command_buffer);
			nglDraw(m_depth_downsample_renderers[level], NGL_TRIANGLES, m_downsample_depth_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p);
			nglEnd(m_depth_downsample_renderers[level]);

			offsets.x = 1.0f / float(width);
			offsets.y = 1.0f / float(height);
		}
	}
}


void Scene5::RenderHalfResTransparents(KCL::uint32 command_buffer)
{
	if (m_half_res_transparents_enabled == false)
	{
		return;
	}

	Transitions &transition = Transitions::Get();
	for (KCL::uint32 i = 0; i < SSAO_DEPTH_MIP_LEVELS; i++)
	{
		transition.TextureMipLevelBarrier(m_depth_downsample_texture, i, NGL_SHADER_RESOURCE);
	}

	transition
		.TextureBarrier(m_transparent_texture, NGL_COLOR_ATTACHMENT)
		.Execute(command_buffer);

	KCL::uint32 particle_renderer = m_transparent_render;

	nglBegin(m_transparent_render, command_buffer);

	// Render lightshafts
	if (RenderFlagEnabled(RenderOpts::FLAG_LIGHTSHAFT))
	{
		RenderLightshafts(m_transparent_render);

		// Blur the lightshafts
		if (RenderFlagEnabled(RenderOpts::FLAG_BLUR_TRANSPARENTS))
		{
			// Close the lightshaft renderer
			nglEnd(m_transparent_render);

			// Execute the blur
			m_transparent_blur->RenderVerticalPass(command_buffer);
			particle_renderer = m_transparent_blur->RenderHorizontalPass(command_buffer, 0, false);

			nglBlendState(particle_renderer, 0, NGL_BLEND_TRANSPARENT_ACCUMULATION, NGL_CHANNEL_ALL);
		}
	}

	// Render the particles
	RenderParticles(particle_renderer);

	// Close the transparent or blur renderer
	nglEnd(particle_renderer);
}


void Scene5::RenderSSAO(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_SSAO))
	{
		return;
	}

	Transitions &transition = Transitions::Get();
	for (KCL::uint32 i = 0; i < SSAO_DEPTH_MIP_LEVELS; i++)
	{
		transition.TextureMipLevelBarrier(m_depth_downsample_texture, i, NGL_SHADER_RESOURCE);
	}

	transition
		.TextureBarrier(m_gbuffer_normal_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_ssao_texture, NGL_COLOR_ATTACHMENT)
		.Execute(command_buffer);

	KCL::Vector4D inv_resolution(float(SSAO_DOWNSCALE) / float(m_viewport_width), float(SSAO_DOWNSCALE) / (float(m_viewport_height)), 0.0f, 0.0f);
	float SAO_projection_scale = (float)((m_viewport_width * 0.5f) / (2.0f * tan(KCL::Math::Rad(m_active_camera->GetFov() * 0.5f))));

	const void *p[UNIFORM_MAX];
	p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_depth_downsample_texture;
	p[UNIFORM_GBUFFER_NORMAL_TEX] = &m_gbuffer_normal_texture;
	p[UNIFORM_CORNERS] = m_vs_corners;
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;
	p[UNIFORM_INV_RESOLUTION] = inv_resolution.v;
	p[UNIFORM_VIEW] = (void*)m_active_camera->GetView().v;
	p[UNIFORM_SAO_PROJECTION_SCALE] = &SAO_projection_scale;

	nglBegin(m_ssao_render, command_buffer);

	nglDrawTwoSided(m_ssao_render, m_ssao_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);

	nglEnd(m_ssao_render);

	// blur final texture
	if (RenderFlagEnabled(RenderOpts::FLAG_SSAO_BLUR))
	{
		m_ssao_blur->SetInputTexture(m_ssao_texture);
		m_ssao_blur->RenderVerticalPass(command_buffer);
		m_ssao_blur->RenderHorizontalPass(command_buffer);
	}
}


void Scene5::RenderApplySSAO(KCL::uint32 renderer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_SSAO))
	{
		return;
	}

	KCL::uint32 input_texture = RenderFlagEnabled(RenderOpts::FLAG_SSAO_BLUR) ? m_ssao_blur->GetOutputTexture() : m_ssao_texture;

	const void *p[UNIFORM_MAX];
	p[UNIFORM_SSAO_TEXTURE] = &input_texture;

	nglDrawTwoSided(renderer, m_ssao_apply_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
}


void Scene5::RenderTransparents(KCL::uint32 renderer)
{
	// Render transparent meshes
	nglBlendStateAll(renderer, NGL_BLEND_ADDITIVE_INVERSE_ALFA);
	nglDepthState(renderer, NGL_DEPTH_LEQUAL, false);
	RenderCamera(renderer, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_TRANSPARENT]);

	// Upsample the half resolution transparent texture
	if (m_half_res_transparents_enabled)
	{
		nglBlendState(renderer, 0, NGL_BLEND_ADDITIVE_ALFA, NGL_CHANNEL_ALL);
		nglDepthState(renderer, NGL_DEPTH_DISABLED, false);
		RenderTransparentUpsamplePass(renderer);
	}
}


void Scene5::NormalizeIndirectLighting(KCL::uint32 renderer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		return;
	}

	const void *p[UNIFORM_MAX];

	memset(p, 0, sizeof(void*)*UNIFORM_MAX);

	p[UNIFORM_INDIRECT_WEIGHT_TEXTURE] = &m_indirect_light_weight_texture;

	nglDepthState(renderer, NGL_DEPTH_DISABLED, false);

	nglDrawTwoSided(renderer, m_normalize_irradiance_light_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
}


void Scene5::RenderIrradianceLights(KCL::uint32 renderer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		return;
	}

	NGL_buffer_subresource envprobe_sh_atlas_bsr(m_gi->m_envprobe_sh_atlas);
	KCL::Matrix4x4 light_pom;
	KCL::Matrix4x4 mvp;
	KCL::Vector4D envprobe_inv_half_extent;
	KCL::Vector2D inv_resolution(1.0f / (float)m_viewport_width, 1.0f / (float)m_viewport_height);
	float envprobe_index;
	const void *p[UNIFORM_MAX];
	GIValues giv = GetEnvironment()->GetValues().m_gi_values;

	memset(p, 0, sizeof(void*)*UNIFORM_MAX);

	BindGBuffer(p);

	p[UNIFORM_VIEW_POS] = m_view_pos.v;
	p[UNIFORM_VIEW_DIR] = m_view_dir.v;
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;
	p[UNIFORM_MODEL] = light_pom.v;
	p[UNIFORM_MVP] = mvp.v;
	p[UNIFORM_INV_RESOLUTION] = inv_resolution.v;
	p[UNIFORM_INDIRECT_LIGHTING_FACTOR] = &m_gi->m_indirect_lighting_factor;
	p[UNIFORM_envprobe_envmap_atlas] = &m_gi->m_envprobe_envmap_atlas;
	p[UNIFORM_envprobe_index] = &envprobe_index;
	p[UNIFORM_SSAO_TEXTURE] = &m_ssao_texture;
	p[UNIFORM_envprobe_inv_half_extent] = envprobe_inv_half_extent.v;
	p[UNIFORM_BRDF] = &m_integrate_brdf_lut_texture;
	p[UNIFORM_ENVMAP0] = &m_prefiltered_cubemap_texture;
	p[UNIFORM_IBL_REFLECTION_INTENSITY] = &giv.m_ibl_reflection_intensity;

	if (RenderFlagEnabled(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS))
	{
		p[UNIFORM_envprobe_sh_atlas_texture] = &m_gi->m_envprobe_sh_atlas_texture;
	}
	else
	{
		p[UNIFORM_envprobe_sh_atlas] = &envprobe_sh_atlas_bsr;
	}

	nglDepthState(renderer, NGL_DEPTH_GREATER, false);

	for (size_t i = 0; i < m_main_frustum_cull->m_visible_inside_probes.size(); i++)
	{
		KCL::EnvProbe *ep = m_main_frustum_cull->m_visible_inside_probes[i];

		envprobe_index = (float)ep->m_index;

		envprobe_sh_atlas_bsr.m_offset = sizeof(KCL::Vector4D) * 16 * ep->m_index;
		envprobe_sh_atlas_bsr.m_size = sizeof(KCL::Vector4D) * 9;

		light_pom.identity();
		light_pom.translate(ep->m_pos);
		light_pom.scale(ep->m_half_extent);

		mvp = light_pom * m_active_camera->GetViewProjection();
		envprobe_inv_half_extent.x = 1.0f / ep->m_half_extent.x;
		envprobe_inv_half_extent.y = 1.0f / ep->m_half_extent.y;
		envprobe_inv_half_extent.z = 1.0f / ep->m_half_extent.z;

		p[UNIFORM_LIGHT_POS] = ep->m_pos.v;

		nglDrawBackSided(renderer, m_irradiance_light_shader, m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, p);
	}

	nglDepthState(renderer, NGL_DEPTH_LESS, false);

	for (size_t i = 0; i < m_main_frustum_cull->m_visible_outside_probes.size(); i++)
	{
		KCL::EnvProbe *ep = m_main_frustum_cull->m_visible_outside_probes[i];

		envprobe_index = (float)ep->m_index;

		envprobe_sh_atlas_bsr.m_offset = sizeof(KCL::Vector4D) * 16 * ep->m_index;
		envprobe_sh_atlas_bsr.m_size = sizeof(KCL::Vector4D) * 9;

		light_pom.identity();
		light_pom.translate(ep->m_pos);
		light_pom.scale(ep->m_half_extent);

		mvp = light_pom * m_active_camera->GetViewProjection();
		envprobe_inv_half_extent.x = 1.0f / ep->m_half_extent.x;
		envprobe_inv_half_extent.y = 1.0f / ep->m_half_extent.y;
		envprobe_inv_half_extent.z = 1.0f / ep->m_half_extent.z;

		p[UNIFORM_LIGHT_POS] = ep->m_pos.v;

		nglDrawFrontSided(renderer, m_irradiance_light_shader, m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, p);
	}
}


void Scene5::RenderDirectLights(KCL::uint32 renderer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_DIRECT_LIGHTING))
	{
		return;
	}

	KCL::Matrix4x4 mvp;

	const void *p[UNIFORM_MAX];
	BindGBuffer(p);

	// Camera
	p[UNIFORM_VIEW_POS] = m_view_pos.v;
	p[UNIFORM_VIEW_DIR] = m_view_dir.v;
	p[UNIFORM_INV_VP] = m_view_projection_inverse.v;
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;

	p[UNIFORM_MVP] = mvp.v;

	nglDepthState(renderer, NGL_DEPTH_GREATER, false);

	const bool render_shadows = RenderFlagEnabled(RenderOpts::FLAG_DIRECT_SHADOWS);

	for (size_t i = 0; i < m_main_frustum_cull->m_visible_lights.size(); i++)
	{
		Light *light = (Light*)m_main_frustum_cull->m_visible_lights[i];

		const KCL::LightShape* light_shape = light->m_light_shape;

		const bool shadow_caster = light_shape->m_is_shadow_caster && render_shadows;

		if (light->GetShadowMap() != nullptr)
		{
			p[UNIFORM_SHADOW_MAP] = light->GetShadowMap()->GetUniformShadowTexture();
			p[UNIFORM_SHADOW_MATRIX] = light->GetShadowMap()->GetUniformShadowMatrix();
			p[UNIFORM_SHADOW_LIGHT_POS] = light->GetShadowMap()->GetLight()->m_uniform_pos.v;
		}

		// Light uniforms
		p[UNIFORM_LIGHT_DIR] = light->m_uniform_dir.v;
		p[UNIFORM_LIGHT_COLOR] = light->m_uniform_color.v;
		p[UNIFORM_LIGHT_POS] = light->m_uniform_pos.v;
		p[UNIFORM_MODEL] = light->m_uniform_shape_world_pom.v;
		p[UNIFORM_ATTENUATION_PARAMETERS] = light->m_uniform_attenuation_parameters.v;
		p[UNIFORM_SPOT_COS] = light->m_uniform_spot_cos.v;

		switch (light_shape->m_light_type)
		{
		case KCL::LightShape::DIRECTIONAL:
		{
			if( light_shape->m_box_light )
			{
				mvp = m_active_camera->GetViewProjection();
				nglDrawBackSided( renderer, shadow_caster ? m_directional_light_cube_shadow_shader : m_directional_light_cube_shader, m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, p );
			}
			else
			{
				//nglDrawTwoSided( renderer, shadow_caster ? m_directional_light_shadow_shader : m_directional_light_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p );
				nglDrawTwoSided(renderer, m_directional_light_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
			}

			break;
		}

		case KCL::LightShape::OMNI:
		{
			mvp = light->m_uniform_shape_world_pom * m_active_camera->GetViewProjection();
			nglDrawBackSided(renderer, shadow_caster ? m_omni_light_shadow_shader : m_omni_light_shader, m_shapes->m_sphere_vbid, m_shapes->m_sphere_ibid, p);
			break;
		}

		case KCL::LightShape::SPOT:
		{
			mvp = light->m_uniform_shape_world_pom * m_active_camera->GetViewProjection();
			nglDrawBackSided(renderer, shadow_caster ? m_spot_light_shadow_shader : m_spot_light_shader, m_shapes->m_cone_vbid, m_shapes->m_cone_ibid, p);
			break;
		}

		default:
			INFO("Unkown light type: %d", light_shape->m_light_type);
			break;
		}
	}
}


void Scene5::RenderEmissiveLights(KCL::uint32 renderer)
{
	nglDepthState(renderer, NGL_DEPTH_EQUAL, false);
	RenderCamera(renderer, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE_EMISSIVE], m_emissive_light_shader);
}


void Scene5::RenderIBL(KCL::uint32 renderer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_IBL))
	{
		return;
	}

	GIValues giv = GetEnvironment()->GetValues().m_gi_values;

	const void *p[UNIFORM_MAX];

	// G-buffers
	BindGBuffer(p);

	// Camera
	p[UNIFORM_VIEW_POS] = m_view_pos.v;
	p[UNIFORM_VIEW_DIR] = m_view_dir.v;
	p[UNIFORM_INV_VP] = m_view_projection_inverse.v;
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;

	// IBL
	p[UNIFORM_BRDF] = &m_integrate_brdf_lut_texture;
	p[UNIFORM_ENVMAP0] = &m_prefiltered_cubemap_texture;
	p[UNIFORM_IBL_DIFFUSE_INTENSITY] = &giv.m_ibl_diffuse_intensity;
	p[UNIFORM_IBL_REFLECTION_INTENSITY] = &giv.m_ibl_reflection_intensity;

	// SSAO
	p[UNIFORM_SSAO_TEXTURE] = &m_ssao_texture;

	nglDepthState(renderer, NGL_DEPTH_GREATER, false);
	nglDrawTwoSided(renderer, m_ibl_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
}


void Scene5::DispatchHDR(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_HDR))
	{
		return;
	}

	m_compute_hdr->SetInputTexture(m_lighting_texture);
	m_compute_hdr->Render(command_buffer);
}


void Scene5::RenderBloom(KCL::uint32 command_buffer)
{
	if(RenderFlagDisabled(RenderOpts::FLAG_HDR) || RenderFlagDisabled(RenderOpts::FLAG_BLOOM))
	{
		return;
	}

	if (RenderFlagDisabled(RenderOpts::FLAG_BLOOM_MOBILE))
	{
		// High tier bloom
		m_bloom->SetColorCorrection(m_color_correction);
		m_bloom->ExecuteBrightPass(command_buffer, m_lighting_texture);

		for (KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount() - 1; layer++)
		{
			m_bloom->ExecuteDownsample(command_buffer, layer);
		}
		for (KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount(); layer++)
		{
			m_bloom->ExectureVerticalBlur(command_buffer, layer);
		}
		for (KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount(); layer++)
		{
			m_bloom->ExectureHorizontalBlur(command_buffer, layer);
		}
	}
	else
	{
		m_bloom_mobile->ExecuteLumianceBufferBarrier(command_buffer);
		// Normal tier bloom
		m_bloom_mobile->SetColorCorrection(m_color_correction);
		// Initial downsampling pass 1/1 --> 1/4
		m_bloom_mobile->ExecuteBrightPass(command_buffer, m_lighting_texture);

		// 1/4  --> 1/8
		// 1/8  --> 1/16
		// 1/16 --> 1/32
		// 1/32 --> 1/64
		for(KCL::uint32 layer = 1; layer < m_bloom_mobile->GetLayerCount(); layer++)
		{
			m_bloom_mobile->ExecuteDownsample(command_buffer, layer);
		}

		// 1/64 + 1/32 --> 1/32
		// 1/32 + 1/16 --> 1/16
		// 1/16 + 1/8  --> 1/8
		// 1/8         --> 1/4
		for(KCL::uint32 layer = 1; layer < m_bloom_mobile->GetLayerCount(); layer++)
		{
			m_bloom_mobile->ExecuteUpsample(command_buffer, layer);
		}
	}
}


void Scene5::RenderTonemapperPass(KCL::uint32 command_buffer)
{
	const bool mobile_bloom = RenderFlagEnabled(RenderOpts::FLAG_BLOOM_MOBILE);

	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_final_texture, NGL_COLOR_ATTACHMENT)
		.TextureBarrier(m_lighting_texture, NGL_SHADER_RESOURCE);

	if (mobile_bloom == false)
	{
		transitions
			.TextureBarrier(m_bloom->GetBloomTexture(0), NGL_SHADER_RESOURCE)
			.TextureBarrier(m_bloom->GetBloomTexture(1), NGL_SHADER_RESOURCE)
			.TextureBarrier(m_bloom->GetBloomTexture(2), NGL_SHADER_RESOURCE)
			.TextureBarrier(m_bloom->GetBloomTexture(3), NGL_SHADER_RESOURCE);
	}
	else
	{
		transitions.TextureBarrier(m_bloom_mobile->GetBloomTexture(m_bloom_mobile->GetLayerCount() - 1), NGL_SHADER_RESOURCE);
	}

	if (m_compute_hdr->GetExposureMode() != EXPOSURE_MANUAL)
	{
		transitions.BufferBarrier(m_compute_hdr->GetLuminanceBuffer(), NGL_SHADER_RESOURCE);
	}

	transitions.Execute(command_buffer);

	const void *p[UNIFORM_MAX];
	BindGBuffer( p );

	// Camera uniforms
	p[UNIFORM_VIEW_POS] = m_view_pos.v;
	p[UNIFORM_VIEW_DIR] = m_view_dir.v;
	p[UNIFORM_INV_VP] = m_view_projection_inverse.v;
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;
	p[UNIFORM_INV_RESOLUTION] = m_inv_resolution.v;

	p[UNIFORM_LIGHTING_TEX] = &m_lighting_texture;

	p[UNIFORM_COLOR_CORRECTION] = m_color_correction.v;
	p[UNIFORM_SHARPEN_FILTER] = m_sharpen_filter.v;

	// FOG values
	FogValues fog_values = GetEnvironment()->GetValues().m_fog_values;
	p[UNIFORM_FOG_COLOR] = fog_values.m_fog_color.v;
	p[UNIFORM_FOG_PARAMETERS1] = fog_values.m_fog_parameters1.v;
	p[UNIFORM_FOG_PARAMETERS2] = fog_values.m_fog_parameters2.v;

	p[UNIFORM_HDR_ABCD] = m_compute_hdr->GetUniformTonemapperConstants1();
	p[UNIFORM_HDR_EFW_TAU] = m_compute_hdr->GetUniformTonemapperConstants2();
	p[UNIFORM_HDR_TONEMAP_WHITE] = m_compute_hdr->GetUniformTonemapWhite();
	p[UNIFORM_HDR_EXPOSURE] = m_compute_hdr->GetUniformExposure();
	p[UNIFORM_BLOOM_PARAMETERS] = m_compute_hdr->GetUniformBloomParameters();

	if(RenderFlagDisabled(RenderOpts::FLAG_BLOOM_MOBILE))
	{
		p[UNIFORM_BLOOM_TEXTURE0] = m_bloom->GetUniformBloomTexture(0);
		p[UNIFORM_BLOOM_TEXTURE1] = m_bloom->GetUniformBloomTexture(1);
		p[UNIFORM_BLOOM_TEXTURE2] = m_bloom->GetUniformBloomTexture(2);
		p[UNIFORM_BLOOM_TEXTURE3] = m_bloom->GetUniformBloomTexture(3);
	}
	else
	{
		p[UNIFORM_BLOOM_TEXTURE0] = m_bloom_mobile->GetUniformBloomTexture(m_bloom_mobile->GetLayerCount() - 1);
	}

	KCL::uint32 tonemapper_shader = m_tonemapper_manual_exposure_shader;
	if (RenderFlagEnabled(RenderOpts::FLAG_HDR) && m_compute_hdr->GetExposureMode() != EXPOSURE_MANUAL)
	{
		tonemapper_shader = m_tonemapper_auto_exposure_shader;
	}

	nglBegin(m_tonemapper_render, command_buffer);
	nglDrawTwoSided(m_tonemapper_render, tonemapper_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(m_tonemapper_render);
}


void Scene5::DispatchMotionBlur(KCL::uint32 command_buffer)
{
	if (RenderFlagEnabled(GFXB::RenderOpts::FLAG_MOTION_BLUR) && m_motion_blur->IsEnabled())
	{
		m_motion_blur->ExecuteReduction(command_buffer);

		m_motion_blur->ExecuteNeighborMax(command_buffer);
	}
}


void Scene5::RenderMotionBlur(KCL::uint32 command_buffer)
{
	if (RenderFlagEnabled(RenderOpts::FLAG_MOTION_BLUR) && m_motion_blur->IsEnabled())
	{
		m_motion_blur->RenderMotionBlur(command_buffer, m_active_camera);
	}
}


void Scene5::RenderDOF(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_DOF))
	{
		return;
	}

	const bool m_half_res_dof = RenderFlagEnabled(RenderOpts::FLAG_DOF_HALF_RES);

	const DOFValues &dof_values = GetEnvironment()->GetValues().m_dof_values;
	KCL::uint32 dof_input_texture = RenderFlagEnabled(RenderOpts::FLAG_MOTION_BLUR) && m_motion_blur->IsEnabled() ? m_motion_blur->GetOutputTexture() : m_final_texture;

	m_dof->SetDOFValues(dof_values);
	m_dof->SetInputTexture(dof_input_texture);
	m_dof->SetKernelSize(m_dof_normalized_strength);
	m_dof->SetOutputBufferId(IsEditorMode() ? 0 : m_active_backbuffer_id);

	m_dof->Render(command_buffer, m_active_camera, m_half_res_dof ? m_depth_downsample_texture : m_gbuffer_depth_texture, m_camera_focus_distance);
}


void Scene5::RenderDebugProbes(KCL::uint32 command_buffer)
{
	if (RenderFlagEnabled(RenderOpts::FLAG_RENDER_PROBES_ATLAS) || RenderFlagEnabled(RenderOpts::FLAG_RENDER_PROBES_SH))
	{
		KCL::int32 vp[4] = { 0, 0, (KCL::int32)m_viewport_width, (KCL::int32)m_viewport_height };
		m_gi->DebugRender(command_buffer, vp, m_active_camera, m_shapes->m_sphere_vbid, m_shapes->m_sphere_ibid);
	}

	/*
	if (m_debug_renderer != nullptr)
	{
		m_debug_renderer->SetPerimeterThreshold(0.0f);
		for (size_t i = 0; i < m_gi->m_envprobes.size(); i++)
		{
				KCL::Vector3D offset;
				m_debug_renderer->SetColor(KCL::Vector3D(0.1f, 0.1f, 0.3f));
				m_debug_renderer->SetCullMode(NGL_BACK_SIDED);
				m_debug_renderer->SetDepthTestMode(NGL_depth_func::NGL_DEPTH_LESS);
				m_debug_renderer->DrawSphere(m_gi->m_envprobes[i]->m_sampling_pos, 0.25f);
		}
		//for (size_t i = 0; i < m_debug_nodes.size(); i++)
		//{
		//	for (size_t h = 0; h < m_debug_nodes[0].size(); h++)
		//	{
		//		for (size_t j = 0; j < m_debug_nodes[0][0].size(); j++)
		//		{
		//			KCL::Vector3D offset;
		//			//m_debug_nodes[i][h][j] = KCL::Vector3D(m_selected_probes.m_offset.x + i * 5.0f, m_selected_probes.m_offset.y + h * 5.0f, m_selected_probes.m_offset.z + j * 5.0f);
		//			m_debug_nodes[i][h][j] = KCL::Vector3D(offset.x + i * 5.0f, offset.y + h * 5.0f, offset.z + j * 5.0f);
		//			m_debug_renderer->SetColor(KCL::Vector3D(0.1f, 0.1f, 0.3f));
		//			m_debug_renderer->SetCullMode(NGL_BACK_SIDED);
		//			m_debug_renderer->SetDepthTestMode(NGL_depth_func::NGL_DEPTH_LESS);
		//			m_debug_renderer->DrawSphere(m_debug_nodes[i][h][j], 0.25f);
		//		}
		//	}
		//}
		if (!m_debug_nodes.empty())
		{
			for (std::vector<KCL::Vector3D>::const_iterator it = m_selected_probes.m_position.begin(); it != m_selected_probes.m_position.end(); it++)
			{
				m_debug_renderer->SetColor(KCL::Vector3D(0.3f, 0.1f, 0.1f));
				m_debug_renderer->SetCullMode(NGL_BACK_SIDED);
				m_debug_renderer->SetDepthTestMode(NGL_depth_func::NGL_DEPTH_LESS);
				m_debug_renderer->DrawSphere(KCL::Vector3D(it->x, it->y, it->z), 0.25f);
			}
		}
	}*/
}


void Scene5::RenderDebugView(KCL::uint32 command_buffer)
{
	// Render the freezed camera
	if (m_debug_freezed_camera != nullptr)
	{
		m_debug_renderer->SetColor(KCL::Vector3D(1.0f, 1.0f, 1.0f));
		m_debug_renderer->DrawCamera(m_debug_freezed_camera);
	}

	// Render selected meshes
	m_debug_renderer->RenderSelectedMeshes(command_buffer, m_active_camera);

	// Debug visualization
	m_debug_renderer->Render(command_buffer, m_active_camera);

	// Blit to onscreen
	m_debug_renderer->RenderOnscreen(command_buffer);

	// Render target visualization
	if (m_debug_view == RenderOpts::DEBUG_VIEW_DISABLED)
	{
		return;
	}

	if (m_debug_view == RenderOpts::DEBUG_VIEW_DIRECT_SHADOW && m_debug_shadow_map == nullptr)
	{
		return;
	}

	const void *p[UNIFORM_MAX];
	BindGBuffer(p);

	{
		// Scene5 does not have emissive Gbuffer
		p[UNIFORM_GBUFFER_EMISSIVE_TEX] = &m_gbuffer_color_texture;
	}

	if (m_debug_shadow_map)
	{
		p[UNIFORM_SHADOW_MAP] = m_debug_shadow_map->GetUniformShadowTexture();
	}

	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;

	if(RenderFlagDisabled( RenderOpts::FLAG_BLOOM_MOBILE ))
	{
		p[UNIFORM_BLOOM_TEXTURE0] = m_bloom->GetUniformBloomTexture( 0 );
		p[UNIFORM_BLOOM_TEXTURE1] = m_bloom->GetUniformBloomTexture( 1 );
		p[UNIFORM_BLOOM_TEXTURE2] = m_bloom->GetUniformBloomTexture( 2 );
		p[UNIFORM_BLOOM_TEXTURE3] = m_bloom->GetUniformBloomTexture( 3 );
		p[UNIFORM_BRIGHT_TEXTURE0] = m_bloom->GetUniformBrightTexture( 0 );
		p[UNIFORM_BRIGHT_TEXTURE1] = m_bloom->GetUniformBrightTexture( 1 );
		p[UNIFORM_BRIGHT_TEXTURE2] = m_bloom->GetUniformBrightTexture( 2 );
		p[UNIFORM_BRIGHT_TEXTURE3] = m_bloom->GetUniformBrightTexture( 3 );
	}
	else
	{
		p[UNIFORM_BLOOM_TEXTURE0] = m_bloom_mobile->GetUniformBloomTexture( m_bloom_mobile->GetLayerCount() - 1 );
	}

	p[UNIFORM_DEPTH_DOWNSAMPLE_TEX] = &m_depth_downsample_texture;
	p[UNIFORM_SSAO_TEXTURE] = RenderFlagEnabled(RenderOpts::FLAG_SSAO_BLUR) ? m_ssao_blur->GetUniformOutputTexture() : &m_ssao_texture;
	p[UNIFORM_DOF_INPUT_TEXTURE] = m_dof->IsHalfResMode() ? m_dof->GetUniformDownsampledTexture() : m_dof->GetUniformInputTexture();
	p[UNIFORM_DOF_BLUR_TEXTURE] = m_dof->GetUniformOutputTexture();
	p[UNIFORM_NEIGHBOR_MAX_TEXTURE] = &m_motion_blur->GetNeighborMaxTexture();
	p[UNIFORM_MOTION_BLUR_TEXTURE] = &m_motion_blur->GetOutputTexture();
	p[UNIFORM_HALF_RES_TRANSPARENT_TEXTURE] = &m_transparent_texture;

	nglBegin(m_debug_view_render, command_buffer);
	nglDrawTwoSided(m_debug_view_render, m_debug_view_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(m_debug_view_render);
}


void Scene5::RenderCamera(KCL::uint32 job, KCL::Camera2 *camera, std::vector<KCL::Mesh*> &meshes, KCL::uint32 custom_shader)
{
	const size_t mesh_count = meshes.size();
	if (mesh_count == 0)
	{
		return;
	}

	// Camera
	KCL::Matrix4x4 vp = camera->GetViewProjection();
	KCL::Vector4D view_pos = KCL::Vector4D(camera->GetEye());
	KCL::Vector4D view_dir = KCL::Vector4D(KCL::Vector3D(-camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]).normalize(), 0.0f);

	// Mesh
	KCL::Matrix4x4 model;
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 prev_mvp;
	KCL::Matrix4x4 mv;

	// Material
	KCL::Vector2D texture_size;

	const void *p[UNIFORM_MAX];

	// Camera uniforms
	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_PREV_VP] = m_prev_vp.v;
	p[UNIFORM_VIEW_POS] = &view_pos;
	p[UNIFORM_VIEW_DIR] = &view_dir;
	p[UNIFORM_DEPTH_PARAMETERS] = &camera->m_depth_linearize_factors;

	// Motion blur
	p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_motion_blur->GetVelocityParams().v;

	// Mesh uniforms
	p[UNIFORM_MODEL] = model.v;
	p[UNIFORM_MV] = mv.v;
	p[UNIFORM_MVP] = mvp.v;
	p[UNIFORM_PREV_MVP] = prev_mvp.v;

	// Debug
	const bool colorize_shadow_casters = RenderFlagEnabled(RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS);
	KCL::uint32 lod_index = 0;
	p[UNIFORM_MIP_TEX] = &m_mip_texture;
	p[UNIFORM_MIN_IDEAL_MAX_TEXTURE_DENSITY] = &m_texture_density_square;
	p[UNIFORM_DEBUG_WIREFRAME_MODE] = &m_wireframe_uniform;

	KCL::uint32 shader_code = custom_shader;
	for (KCL::uint32 i = 0; i < mesh_count; i++)
	{
		Scene5Mesh *mesh = (Scene5Mesh*)meshes[i];

		Mesh3 *gfxb_mesh3 = (Mesh3*)mesh->GetMesh3(camera->GetEye(), m_lod1_threshold2, m_lod2_threshold2, &lod_index);

		Material *gfxb_material = (Material*)mesh->m_material;

		/*
		if (mesh->m_dither_value > 0.0f && mesh->m_dither_value < 1.0f)
		{
			// TODO: dither shader
		}
		*/

		if (gfxb_material->m_material_type == KCL::Material::SKY)
		{
			mvp = mesh->m_world_pom * camera->GetViewProjectionOrigo();
			prev_mvp = mvp;
		}
		else
		{
			mv = mesh->m_world_pom * camera->GetView();
			mvp = mesh->m_world_pom * vp;
			prev_mvp = mesh->m_prev_world_pom * m_prev_vp;
		}

		model = mesh->m_world_pom;
		p[UNIFORM_COLOR_TEX] = &gfxb_material->GetTexture(KCL::Material::COLOR)->m_id;
		p[UNIFORM_NORMAL_TEX] = &gfxb_material->GetTexture(KCL::Material::BUMP)->m_id;
		p[UNIFORM_SPECULAR_TEX] = &gfxb_material->GetTexture(KCL::Material::MASK)->m_id;
		p[UNIFORM_EMISSIVE_TEX] = &gfxb_material->GetTexture(KCL::Material::EMISSION)->m_id;
		p[UNIFORM_AUX_TEX0] = &gfxb_material->GetTexture(KCL::Material::AUX0)->m_id;
		p[UNIFORM_AUX_TEX1] = &gfxb_material->GetTexture(KCL::Material::AUX1)->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &gfxb_material->m_alpha_test_threshold;
		p[UNIFORM_BONES] = gfxb_mesh3->m_node_matrices.empty() ? nullptr : &gfxb_mesh3->m_node_matrices[0];
		p[UNIFORM_PREV_BONES] = gfxb_mesh3->m_prev_node_matrices.empty() ? nullptr : &gfxb_mesh3->m_prev_node_matrices[0];
		p[UNIFORM_EMISSIVE_INTENSITY] = &mesh->m_emissive_intensity;
		p[UNIFORM_FIRE_TIME] = &mesh->m_fire_time;
		p[UNIFORM_DITHER_VALUE] = &mesh->m_dither_value;

		texture_size.set(
			(float)gfxb_material->GetTexture(KCL::Material::COLOR)->getWidth(),
			(float)gfxb_material->GetTexture(KCL::Material::COLOR)->getHeight());
		p[UNIFORM_TEXSIZE_SIZE] = texture_size.v;

		if (custom_shader == (KCL::uint32)-1)
		{
			Material::ShaderVariant shader_variant = mesh->m_mesh->m_nodes.size() ? Material::SHADER_SKELETAL : Material::SHADER_NORMAL;
			shader_code = gfxb_material->GetShaderCode(shader_variant);
		}

		if (colorize_shadow_casters)
		{
			p[UNIFORM_DEBUG_FRUSTUM_IDS] = mesh->m_debug_frustum_ids.data();
		}
		p[UNIFORM_DEBUG_MESH_LOD_INDEX] = &lod_index;

		nglDraw(job, NGL_TRIANGLES, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, gfxb_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p);
	}
}


void Scene5::RenderLightshafts(KCL::uint32 renderer)
{
	if (m_main_frustum_cull->m_visible_light_shafts.empty())
	{
		return;
	}

	KCL::Matrix4x4 vp = m_active_camera->GetViewProjection();
	KCL::Vector4D lightshaft_color;
	KCL::Vector4D lightshaft_params;
	KCL::Vector4D lightshaft_frustum_planes[6];

	// Camera
	const void *p[UNIFORM_MAX];
	p[UNIFORM_DEPTH_PARAMETERS] = m_active_camera->m_depth_linearize_factors.v;
	p[UNIFORM_VIEW_POS] = m_view_pos.v;
	p[UNIFORM_VIEW_DIR] = m_view_dir.v;
	p[UNIFORM_VP] = vp.v;

	// Lightshaft
	p[UNIFORM_LIGHT_COLOR] = lightshaft_color.v;
	p[UNIFORM_LIGHT_FUSTUM_PLANES] = lightshaft_frustum_planes;
	p[UNIFORM_LIGHTSHAFT_PARAMETERS] = lightshaft_params.v;

	// Downsampled depth
	p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_depth_downsample_texture;

	// Render lightshafts to half sized render target
	for (size_t i = 0; i < m_main_frustum_cull->m_visible_light_shafts.size(); i++)
	{
		Light *light = (Light*)m_main_frustum_cull->m_visible_light_shafts[i];
		Lightshaft::Values &values = light->GetLightshaft()->GetValues();

		if (values.m_intensity <= 0.0f || values.m_sample_count == 0)
		{
			// Disabled lightshaft
			continue;
		}


		KCL::LightShape *shape = light->m_light_shape;
		if (!shape->m_is_shadow_caster || shape->m_light_type != KCL::LightShape::DIRECTIONAL || !shape->m_box_light)
		{
			// Currently only support directional box lights with shadow as light shafts
			continue;
		}

		// Shadow
		p[UNIFORM_SHADOW_MATRIX] = light->GetShadowMap()->GetUniformShadowMatrix();
		p[UNIFORM_SHADOW_MAP] = light->GetShadowMap()->GetUniformShadowTexture();

		// sample count 32 hardcoded in the shader
		assert(values.m_sample_count == 32);

		// Lightshaft
		lightshaft_params.x = 0.0f; // Not used
		lightshaft_params.y = values.m_intensity;
		lightshaft_params.z = float(values.m_sample_count);
		lightshaft_params.w = 1.0f / float(values.m_sample_count);

		lightshaft_color.set(light->m_animated_color, light->m_uniform_intensity);

		for (size_t i = 0; i < 6; i++)
		{
			lightshaft_frustum_planes[i] = light->GetShadowMap()->GetCamera().GetCullPlane(i);
		}

		m_lightshaft_util->RenderBoxLightShaft(renderer, m_active_camera, m_shapes, light, p);
	}
}


void Scene5::RenderParticles(KCL::uint32 renderer)
{
	if (RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS))
	{
		m_particlesystem_manager->RenderAll(renderer, m_depth_downsample_texture, m_active_camera);
	}
}


void Scene5::RenderTransparentUpsamplePass(KCL::uint32 renderer)
{
	KCL::uint32 input_texture = m_transparent_texture;
	if (RenderFlagEnabled(RenderOpts::FLAG_LIGHTSHAFT) && RenderFlagEnabled(RenderOpts::FLAG_BLUR_TRANSPARENTS))
	{
		input_texture = m_transparent_blur->GetOutputTexture();
	}

	// Bilateral upsampling
	const void *p[UNIFORM_MAX];
	p[UNIFORM_COLOR_TEX] = &input_texture;
	p[UNIFORM_INPUT_TEXTURE] = &m_depth_downsample_texture;

	float downscale = (float)TRANSPARENT_DOWNSCALE;

	KCL::Vector4D res_and_inv_res = KCL::Vector4D(m_viewport_width / downscale, m_viewport_height / downscale, downscale / m_viewport_width, downscale / m_viewport_height);
	p[UNIFORM_INV_RESOLUTION] = res_and_inv_res.v;

	nglDraw(renderer, NGL_TRIANGLES, m_upsample_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p);
}


void Scene5::Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h)
{
	SceneBase::Resize(x, y, w, h);

	NormalizeEffectParameters();

	// Resize textures
	KCL::uint32 size[3] = { w, h, 0 };
	uint32_t textures[] =
	{
		m_gbuffer_color_texture, m_gbuffer_normal_texture, m_gbuffer_specular_texture, m_gbuffer_velocity_texture, m_gbuffer_depth_texture,
		m_lighting_texture, m_indirect_light_weight_texture, m_final_texture
	};
	nglResizeTextures( COUNT_OF(textures), textures, size);

	KCL::uint32 half_size[3] = { w / TRANSPARENT_DOWNSCALE, h / TRANSPARENT_DOWNSCALE, 0 };
	nglResizeTextures(1, &m_depth_downsample_texture, half_size);

	KCL::uint32 ssao_size[3] = { w / SSAO_DOWNSCALE, h / SSAO_DOWNSCALE, 0 };
	nglResizeTextures(1, &m_ssao_texture, ssao_size);

	KCL::uint32 transparent_size[3] = { w / TRANSPARENT_DOWNSCALE, h / TRANSPARENT_DOWNSCALE, 0 };
	nglResizeTextures(1, &m_transparent_texture, transparent_size);

	// Resize viewports
	for (KCL::uint32 i = 0; i < SSAO_DEPTH_MIP_LEVELS; i++)
	{
		KCL::uint32 width = w / (1 << (i + 1));
		KCL::uint32 height = h / (1 << (i + 1));
		width = width ? width : 1;
		height = height ? height : 1;

		KCL::int32 viewport[4] =
		{
			0, 0, KCL::int32(width), KCL::int32(height)
		};
		nglViewportScissor(m_depth_downsample_renderers[i], viewport, viewport);
	}

	KCL::int32 ssao_vp[4] = { 0, 0, KCL::int32(w / SSAO_DOWNSCALE), KCL::int32(h / SSAO_DOWNSCALE) };
	nglViewportScissor(m_ssao_render, ssao_vp, ssao_vp);

	KCL::int32 transparent_vp[4] = { 0, 0, KCL::int32(w / TRANSPARENT_DOWNSCALE), KCL::int32(h / TRANSPARENT_DOWNSCALE) };
	nglViewportScissor(m_transparent_render, transparent_vp, transparent_vp);

	KCL::int32 vp[4] = { 0, 0, (KCL::int32)w, (KCL::int32)h };
	nglViewportScissor(m_tonemapper_render, vp, vp);

	if (IsEditorMode())
	{
		// Always Onscreen
		KCL::int32 onscreen_vp[4] = { (KCL::int32)x, (KCL::int32)y, (KCL::int32)w, (KCL::int32)h };
		nglViewportScissor(m_debug_view_render, onscreen_vp, onscreen_vp);
	}

	if (m_compute_hdr)
	{
		if( RenderFlagEnabled( RenderOpts::FLAG_BLOOM_MOBILE ) )
		{
			m_compute_hdr->SetDownsample( 4 );
		}
		else
		{
			m_compute_hdr->SetDownsample( 2 );
		}
		m_compute_hdr->UpdateViewport(w, h);
	}


	if(m_bloom)
	{
		m_bloom->Resize(m_bloom_normalized_strength, w / BLOOM_DOWNSCALE, h / BLOOM_DOWNSCALE);
	}

	if(m_bloom_mobile)
	{
		m_bloom_mobile->Resize(m_bloom_normalized_strength, w / (BLOOM_DOWNSCALE * 2), h / (BLOOM_DOWNSCALE * 2));
	}

	if (m_dof)
	{
		// Onscreen (Offscreen in editor mode)
		m_dof->Resize(m_dof_normalized_strength, 0, 0, w, h);
	}

	if (m_ssao_blur)
	{
		m_ssao_blur->Resize(m_ssao_blur_normalized_strength, w / SSAO_DOWNSCALE, h / SSAO_DOWNSCALE);
	}

	if (m_motion_blur)
	{
		m_motion_blur->Resize(w, h);
	}

	if (m_transparent_blur)
	{
		m_transparent_blur->Resize(m_transparent_normalized_blur_strength, w / TRANSPARENT_DOWNSCALE, h / TRANSPARENT_DOWNSCALE);
	}

	if (m_debug_renderer)
	{
		m_debug_renderer->UpdateViewport(x, y, w, h);
		nglResizeTextures(1, &m_debug_texture, size);
	}
}


KCL::KCL_Status Scene5::ReloadShaders()
{
	InitShaders();

	for(KCL::uint32 i = 0; i < SSAO_DEPTH_MIP_LEVELS; i++)
	{
		nglDeletePipelines(m_depth_downsample_renderers[i]);
	}

	nglDeletePipelines(m_transparent_render);
	nglDeletePipelines(m_ssao_render);
	nglDeletePipelines(m_tonemapper_render);
	if (IsEditorMode())
	{
		nglDeletePipelines(m_debug_view_render);
	}

	if (m_gi)
	{
		m_gi->ReloadShaders();
	}

	if (m_compute_hdr)
	{
		m_compute_hdr->DeleteRenderers();
	}

	if (m_bloom)
	{
		m_bloom->DeletePipelines();
	}

	if (m_bloom_mobile)
	{
		m_bloom_mobile->DeletePipelines();
	}

	if (m_dof)
	{
		m_dof->DeletePipelines();
	}

	if (m_ssao_blur)
	{
		m_ssao_blur->DeletePipelines();
	}

	if (m_motion_blur)
	{
		m_motion_blur->DeletePipelines();
	}

	if (m_debug_renderer)
	{
		m_debug_renderer->DeleteRenderers();
	}

	if (m_transparent_blur)
	{
		m_transparent_blur->DeletePipelines();
	}

	for (size_t i = 0; i < m_shadow_maps.size(); i++)
	{
		m_shadow_maps[i]->DeletePipelines();
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


Scheduler *Scene5::CreateScheduler()
{
	switch (nglGetApi())
	{
	case NGL_METAL_MACOS:
	case NGL_METAL_IOS:
	case NGL_DIRECT3D_12:
		//INFO("Create Multi Threaded Sceduler...");
		//return new MultiThreadedOnDemandScheduler(4);

	default:
		//INFO("Create Single Threaded Scheduler...");
		return new SingleThreadedScheduler();
	}
}


KCL::TextureFactory &Scene5::TextureFactory()
{
	return *m_texture_factory;
}


GFXB::Mesh3Factory &Scene5::Mesh3Factory()
{
	return *m_mesh3_factory;
}


Shapes *Scene5::GetShapes() const
{
	return m_shapes;
}


ComputeHDR *Scene5::GetComputeHDR() const
{
	return m_compute_hdr;
}


FrustumCull *Scene5::GetMainCameraFrustumCull() const
{
	return m_main_frustum_cull;
}


MeshFilter *Scene5::GetShadowMeshFilter() const
{
	return m_shadow_mesh_filter;
}


KCL::uint32 Scene5::GetGBufferColor() const
{
	return m_gbuffer_color_texture;
}


KCL::uint32 Scene5::GetGBufferSpecular() const
{
	return m_gbuffer_specular_texture;
}


KCL::uint32 Scene5::GetGBufferNormal() const
{
	return m_gbuffer_normal_texture;
}


KCL::uint32 Scene5::GetGBufferEmissive() const
{
	return m_gbuffer_emissive_texture;
}


KCL::uint32 Scene5::GetGBufferDepth() const
{
	return m_gbuffer_depth_texture;
}


DebugRenderer *Scene5::GetDebugRenderer() const
{
	return m_debug_renderer;
}


std::vector<std::vector<KCL::Mesh*>> &Scene5::GetPartitionMeshes()
{
	return m_partition_meshes;
}


std::vector<ShadowMap*> Scene5::GetShadowMaps()
{
	std::vector<ShadowMap*> shadow_maps;

	for (size_t i = 0; i < m_actors.size(); i++)
	{
		for (size_t j = 0; j < m_actors[i]->m_lights.size(); j++)
		{
			Light *light = (Light*)m_actors[i]->m_lights[j];
			if (light->m_light_shape->m_is_shadow_caster && light->GetShadowMap() != nullptr)
			{
				shadow_maps.push_back(light->GetShadowMap());
			}
		}
	}

	return shadow_maps;
}


void Scene5::SetRenderFlag(RenderOpts::Flag flag, bool value)
{
	KCL::uint32 new_value = m_render_flags;
	if (value)
	{
		// Set bit
		new_value |= (1 << KCL::uint32(flag));
	}
	else
	{
		// Clear bit
		new_value &= (~(1 << KCL::uint32(flag)));
	}

	if (new_value != m_render_flags)
	{
		m_render_flags = new_value;

		if (IsInitialized())
		{
			if (m_gi != nullptr && m_gi->m_use_texture_sh_atlas != RenderFlagEnabled(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS))
			{
				// TODO: to be able to change FLAG_GI_USE_TEXTURE_SH_ATLAS while running GI has to be reinitialized
				INFO("Cannot change FLAG_GI_USE_TEXTURE_SH_ATLAS while running");
				assert(false);

				// prevent changing FLAG_GI_USE_TEXTURE_SH_ATLAS. If changing the flag is safely handled, remove this line
				SetRenderFlag(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS, m_gi->m_use_texture_sh_atlas);
			}

			ReloadShaders();

			InitDOF();

			if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING) || RenderFlagEnabled(RenderOpts::FLAG_IBL))
			{
				nglBeginCommandBuffer(m_command_buffer_default);

				IntegrateBRDFTexture(m_command_buffer_default);

				if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING) && m_gi == nullptr)
				{
					m_gi = new GI2();
					CreateGIStuff(m_command_buffer_default);
				}

				nglEndCommandBuffer(m_command_buffer_default);
				nglSubmitCommandBuffer(m_command_buffer_default);
			}
		}
	}
}


void Scene5::SetRenderFlag(KCL::uint32 flags, RenderOpts::Flag flag, bool value)
{
	if (value)
	{
		// Set bit
		flags |= (1 << KCL::uint32(flag));
	}
	else
	{
		// Clear bit
		flags &= (~(1 << KCL::uint32(flag)));
	}
}


bool Scene5::RenderFlagEnabled(KCL::uint32 flags, RenderOpts::Flag flag)
{
	return (flags & (1 << KCL::uint32(flag))) != 0;
}


bool Scene5::RenderFlagEnabled(RenderOpts::Flag flag) const
{
	return (m_render_flags & (1 << KCL::uint32(flag))) != 0;
}


bool Scene5::RenderFlagDisabled(RenderOpts::Flag flag) const
{
	return (m_render_flags & (1 << KCL::uint32(flag))) == 0;
}


void Scene5::DebugFreezCamera(bool value)
{
	if (value)
	{
		if (m_debug_freezed_camera == nullptr)
		{
			m_debug_freezed_camera = new KCL::Camera2();
		}
		*m_debug_freezed_camera = *m_active_camera;
	}
	else
	{
		delete m_debug_freezed_camera;
		m_debug_freezed_camera = nullptr;
	}
}


bool Scene5::DebugHasFreezCamera()
{
	return m_debug_freezed_camera != nullptr;
}


void Scene5::DebugShowRoomPlanes()
{
	m_debug_show_room_planes++;
	if (m_debug_show_room_planes > m_partition_meshes.size() - 1)
	{
		m_debug_show_room_planes = 0;
	}
	INFO("Showing parition meshes for room: %s", m_rooms[m_debug_show_room_planes]->m_name.c_str());
}


void Scene5::SetDebugView(RenderOpts::DebugView debug_view)
{
	if (debug_view != m_debug_view)
	{
		m_debug_view = debug_view;
		if (m_debug_view != RenderOpts::DEBUG_VIEW_DISABLED)
		{
			ReloadShaders();
		}
	}
}


void Scene5::SetDebugShadowMap(ShadowMap *shadow_map)
{
	m_debug_shadow_map = shadow_map;
}


RenderOpts::DebugView Scene5::GetDebugView() const
{
	return m_debug_view;
}


void Scene5::NormalizeEffectParameters()
{
	// depth of field blur strength for full HD
	m_dof_strength = GetEnvironment()->GetValues().m_dof_values.m_strength;
	m_dof_normalized_strength = (m_dof_strength * m_viewport_height) / 1080;

	m_bloom_normalized_strength = (m_bloom_strength * m_viewport_height) / 1080;
	m_transparent_normalized_blur_strength = (m_transparent_blur_strength * m_viewport_height) / 1080;
	m_ssao_blur_normalized_strength = (m_ssao_blur_strength * m_viewport_height) / 1080;
}


void Scene5::GenRenderTarget(KCL::uint32 &texture, NGL_texture_descriptor &descriptor, std::vector<std::vector<uint8_t> > *datas)
{
	nglGenTexture(texture, descriptor, datas);

	Transitions::Get().Register(texture, descriptor);
}


void Scene5::BeginStatistics()
{
	if (m_ngl_statistics.IsEnabled())
	{
		if (IsEditorMode() == false)
		{
			nglFinish();
		}

		nglBeginStatistic(m_ngl_statistics);
	}
}


void Scene5::EndStatistics()
{
	if (m_ngl_statistics.IsEnabled())
	{
		nglEndStatistic();
		//Scene5Tools::DumpPipelineStatistics(m_ngl_statistics);
	}
}
