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
#include "gfxb_lightshaft_util.h"
#include "gfxb_font_renderer.h"
//#include "gfxb_ovr_gen.h"
#include "gfxb_barrier.h"

#include "gi/gi.h"

#include "common/gfxb_compute_motion_blur.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_debug_renderer.h"
#include "common/gfxb_compute_motion_blur.h"
#include "common/gfxb_dof.h"
#include "common/gfxb_bloom.h"
#include "common/gfxb_bloom_mobile.h"
#include "common/gfxb_shadow_map.h"
#include "common/gfxb_cubemap.h"
#include "common/gfxb_fragment_blur.h"
#include "common/gfxb_light.h"
#include "common/gfxb_environment.h"
#include "common/gfxb_shot_handler.h"

#include <kcl_envprobe.h>


using namespace GFXB;

KCL::KCL_Status Scene5::Init()
{
	KCL::KCL_Status status = SceneBase::Init();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	// disable shader warmup
#if 0
	m_warmup_helper->At(ComputeMotionBlur::MB_NEIGHBOR_MAX_WGS_NAME).x = 8;
	m_warmup_helper->At(ComputeMotionBlur::MB_TILE_MAX_WGS_NAME).x = 16;

	m_warmup_helper->At(GI2::GI_COMPUTE_SH_WGS_NAME).x = 128;
#endif

	//ovr_gen::init(this);

	// Set if SH Atlas is stored in a texture or storage buffer
	{
		bool gi_use_texture_sh_atlas = m_gi_use_texture_sh_atlas;

		SetRenderFlag(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS, gi_use_texture_sh_atlas);
		INFO("FLAG_GI_USE_TEXTURE_SH_ATLAS = %s", gi_use_texture_sh_atlas ? "true" : "false");
	}

	{
		m_partition_meshes.resize(m_rooms.size());
		for (size_t i = 1; i < m_rooms.size(); i++)
		{
			for (size_t j = 0; j < m_rooms[i]->m_meshes.size(); j++)
			{
				m_partition_meshes[i].push_back(m_rooms[i]->m_meshes[j]);
			}
			m_rooms[i]->m_meshes.clear();
		}
	}

	RoomSetup::SetupRooms(this);

	/////////////////////////
	INFO("Fix up the content...");

	// Fix up the content
	for (size_t i = 0; i < m_materials.size(); i++)
	{
		KCL::Material *m = m_materials[i];
		m->m_is_shadow_caster = true;

		// Look for emissive materials: TODO handle has emissive flag in material
		KCL::Texture *emissive_texture = m->m_textures[KCL::Material::EMISSION];
		if (emissive_texture)
		{
			bool is_valid = emissive_texture->getName().find("default_emissive") == std::string::npos;
			is_valid &= emissive_texture->getName().find("no_texture") == std::string::npos;
			if (is_valid)
			{
				//	INFO("Emissive material: %s (%s)", m->m_name.c_str(), emissive_texture->getName().c_str());
				m->m_has_emissive_channel = true;
			}
		}
	}


	for (size_t i = 0; i < m_sky_mesh.size(); i++)
	{
		KCL::Material *m = m_sky_mesh[i]->m_material;
		if (m->m_shader_names[0] != "sky")
		{
			// Ensure that sky materials use sky shader
			m->m_shader_names[0] = "sky";
			m->SaveParameters();
		}
	}

	for (size_t i = 0; i < m_rooms.size(); i++)
	{
		for (size_t j = 0; j < m_rooms[i]->m_meshes.size(); j++)
		{
			if (m_rooms[i]->m_meshes[j]->m_material->m_has_emissive_channel)
			{
				m_emissive_meshes.push_back(m_rooms[i]->m_meshes[j]);
			}
		}
	}

	for (size_t i = 0; i < m_actors.size(); i++)
	{
		m_actors[i]->m_userId = (KCL::uint32)i;

		for (size_t j = 0; j < m_actors[i]->m_meshes.size(); j++)
		{
			if (m_actors[i]->m_meshes[j]->m_material->m_has_emissive_channel)
			{
				m_emissive_meshes.push_back(m_actors[i]->m_meshes[j]);
			}
		}

		if (m_actors[i]->m_name == "HERO_xActor_tanya")
		{
			m_actor_tanya = m_actors[i];
		}
		else if (m_actors[i]->m_name == "EVIL_xActor_golem")
		{
			m_actor_golem = m_actors[i];
		}
	}

	if (m_actor_tanya == nullptr)
	{
		INFO("Can not find Tanya!");
	}
	if (m_actor_golem == nullptr)
	{
		INFO("Can not find Golem!");
	}

	/////////////////////////

	m_shapes = new Shapes();
	m_shapes->Init();

	/*
	ovr_gen::init_csm(this, &m_csm_mesh_filter, &m_cascaded_shadow);
	*/

	InitIBL();

	INFO("Init shaders...");
	InitShaders();

	// Load engine materials
	m_fire_material = CreateMaterial("engine_fire_mat");
	m_glow_material = CreateMaterial("engine_glow_mat");

	// Engine meshes
	m_billboard_mesh = (GFXB::Mesh3*) Mesh3Factory().Create("billboard_shape");
	m_billboard_mesh->ConvertToBillboard(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f);
	m_meshes.push_back(m_billboard_mesh);

	//	Add fire and glow meshes
	for (size_t i = 0; i<m_actors.size(); ++i)
	{
		KCL::Actor* a = m_actors[i];
		for (size_t j = 0; j< a->m_lights.size(); ++j)
		{
			KCL::Light* light = a->m_lights[j];

			if (light->m_light_shape->m_has_fire)
			{
				GFXB::Scene5Mesh* fm = dynamic_cast<GFXB::Scene5Mesh*>(KCL::SceneHandler::GetMeshFactory().Create(light->m_name + "_fire_mesh", light, a));
				fm->m_flags = 0;
				fm->m_fire_time_offset = (KCL::uint32)j * 157;

				// move the fire up
				fm->m_local_pom.v[13] = 0.4f;

				fm->SetMaterials(m_fire_material, m_fire_material);
				fm->m_mesh = m_billboard_mesh;
				fm->m_mesh_variants[0] = m_billboard_mesh;

				fm->CalculateStaticAABB();
				fm->CreateFlickeringAnimation(22600, 5, 0.0f);
			}

			if (light->m_light_shape->m_has_glow)
			{
				GFXB::Scene5Mesh* gm = dynamic_cast<GFXB::Scene5Mesh*>(KCL::SceneHandler::GetMeshFactory().Create(light->m_name + "_glow_mesh", light, a));
				gm->m_flags = 0;
				gm->SetMaterials(m_glow_material, m_glow_material);
				gm->m_mesh = m_billboard_mesh;
				gm->m_mesh_variants[0] = m_billboard_mesh;

				gm->CalculateStaticAABB();
			}
		}
	}

	if (IsEditorMode() || RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS))
	{
		INFO("Init Particle Systems...");

		m_particlesystem_manager->Init(this);

		for (size_t i = 0; i < m_actors.size(); ++i)
		{
			KCL::Actor* actor = m_actors[i];
			for (size_t k = 0; k < actor->m_emitters.size(); ++k)
			{
				if (actor->m_emitters[k]->m_type == KCL::EMITTER5)
				{
					GFXB::ParticleEmitter* pse = dynamic_cast<GFXB::ParticleEmitter*>(actor->m_emitters[k]);

					pse->Init();
				}
			}
		}

		m_particlesystem_manager->Allocate();
		INFO("Particle Emitter Count: %d", m_particlesystem_manager->GetEmitterCount());
	}

	INFO("Create render targets and renderers...");
	int32_t scene_viewport[4] =
	{
		0, 0, (int32_t)m_viewport_width, (int32_t)m_viewport_height
	};
	int32_t ssao_viewport[4] =
	{
		0, 0, (int32_t)m_viewport_width / (int32_t)SSAO_DOWNSCALE, (int32_t)m_viewport_height / (int32_t)SSAO_DOWNSCALE
	};

	// G-Buffer
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_is_renderable = true;
		texture_layout.m_input_attachment = true;
		texture_layout.SetAllClearValue(0.0f);

		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;

		texture_layout.m_name = "gbuffer_color";
		texture_layout.m_memoryless = nglGetInteger(NGL_SUBPASS_ENABLED) != 0;
		GenRenderTarget(m_gbuffer_color_texture, texture_layout, nullptr);

		texture_layout.m_name = "gbuffer_specular";
		GenRenderTarget(m_gbuffer_specular_texture, texture_layout, nullptr);

		texture_layout.m_name = "gbuffer_normal";
		texture_layout.m_memoryless = false;
		GenRenderTarget(m_gbuffer_normal_texture, texture_layout, nullptr);

		texture_layout.m_name = "indirect_light_weight";
		texture_layout.m_format = NGL_R16_FLOAT;
		texture_layout.m_memoryless = nglGetInteger(NGL_SUBPASS_ENABLED) != 0;
		GenRenderTarget(m_indirect_light_weight_texture, texture_layout, nullptr);

		texture_layout.m_name = "gbuffer_velocity";
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_format = NGL_R16_G16_FLOAT;
		texture_layout.m_input_attachment = false;
		texture_layout.m_memoryless = false;
		texture_layout.SetAllClearValue(0.0f);
		GenRenderTarget(m_gbuffer_velocity_texture, texture_layout, nullptr);

		texture_layout.m_name = "gbuffer_depth";
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_format = NGL_D24_UNORM;
		texture_layout.SetAllClearValue(1.0f);
		texture_layout.m_input_attachment = true;
		GenRenderTarget(m_gbuffer_depth_texture, texture_layout, nullptr);
	}

	// Lighting texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "lighting";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_format = NGL_R11_B11_B10_FLOAT;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);

		GenRenderTarget(m_lighting_texture, texture_layout, nullptr);
	}

	bool half_res_transparents = IsEditorMode();
	half_res_transparents |= RenderFlagEnabled(RenderOpts::FLAG_LIGHTSHAFT);
	half_res_transparents |= RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS);

	// Half depth texture
	if (half_res_transparents || RenderFlagEnabled(RenderOpts::FLAG_SSAO) || RenderFlagEnabled(RenderOpts::FLAG_DOF_HALF_RES))
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "depth_half_texture";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST_MIPMAPPED;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_size[0] = m_viewport_width / 2;
		texture_layout.m_size[1] = m_viewport_height / 2;
		texture_layout.m_num_levels = SSAO_DEPTH_MIP_LEVELS;
		texture_layout.m_format = NGL_R16_FLOAT;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(1.0f);

		GenRenderTarget(m_depth_downsample_texture, texture_layout, nullptr);
	}

	// Half-res transparent texture
	if (half_res_transparents)
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "half_res_transp";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width / TRANSPARENT_DOWNSCALE;
		texture_layout.m_size[1] = m_viewport_height / TRANSPARENT_DOWNSCALE;
		texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);
		texture_layout.m_clear_value[3] = 1.0f; // Clear alpha to white

		GenRenderTarget(m_transparent_texture, texture_layout, nullptr);
	}

	// SSAO texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "ssao";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = (uint32_t)ssao_viewport[2];
		texture_layout.m_size[1] = (uint32_t)ssao_viewport[3];
		texture_layout.m_format = NGL_R8_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);

		GenRenderTarget(m_ssao_texture, texture_layout, nullptr);
	}

	// Debug textures
	if (IsEditorMode())
	{
		// Mipmap usage texture
		m_mip_texture = Scene5Tools::CreateDebugMipmapTexture();

		// Debug view texture
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "debug renderer target";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);

		GenRenderTarget(m_debug_texture, texture_layout, nullptr);

	}

	// Final texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "final";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue(0.0f);

		GenRenderTarget(m_final_texture, texture_layout, nullptr);
	}

	// Depth downsample job
	if (half_res_transparents || RenderFlagEnabled(RenderOpts::FLAG_SSAO) || RenderFlagEnabled(RenderOpts::FLAG_DOF_HALF_RES))
	{
		m_depth_downsample_renderers.resize(SSAO_DEPTH_MIP_LEVELS, 0);
		for (KCL::uint32 i = 0; i < SSAO_DEPTH_MIP_LEVELS; i++)
		{
			NGL_job_descriptor rrd;
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = m_depth_downsample_texture;
				ad.m_attachment.m_level = i;
				ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				std::stringstream sstream;
				sstream << "depth_downsample::" << i;

				NGL_subpass sp;
				sp.m_name = sstream.str().c_str();
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
				rrd.m_subpasses.push_back(sp);
			}

			rrd.m_load_shader_callback = LoadShader;
			m_depth_downsample_renderers[i] = nglGenJob(rrd);

			KCL::uint32 width = m_viewport_width / (1 << (i + 1));
			KCL::uint32 height = m_viewport_height / (1 << (i + 1));
			width = KCL::Max(width, 1u);
			height = KCL::Max(height, 1u);

			KCL::int32 viewport[4] =
			{
				0, 0, KCL::int32(width), KCL::int32(height)
			};
			nglViewportScissor(m_depth_downsample_renderers[i], viewport, viewport);
			nglDepthState(m_depth_downsample_renderers[i], NGL_DEPTH_DISABLED, false);
		}
	}

	// Half-res transparent render
	if (half_res_transparents)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_transparent_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "transp::render";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_transparent_render = nglGenJob(rrd);

		int viewport[4] =
		{
			scene_viewport[0], scene_viewport[1], int(scene_viewport[2] / TRANSPARENT_DOWNSCALE), int(scene_viewport[3] / TRANSPARENT_DOWNSCALE)
		};
		nglViewportScissor(m_transparent_render, viewport, viewport);
		nglBlendState(m_transparent_render, 0, NGL_BLEND_TRANSPARENT_ACCUMULATION, NGL_CHANNEL_ALL);
		nglDepthState(m_transparent_render, NGL_DEPTH_DISABLED, false);
	}

	// SSAO job
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_ssao_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "ssao::ssao";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_ssao_render = nglGenJob(rrd);

		nglViewportScissor(m_ssao_render, ssao_viewport, ssao_viewport);
	}

	// Tonemapper job
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_final_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "final";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_tonemapper_render = nglGenJob(rrd);

		nglViewportScissor(m_tonemapper_render, scene_viewport, scene_viewport);
		nglBlendStateAll(m_tonemapper_render, NGL_BLEND_DISABLED);
		nglDepthState(m_tonemapper_render, NGL_DEPTH_DISABLED, false);
	}

	//ovr_gen::gen_cubemap(&ad);

	// Debug view job (renders other render targets)
	if (IsEditorMode())
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = 0;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "debug_view";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_debug_view_render = nglGenJob(rrd);

		nglViewportScissor(m_debug_view_render, scene_viewport, scene_viewport);
	}

	{
		INFO("Init meshes...");
		std::map<Mesh3*, int> upload_flags;
		if (m_rooms.empty() == false)
		{
			for (size_t i = 0; i < m_rooms[0]->m_meshes.size(); i++)
			{
				KCL::Mesh *mesh = m_rooms[0]->m_meshes[i];

				int flags = 0;

				flags |= (mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER) ? Mesh3::FLAG_CREATE_SHADOW_BUFFER : 0;
				flags |= (mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST) ? Mesh3::FLAG_SHADOW_UV0 : 0;

				upload_flags[(Mesh3*)mesh->m_mesh] |= flags;
			}
		}

		for (size_t i = 0; i < m_actors.size(); i++)
		{
			for (size_t j = 0; j < m_actors[i]->m_meshes.size(); j++)
			{
				KCL::Mesh *mesh = m_actors[i]->m_meshes[j];

				int flags = 0;

				flags |= (mesh->m_flags & KCL::Mesh::OF_SHADOW_CASTER) ? Mesh3::FLAG_CREATE_SHADOW_BUFFER : 0;
				flags |= (mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST) ? Mesh3::FLAG_SHADOW_UV0 : 0;

				upload_flags[(Mesh3*)mesh->m_mesh] |= flags;
			}
		}

		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			Mesh3 *gfxb_mesh = (Mesh3*)m_meshes[i];

			int flag = upload_flags[gfxb_mesh];

			if (IsEditorMode())
			{
				// In editor mode we always upload the uv channel to be able to set alfa test on the fly
				flag |= Mesh3::FLAG_CREATE_SHADOW_BUFFER;
				flag |= Mesh3::FLAG_SHADOW_UV0;
			}

			if (gfxb_mesh->UploadMesh(flag) == false)
			{
				INFO("Can not upload mesh to NGL: %s", gfxb_mesh->m_name.c_str());
				return KCL::KCL_TESTERROR_UNKNOWNERROR;
			}
		}
	}

	INFO("Init materials...");
	for (size_t i = 0; i < m_materials.size(); i++)
	{
		Material *m = (Material*)m_materials[i];

		KCL::KCL_Status status = m->Init();
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			INFO("Can not init material: %s", m->m_name.c_str());
			return status;
		}
	}

	INFO("Init engine components...");
	m_main_mesh_filter = new Scene5MainMeshFilter();
	m_main_frustum_cull = new FrustumCull(this, m_main_mesh_filter);
	m_main_frustum_cull->SetCullLights(true);
	m_main_frustum_cull->SetCullProbes(true);

	NormalizeEffectParameters();

	m_compute_hdr = new ComputeHDR();
	m_compute_hdr->Init( this, m_viewport_width, m_viewport_height, 4 );
	m_compute_hdr->SetSaveLuminanceValues(m_hdr_save_luminance_values);

	{
		std::string adaptation_mode_name;
		switch (m_hdr_adaptation_mode)
		{
		case ADAPTATION_ENABLED: adaptation_mode_name = "ENABLED"; break;
		case ADAPTATION_DISABLED: adaptation_mode_name = "DISABLED"; break;
		case ADAPTATION_PREDEFINED: adaptation_mode_name = "PREDEFINED"; break;
		default: adaptation_mode_name = "UNKNOWN"; break;
		}
		INFO("Adaptation mode: %s", adaptation_mode_name.c_str());
	}

	if (m_hdr_adaptation_mode == ADAPTATION_PREDEFINED)
	{
		m_compute_hdr->LoadLuminanceValues(GetPredefinedLuminanceFilename(m_tier_level_name));
	}

	if( IsEditorMode() || RenderFlagDisabled( RenderOpts::FLAG_BLOOM_MOBILE ) )
	{
		m_bloom = new Bloom();
		m_bloom->Init( m_shapes, m_compute_hdr, m_viewport_width / BLOOM_DOWNSCALE, m_viewport_height / BLOOM_DOWNSCALE, 4, m_bloom_strength, NGL_R8_G8_B8_A8_UNORM );
	}

	if( IsEditorMode() || RenderFlagEnabled( RenderOpts::FLAG_BLOOM_MOBILE ) )
	{
		m_bloom_mobile = new BloomMobile();
		m_bloom_mobile->Init( m_shapes, m_compute_hdr, uint32_t( scene_viewport[2] / float( BLOOM_DOWNSCALE * 2 ) ), uint32_t( scene_viewport[3] / float( BLOOM_DOWNSCALE * 2 ) ), 5, 15, 8, NGL_R8_G8_B8_A8_UNORM );
	}

	m_ssao_blur = new FragmentBlur();
	m_ssao_blur->SetPrecision("half");
	m_ssao_blur->SetComponentCount(1);
	m_ssao_blur->Init( "ssao::blur", m_shapes, m_ssao_texture, m_viewport_width / SSAO_DOWNSCALE, m_viewport_height / SSAO_DOWNSCALE, m_ssao_blur_strength, NGL_R8_UNORM, 1 );

	m_motion_blur = new ComputeMotionBlur();
	m_motion_blur->Init(m_shapes, m_viewport_width, m_viewport_height, m_motion_blur_strength, ComputeMotionBlur::Adaptive);
	m_motion_blur->SetInputTextures(m_final_texture, m_gbuffer_velocity_texture, m_gbuffer_depth_texture);

	InitDOF();

	if (half_res_transparents)
	{
		m_transparent_blur = new GFXB::FragmentBlur();
		m_transparent_blur->SetPrecision("half");
		m_transparent_blur->SetComponentCount(4);
		m_transparent_blur->Init("transp::blur", m_shapes, m_transparent_texture, m_viewport_width / TRANSPARENT_DOWNSCALE, m_viewport_height / TRANSPARENT_DOWNSCALE, m_transparent_blur_strength, NGL_R16_G16_B16_A16_FLOAT, 1);
	}

	if (IsEditorMode())
	{
		m_debug_renderer = new DebugRenderer();
		m_debug_renderer->Init(this, m_viewport_width, m_viewport_height, m_shapes, m_debug_texture, m_gbuffer_depth_texture);
	}

	// Shadow init
	{
		m_shadow_mesh_filter = new Scene5ShadowMeshFilter();

		InitShadowMaps();
	}

	/*
	// Init font renderer
	{
		KCL::AssetFile font_file("fonts/verdana12o.fnt");
		assert(font_file.Opened());

		m_font_renderer = new FontRenderer();
		m_font_renderer->Init(font_file.GetBuffer(), 0, m_viewport_width, m_viewport_height);

		std::string font_file_name = std::string("fonts/") + m_font_renderer->GetTextureFilename();
		GFXB::Texture* m_font_texture = (GFXB::Texture*) TextureFactory().CreateAndSetup(KCL::Texture_2D, font_file_name.c_str(), KCL::TC_Clamp);
		assert(m_font_texture != NULL);

		m_font_renderer->SetTexture(m_font_texture->m_id, m_font_texture->getWidth(), m_font_texture->getHeight());
	}
	*/

	nglBeginCommandBuffer(m_command_buffer_default);

	IntegrateBRDFTexture(m_command_buffer_default);

	// GI
	if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		m_gi = new GI2();
		CreateGIStuff(m_command_buffer_default);
	}

	nglEndCommandBuffer(m_command_buffer_default);
	nglSubmitCommandBuffer(m_command_buffer_default);

	nglFinish();

	//nglEndStatistic();

	//Scene5Tools::SaveMemoryStatistics(m_glstatistic);

	SetInitialized(true);

	return KCL::KCL_TESTERROR_NOERROR;
}


void Scene5::InitDOF()
{
	const bool half_res_dof = RenderFlagEnabled(RenderOpts::FLAG_DOF_HALF_RES);

	if (m_dof)
	{
		if (m_dof->IsHalfResMode() == half_res_dof)
		{
			return;
		}
		else
		{
			delete m_dof;
		}
	}

	m_dof = new DOF();
	m_dof->Init(m_shapes, m_is_portrait ? m_viewport_height : m_viewport_width, m_is_portrait ? m_viewport_width : m_viewport_height, m_is_portrait, m_dof_strength, m_final_texture, IsEditorMode() ? std::vector<KCL::uint32>(1, m_debug_texture) : m_backbuffers, half_res_dof);
}


struct MeshState
{
	KCL::int32 m_filter_id;
	bool m_alpha_tested;
	bool m_skeletal;
	bool m_two_sided;
	bool m_dithered;
};


static void GetMeshState(MeshState &state, KCL::Mesh *mesh, KCL::int32 filter_id)
{
	memset(&state, 0, sizeof(MeshState));

	state.m_filter_id = filter_id;
	state.m_alpha_tested = mesh->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST;
	state.m_skeletal = mesh->m_mesh->m_vertex_matrix_indices.empty() == false;
	state.m_two_sided = mesh->m_material->m_is_two_sided;
	state.m_dithered = mesh->m_material->m_is_dithered;
}


static void CollectWarmUpMesh(std::vector<MeshState> &states, KCL::Mesh *mesh, FrustumCull *cull, MeshFilter *mesh_filter, KCL::Camera2 *camera)
{
	mesh->m_visible = true;

	KCL::int32 index = mesh_filter->FilterMesh(camera, mesh, KCL::OVERLAP_INSIDE);

	if (index < 0)
	{
		return;
	}

	std::vector<KCL::Mesh*> &dest = cull->m_visible_meshes[index];

	if (index == Scene5MainMeshFilter::MESH_OPAQUE || index == Scene5MainMeshFilter::MESH_ALPHA_TESTED)
	{
		MeshState state;
		GetMeshState(state, mesh, index);

		bool found = false;
		for (KCL::uint32 i = 0; i < states.size(); i++)
		{
			if (memcmp(&states[i], &state, sizeof(MeshState)) == 0)
			{
				found = true;
				break;
			}
		}

		if (found == false)
		{
			dest.push_back(mesh);
			states.push_back(state);
		}
	}
	else
	{
		// Others: glow, fire, emissive
		dest.push_back(mesh);
	}
}


KCL::KCL_Status Scene5::Warmup()
{
	KCL::uint32 command_buffer = m_command_buffer_default;

	// Warmup GI compute shader
	if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING))
	{
		INFO("Warmup GI...");
		m_gi->WarmupGenerateSH(m_warmup_helper, command_buffer);
	}

	// Warmup motion blur shader
	if (RenderFlagEnabled(GFXB::RenderOpts::FLAG_MOTION_BLUR))
	{
		INFO("Warmup motion blur...");
		// TODO: setup correct frames for warmup
		m_motion_blur->InitSceneForWarmup(this, 1000);
		m_motion_blur->WarmupNeighbormaxCompute(m_warmup_helper, command_buffer);
		m_motion_blur->WarmupTilemaxCompute(m_warmup_helper, command_buffer);
	}

	// Warmup particle system
	if (RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS))
	{
		INFO("Warmup particle systems...");

		nglBeginCommandBuffer(command_buffer);

		Transitions::Get()
			.TextureBarrier(m_depth_downsample_texture, NGL_SHADER_RESOURCE)
			.TextureBarrier(m_transparent_blur->GetOutputTexture(), NGL_COLOR_ATTACHMENT)
			.Execute(command_buffer);

		KCL::uint32 particle_render = m_transparent_blur->GetHorizontalJob();

		nglBlendState(particle_render, 0, NGL_BLEND_TRANSPARENT_ACCUMULATION, NGL_CHANNEL_ALL);

		nglBegin(particle_render, command_buffer);
		m_particlesystem_manager->Warmup(particle_render, m_depth_downsample_texture, m_active_camera);
		nglEnd(particle_render);

		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);

		nglFinish();
	}

	{
		INFO("Warmup HDR...");
		m_compute_hdr->Warmup(command_buffer, m_lighting_texture);
	}

	{
		INFO("Warmup DoF...");
		std::vector<ShotHandler::CameraShot> shots;
		GetCameraShotHandler()->GetCameraShots(shots);

		for (size_t j = 0; j < shots.size(); j++)
		{
			GFXB::Environment::Values v;
			GetEnvironment()->GetValues(v, shots[j].m_id);

			m_dof_strength = v.m_dof_values.m_strength;
			m_dof_normalized_strength = (m_dof_strength * m_viewport_height) / 1080;

			m_dof->SetKernelSize(m_dof_normalized_strength);

			for (size_t i = 0; i < m_backbuffers.size(); i++)
			{
				nglBeginCommandBuffer(command_buffer);

				m_dof->SetOutputBufferId(KCL::uint32(i));
				m_dof->Render(command_buffer, m_active_camera, m_gbuffer_depth_texture, 1.0f);

				nglEndCommandBuffer(command_buffer);
				nglSubmitCommandBuffer(command_buffer);
			}
		}
	}

	// Setup scene for warmup
	std::vector<std::vector<KCL::Mesh*>> &vis_meshes = m_main_frustum_cull->m_visible_meshes;
	{
		m_animation_time = 0;
		Animate();

		// Clear the frustum cull
		m_main_frustum_cull->Reset();

		std::vector<MeshState> states;

		// Collect actor meshes and lights
		for (size_t i = 0; i < m_actors.size(); i++)
		{
			for (size_t j = 0; j < m_actors[i]->m_meshes.size(); j++)
			{
				CollectWarmUpMesh(states, m_actors[i]->m_meshes[j], m_main_frustum_cull, m_main_mesh_filter, m_active_camera);
			}

			for (size_t j = 0; j < m_actors[i]->m_lights.size(); j++)
			{
				Light *light = (Light*)m_actors[i]->m_lights[j];

				light->m_visible = true;
				light->Animate(0);

				m_main_frustum_cull->m_visible_lights.push_back(light);

				if (light->m_has_lightshaft && m_main_frustum_cull->m_visible_light_shafts.empty())
				{
					m_main_frustum_cull->m_visible_light_shafts.push_back(light);
				}
			}
		}

		// Collect meshes from rooms
		states.clear();
		if (m_rooms.empty() == false)
		{
			for (size_t i = 0; i < m_rooms[0]->m_meshes.size(); i++)
			{
				CollectWarmUpMesh(states, m_rooms[0]->m_meshes[i], m_main_frustum_cull, m_main_mesh_filter, m_active_camera);
			}

			if (m_rooms[0]->m_probes.size() > 1)
			{
				m_main_frustum_cull->m_visible_probes.insert(m_main_frustum_cull->m_visible_probes.end(), m_rooms[0]->m_probes.begin(), m_rooms[0]->m_probes.end());

				m_main_frustum_cull->m_visible_inside_probes.push_back(m_rooms[0]->m_probes[0]);
				m_main_frustum_cull->m_visible_outside_probes.insert(m_main_frustum_cull->m_visible_outside_probes.end(), ++m_rooms[0]->m_probes.begin(), m_rooms[0]->m_probes.end());
			}
		}
	}

	{
		INFO("Warm up shadow maps...");
		std::vector<KCL::Mesh*> shadow_meshes;
		shadow_meshes.insert(shadow_meshes.end(), vis_meshes[Scene5MainMeshFilter::MESH_OPAQUE].begin(), vis_meshes[Scene5MainMeshFilter::MESH_OPAQUE].end());
		shadow_meshes.insert(shadow_meshes.end(), vis_meshes[Scene5MainMeshFilter::MESH_OPAQUE_EMISSIVE].begin(), vis_meshes[Scene5MainMeshFilter::MESH_OPAQUE_EMISSIVE].end());
		shadow_meshes.insert(shadow_meshes.end(), vis_meshes[Scene5MainMeshFilter::MESH_ALPHA_TESTED].begin(), vis_meshes[Scene5MainMeshFilter::MESH_ALPHA_TESTED].end());

		nglBeginCommandBuffer(command_buffer);
		for (KCL::uint32 i = 0; i < m_main_frustum_cull->m_visible_lights.size(); i++)
		{
			Light *light = (Light*)m_main_frustum_cull->m_visible_lights[i];
			if (light->GetShadowMap())
			{
				light->GetShadowMap()->Warmup(command_buffer, shadow_meshes);
			}
		}
		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);
	}

	// Warmup the scene
	{
		INFO("Warm scene...");
		{
			if (m_lightshaft_util)
			{
				m_lightshaft_util->SetWarmup(true);
			}

			if (m_motion_blur)
			{
				m_motion_blur->SetEnabled(true);
			}

			RenderAndClose();

			if (m_lightshaft_util)
			{
				m_lightshaft_util->SetWarmup(false);
			}
		}
	}

	// Restore particle system state from file

	if (GetSingleFrame() >= 0)
	{
		m_particlesystem_manager->RestoreState(GetSingleFrame());
	}

	m_animation_time = 0;

	GetCameraShotHandler()->Reset();
	GetComputeHDR()->Reset();
    
    GFXB::ShaderFactory::Release();

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status Scene5::OnParamsLoaded()
{
	return InitTierLevel();
}


KCL::KCL_Status Scene5::InitTierLevel()
{
	m_tier = Scene5Tools::ResolveTierLevel(m_tier_level_name);
	if (m_tier == TIER_INVALID)
	{
		INFO("Unknown tier level: %s", m_tier_level_name.c_str());
		return KCL::KCL_TESTERROR_INVALID_SCENE_VERSION;
	}

	INFO("Tier level: %s", m_tier_level_name.c_str());;

	if (m_tier == TIER_HIGH)
	{
		m_motion_blur_strength = HIGH_MOTION_BLUR_STRENGTH;
		m_transparent_blur_strength = HIGH_TRANSPARENT_BLUR_STRENGTH;
		m_ssao_blur_strength = HIGH_SSAO_BLUR_STRENGTH;
		m_bloom_strength = HIGH_BLOOM_STRENGTH;
		m_max_num_envprobes = HIGH_MAX_NUM_ENVPROBES;
	}

	if (m_tier == TIER_NORMAL)
	{
		m_texture_factory->SetTextureSizeLimit(1024);

		SetRenderFlag(RenderOpts::FLAG_PARTICLE_SYSTEMS, false);
		SetRenderFlag(RenderOpts::FLAG_LIGHTSHAFT, false);
		SetRenderFlag(RenderOpts::FLAG_MID_RANGE_SSAO, false);
		SetRenderFlag( RenderOpts::FLAG_BLOOM_MOBILE, true );
		SetRenderFlag( RenderOpts::FLAG_DOF_HALF_RES, true );

		m_motion_blur_strength = NORMAL_MOTION_BLUR_STRENGTH;
		m_transparent_blur_strength = NORMAL_TRANSPARENT_BLUR_STRENGTH;
		m_ssao_blur_strength = NORMAL_SSAO_BLUR_STRENGTH;
		m_bloom_strength = NORMAL_BLOOM_STRENGTH;
		m_max_num_envprobes = NORMAL_MAX_NUM_ENVPROBES;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


void Scene5::InitShadowMaps()
{
	for (size_t i = 0; i < m_actors.size(); i++)
	{
		for (size_t j = 0; j < m_actors[i]->m_lights.size(); j++)
		{
			Light *light = (Light*)m_actors[i]->m_lights[j];

			if (!light->m_light_shape->m_is_shadow_caster)
			{
				continue;
			}

			std::string shadow_light_name = (light->m_foreign_shadow_light_name.empty()) ? light->m_name : light->m_foreign_shadow_light_name;

			ShadowMap* sm = nullptr;
			for (std::vector<ShadowMap*>::const_iterator it = m_shadow_maps.begin(); it != m_shadow_maps.end(); it++)
			{
				if ((*it)->GetLight()->m_name == shadow_light_name)
				{
					sm = *it;
					break;
				}
			}

			if (sm == nullptr)
			{
				int shadow_map_size = m_fboShadowMap_size;

				if (light->m_light_shape->m_light_type == KCL::LightShape::OMNI)
				{
					shadow_map_size /= 2;
				}

				sm = ShadowMap::CreateShadowMap(this, light, m_shadow_mesh_filter, shadow_map_size, NGL_D24_UNORM);
				m_shadow_maps.push_back(sm);
			}

			light->SetShadowMap(sm);
		}
	}
}


void Scene5::InitShaders()
{
    ShaderFactory::CreateInstance();
	ShaderFactory *shader_factory = ShaderFactory::GetInstance();

	InitShaderFactory(shader_factory);

	// Material shaders
	shader_factory->AddDescriptor(ShaderDescriptor("main.vert", "main.frag").SetName("main").AddHeaderFile("velocity.h"));
	shader_factory->AddDescriptor(ShaderDescriptor("sky.vert", "sky.frag").SetName("sky"));
	shader_factory->AddDescriptor(ShaderDescriptor("fire.vert", "fire.frag").SetName("fire").AddHeaderFile("billboard.h"));
	shader_factory->AddDescriptor(ShaderDescriptor("glow.vert", "glow.frag").SetName("glow").AddHeaderFile("billboard.h"));

	// Light shaders
	m_omni_light_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_omni.vert", "light_omni.frag"));
	m_omni_light_shadow_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_omni.vert", "light_omni.frag").AddDefine("HAS_SHADOW"));

	m_directional_light_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_directional.vert", "light_directional.frag"));
	m_directional_light_shadow_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_directional.vert", "light_directional.frag").AddDefine("HAS_SHADOW").AddHeaderFile("sun_shadow.h"));
	m_directional_light_cube_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_directional_cube.vert", "light_directional_cube.frag"));
	m_directional_light_cube_shadow_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_directional_cube.vert", "light_directional_cube.frag").AddDefine("HAS_SHADOW").AddDefineFloat("SHADOWMAP_SIZE", (float)m_fboShadowMap_size).AddDefineInt("LINEAR_SHADOW_FILTER", nglGetInteger(NGL_D24_LINEAR_SHADOW_FILTER)).AddHeaderFile("shadow.h"));
	m_spot_light_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_spot.vert", "light_spot.frag"));
	m_spot_light_shadow_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_spot.vert", "light_spot.frag").AddDefine("HAS_SHADOW").AddDefineFloat("SHADOWMAP_SIZE", (float)m_fboShadowMap_size).AddDefineInt("LINEAR_SHADOW_FILTER", nglGetInteger(NGL_D24_LINEAR_SHADOW_FILTER)).AddHeaderFile("shadow.h"));

	m_emissive_light_shader = shader_factory->AddDescriptor(ShaderDescriptor("light_emissive.vert", "light_emissive.frag"));

	m_glow_shader = shader_factory->AddDescriptor(ShaderDescriptor("glow.vert", "glow.frag").AddHeaderFile("billboard.h"));

	ShaderDescriptor ibl_descriptor("ibl.vert", "ibl.frag");
	ibl_descriptor.AddDefineFloat("MAX_LOD_LEVEL", (float)KCL::texture_levels(m_prefiltered_cubemap_size, m_prefiltered_cubemap_size));
	if (RenderFlagEnabled(RenderOpts::FLAG_DIRECT_SHADOWS))
	{
		ibl_descriptor.AddDefineInt("HAS_SHADOW", 1);
	}
	m_ibl_shader = shader_factory->AddDescriptor(ibl_descriptor);

	ShaderDescriptor irradiance_light_shader_desc = ShaderDescriptor("light_irradiance.vert", "light_irradiance.frag");
	irradiance_light_shader_desc.AddDefineFloat("MAX_NUM_OF_PROBES", float(m_max_num_envprobes));
	irradiance_light_shader_desc.AddDefineFloat("MAX_LOD_LEVEL", (float)KCL::texture_levels(m_prefiltered_cubemap_size, m_prefiltered_cubemap_size));
	if (RenderFlagEnabled(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS))
	{
		irradiance_light_shader_desc.AddDefineInt("USE_TEXTURE_SH_ATLAS", 1);
	}

	m_irradiance_light_shader = shader_factory->AddDescriptor(irradiance_light_shader_desc);

	m_normalize_irradiance_light_shader = shader_factory->AddDescriptor(ShaderDescriptor("normalize_indirect.vert", "normalize_indirect.frag"));

	// SSAO shaders
	m_ssao_shader = shader_factory->AddDescriptor(ShaderDescriptor("sao.vert", "sao.frag"));
	m_ssao_apply_shader = shader_factory->AddDescriptor(ShaderDescriptor("apply_ssao.vert", "apply_ssao.frag"));

	// Tonemapper shader
	m_tonemapper_manual_exposure_shader = shader_factory->AddDescriptor(ShaderDescriptor("final.vert", "final.frag").AddHeaderFile("tonemapper.h").AddHeaderFile("fog.h").AddDefine("EXPOSURE_MANUAL"));
	m_tonemapper_auto_exposure_shader = shader_factory->AddDescriptor(ShaderDescriptor("final.vert", "final.frag").AddHeaderFile("tonemapper.h").AddHeaderFile("fog.h"));

	m_forward_shader = shader_factory->AddDescriptor(ShaderDescriptor("forward.vert", "forward.frag"));

	// Debug view shader
	ShaderDescriptor debug_view("debug/debug_view.vert", "debug/debug_view.frag");
	debug_view.AddDefineInt("DEBUG_VIEW_ALBEDO", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_ALBEDO || m_debug_view == RenderOpts::DEBUG_VIEW_TEXTURE_DENSITY || m_debug_view == RenderOpts::DEBUG_VIEW_VISUALIZE_MIPMAP));
	debug_view.AddDefineInt("DEBUG_VIEW_ALPHA", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_ALPHA));
	debug_view.AddDefineInt("DEBUG_VIEW_SPECULAR", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_SPECULAR));
	debug_view.AddDefineInt("DEBUG_VIEW_EMISSIVE", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_EMISSIVE));
	debug_view.AddDefineInt("DEBUG_VIEW_GLOSSNESS", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_GLOSSNESS));
	debug_view.AddDefineInt("DEBUG_VIEW_ROUGHNESS", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_ROUGHNESS));
	debug_view.AddDefineInt("DEBUG_VIEW_NORMALS", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_NORMALS));
	debug_view.AddDefineInt("DEBUG_VIEW_VELOCITY", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_VELOCITY));
	debug_view.AddDefineInt("DEBUG_VIEW_DEPTH", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DEPTH));
	debug_view.AddDefineInt("DEBUG_VIEW_DEPTH_LINEAR", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DEPTH_LINEAR));
	debug_view.AddDefineInt("DEBUG_VIEW_DEPTH_DOWNSAMPLE", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DEPTH_DOWNSAMPLE));
	debug_view.AddDefineInt("DEBUG_VIEW_ENERGY_CONSERVATIVITY_CHECK", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_ENERGY_CONSERVATIVITY_CHECK));
	debug_view.AddDefineInt("DEBUG_VIEW_SSAO", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_SSAO));
	debug_view.AddDefineInt("DEBUG_VIEW_BRIGHT", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_BRIGHT));
	debug_view.AddDefineInt("DEBUG_VIEW_BLOOM", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_BLOOM));
	debug_view.AddDefineInt("DEBUG_VIEW_DOF", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DOF));
	debug_view.AddDefineInt("DEBUG_VIEW_DOF_INPUT", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DOF_INPUT));
	debug_view.AddDefineInt("DEBUG_VIEW_HALF_RES_TRANSPARENT", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_HALF_RES_TRANSPARENT));
	debug_view.AddDefineInt("DEBUG_VIEW_NEIGHBOR_MAX", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_NEIGHBOR_MAX));
	debug_view.AddDefineInt("DEBUG_VIEW_MOTION_BLUR", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_MOTION_BLUR));
	debug_view.AddDefineInt("DEBUG_VIEW_DIRECT_SHADOW", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_DIRECT_SHADOW));
	debug_view.AddDefineInt("DEBUG_VIEW_INDIRECT_SHADOW", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_INDIRECT_SHADOW));
	debug_view.AddDefineInt("DEBUG_VIEW_TEXTURE_DENSITY", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_TEXTURE_DENSITY));
	debug_view.AddDefineInt("DEBUG_VIEW_VISUALIZE_MIPMAP", KCL::int32(m_debug_view == RenderOpts::DEBUG_VIEW_VISUALIZE_MIPMAP));
	debug_view.AddDefineInt("DEBUG_SHADOW_ARRAY", KCL::int32(m_debug_shadow_map && m_debug_shadow_map->GetType() == ShadowMap::PARABOLOID));
	debug_view.AddDefineInt("DEBUG_SHADOW_CUBE", KCL::int32(m_debug_shadow_map && m_debug_shadow_map->GetType() == ShadowMap::CUBE));

	m_debug_view_shader = shader_factory->AddDescriptor(debug_view);

	m_lightshaft_util = new LightshaftUtil();
	m_lightshaft_util->Init(m_viewport_width, m_viewport_height);
	m_upsample_shader = shader_factory->AddDescriptor(ShaderDescriptor("bilateral_upsample.vert", "bilateral_upsample.frag"));
	m_downsample_depth_shader = shader_factory->AddDescriptor(ShaderDescriptor("depth_downsample.vert", "depth_downsample.frag").AddDefineInt("LINEARIZE_DEPTH", 0));
	m_downsample_depth_linearize_shader = shader_factory->AddDescriptor( ShaderDescriptor("depth_downsample.vert", "depth_downsample.frag").AddDefineInt("LINEARIZE_DEPTH", 1));
}


void Scene5::InitShaderFactory(ShaderFactory *shader_factory)
{
	shader_factory->ClearGlobals();

	shader_factory->AddShaderSubdirectory("shaders/scene5");

	if (ForceHighp())
	{
		INFO("Force highp enabled");
		shader_factory->SetForceHighp(true);
	}

	shader_factory->AddGlobalHeaderFile("common.h");

	switch (nglGetApi())
	{
	case NGL_OPENGL:
	{
		shader_factory->SetGlobalHeader("#version 430 core\n");
		shader_factory->AddGlobalDefine("SHADER_GLSL");
		break;
	}

	case NGL_OPENGL_ES:
	{
		std::stringstream sstream;
		if (nglGetMinor() == 0)
		{
			sstream << "#version 300 es" << std::endl;
		}
		else if (nglGetMinor() == 1)
		{
			sstream << "#version 310 es" << std::endl;
		}
		else if (nglGetMinor() == 2)
		{
			sstream << "#version 320 es" << std::endl;
		}
		sstream << "precision highp float;" << std::endl;
		sstream << "precision highp sampler2D;" << std::endl;
		sstream << "precision highp sampler2DArray;" << std::endl;
		sstream << "precision highp sampler2DArrayShadow;" << std::endl;
		sstream << "precision highp samplerCube;" << std::endl;
		sstream << "precision highp samplerCubeShadow;" << std::endl;
		sstream << "precision highp image2D;" << std::endl;

		shader_factory->SetGlobalHeader(sstream.str().c_str());

		shader_factory->AddGlobalDefine("SHADER_GLSL");
		break;
	}

	case NGL_VULKAN:
	{
		std::stringstream sstream;

#if 1
		sstream << "#version 310 es" << std::endl;
		sstream << "#extension GL_OES_shader_io_blocks : require" << std::endl;
#else
		sstream << "#version 430 core" << std::endl;
#endif

		shader_factory->SetGlobalHeader(sstream.str().c_str());
		shader_factory->AddGlobalDefine("SHADER_VULKAN");
		break;
	}

	case NGL_DIRECT3D_11:
		shader_factory->AddGlobalDefine("SHADER_HLSL");
		break;

	case NGL_DIRECT3D_12:
		shader_factory->AddGlobalDefine("SHADER_HLSL");
		break;

	case NGL_METAL_IOS:
		shader_factory->AddGlobalDefine("SHADER_METAL_IOS");
		shader_factory->AddGlobalDefine("SHADER_METAL");
		break;

	case NGL_METAL_MACOS:
		shader_factory->AddGlobalDefine("SHADER_METAL_OSX");
		shader_factory->AddGlobalDefine("SHADER_METAL");
		break;

	default:
		break;
	}

	switch (nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE))
	{
	case NGL_ORIGIN_LOWER_LEFT: shader_factory->AddGlobalDefine("NGL_ORIGIN_LOWER_LEFT"); break;
	case NGL_ORIGIN_UPPER_LEFT: shader_factory->AddGlobalDefine("NGL_ORIGIN_UPPER_LEFT"); break;
	case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP: shader_factory->AddGlobalDefine("NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP"); break;
	default: assert(0); break;
	}

	switch (nglGetInteger(NGL_DEPTH_MODE))
	{
	case NGL_ZERO_TO_ONE: shader_factory->AddGlobalDefine("NGL_ZERO_TO_ONE"); break;
	case NGL_NEGATIVE_ONE_TO_ONE: shader_factory->AddGlobalDefine("NGL_NEGATIVE_ONE_TO_ONE"); break;
	default: assert(0); break;
	}

	const char *sky_format = GetEnvironment()->GetValues().m_sky_values.GetSkyFormatString();
	INFO("Sky encoding: %s", sky_format);
	shader_factory->AddGlobalDefine(sky_format);

	shader_factory->AddGlobalDefineInt("NORMAL_MAPPING_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_NORMAL_MAPPING));
	shader_factory->AddGlobalDefineInt("TONEMAPPER_FILMIC", RenderFlagEnabled(RenderOpts::FLAG_HDR));
	shader_factory->AddGlobalDefineInt("BLOOM_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_BLOOM));
	shader_factory->AddGlobalDefineInt("BLOOM_MOBILE", RenderFlagEnabled(RenderOpts::FLAG_BLOOM_MOBILE));
	shader_factory->AddGlobalDefineInt("GAMMA_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_GAMMA_CORRECTION));
	shader_factory->AddGlobalDefineInt("GAMMA_CORRECTION_FAST", RenderFlagEnabled(RenderOpts::FLAG_GAMMA_CORRECTION_FAST));
	shader_factory->AddGlobalDefineInt("SHARPEN_FILTER", RenderFlagEnabled(RenderOpts::FLAG_SHARPEN_FILTER));
	shader_factory->AddGlobalDefineInt("IBL_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_IBL));
	shader_factory->AddGlobalDefineInt("GI_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING));
	shader_factory->AddGlobalDefineInt("SSAO_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_SSAO));
	shader_factory->AddGlobalDefineInt("MID_RANGE_SSAO_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_MID_RANGE_SSAO));
	shader_factory->AddGlobalDefineInt("RENDER_PROBES_SH", RenderFlagEnabled(RenderOpts::FLAG_RENDER_PROBES_SH));
	shader_factory->AddGlobalDefineInt("MOTION_BLUR_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_MOTION_BLUR));
	shader_factory->AddGlobalDefineInt("PARTCILE_SYSTEMS_ENABLED", RenderFlagEnabled(RenderOpts::FLAG_PARTICLE_SYSTEMS));
	shader_factory->AddGlobalDefineInt("COLORIZE_SHADOW_CASTERS", RenderFlagEnabled(RenderOpts::FLAG_COLORIZE_SHADOW_CASTERS));
	shader_factory->AddGlobalDefineInt("COLORIZE_LOD_LEVELS", RenderFlagEnabled(RenderOpts::FLAG_COLORIZE_LOD_LEVELS));
	shader_factory->AddGlobalDefineInt("WIREFRAME", RenderFlagEnabled(RenderOpts::FLAG_WIREFRAME) || RenderFlagEnabled(RenderOpts::FLAG_WIREFRAME_SOLID));
}


void Scene5::IntegrateBRDFTexture(KCL::uint32 command_buffer)
{
	if (RenderFlagDisabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING) && RenderFlagDisabled(RenderOpts::FLAG_IBL))
	{
		return;
	}

	if (m_integrate_brdf_lut_texture == 0)
	{
		INFO("Integrate BRDF look up texture...");
		m_integrate_brdf_lut_texture = CreateIntegrateBRDF_LUT(command_buffer, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, LUT_TEXTURE_SIZE);
	}
}


void Scene5::InitIBL()
{
	if (m_prefiltered_cubemap_texture == 0)
	{
		INFO("Loading IBL cubemap: ibl.pvr");
		KCL::KCL_Status status = LoadCubemapRGB9E5("ibl.pvr", m_prefiltered_cubemap_texture, m_prefiltered_cubemap_size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			INFO("Can not load IBL texture!");
		}
		INFO("IBL cubemap size: %dx%d", m_prefiltered_cubemap_size, m_prefiltered_cubemap_size);
	}
}


void Scene5::CreateGIStuff(KCL::uint32 command_buffer)
{
	// TODO: compute generate sh not work on vulkan
	m_gi->Init(m_max_num_envprobes, m_final_texture, m_gbuffer_depth_texture, true /*nglGetApi() != NGL_VULKAN*/, RenderFlagEnabled(RenderOpts::FLAG_GI_USE_TEXTURE_SH_ATLAS));

	//for (size_t j = 1; j < m_rooms.size(); j++) m_rooms[j]->m_meshes.clear();

	if (!m_irradiance_meshes.empty())
	{
		for (size_t i = 0; i < m_irradiance_meshes.size(); ++i)
		{
			m_gi->m_meshes.push_back(m_irradiance_meshes[i]);

			//	for (size_t j = 1; j < m_rooms.size(); j++)	m_rooms[j]->m_meshes.push_back(m_irradiance_meshes[i]);
			//	m_shadow_mesh_filter->m_excluded_meshes.push_back(m_irradiance_meshes[i]);
		}
	}
	else
	{
		std::string gi_mesh_filename("meshes/irradiance_model_shapeShape");
		KCL::AssetFile file(gi_mesh_filename);

		if (file.Opened())
		{
			GFXB::Mesh3 * mesh3 = (GFXB::Mesh3 *)Mesh3Factory().Create(gi_mesh_filename.c_str());

			KCL::uint32 skin_data_exist;
			KCL::uint32 num_vertices;
			KCL::uint32 num_indices;

			file.Read(&num_vertices, 4, 1);
			file.Read(&num_indices, 4, 1);

			mesh3->m_vertex_attribs3[0].resize(num_vertices);
			mesh3->m_vertex_attribs3[1].resize(num_vertices);
			mesh3->m_vertex_attribs3[2].resize(num_vertices);
			mesh3->m_vertex_attribs2[0].resize(num_vertices);
			mesh3->m_vertex_attribs2[1].resize(num_vertices);
			mesh3->m_vertex_indices[0].resize(num_indices);

			file.Read(&mesh3->m_vertex_indices[0][0], 2, num_indices);
			file.Read(mesh3->m_vertex_attribs3[0][0].v, 4, num_vertices * 3);
			file.Read(mesh3->m_vertex_attribs3[1][0].v, 4, num_vertices * 3);
			file.Read(mesh3->m_vertex_attribs3[2][0].v, 4, num_vertices * 3);
			file.Read(mesh3->m_vertex_attribs2[0][0].v, 4, num_vertices * 2);
			file.Read(&skin_data_exist, 4, 1);

			file.Read(mesh3->m_vertex_attribs2[1][0].v, 4, num_vertices * 2);

			for (size_t i = 0; i < mesh3->m_vertex_attribs2[0].size(); i++)
			{
				mesh3->m_vertex_attribs2[0][i].y = 1.0f - mesh3->m_vertex_attribs2[0][i].y;
			}

			mesh3->UploadMesh();

			KCL::Mesh *gi_mesh = m_mesh_factory->Create("gi_mesh", nullptr, nullptr);
			gi_mesh->m_mesh = mesh3;
			//gi_mesh->m_material = m_materials[10];

			//m_rooms[0]->m_meshes.clear();
			//m_rooms[0]->m_meshes.push_back(gi_mesh);

			m_gi->m_meshes.push_back(gi_mesh);
		}
		else
		{
			INFO("Error: %s is missing!", gi_mesh_filename.c_str());
		}
	}

#if 0
	for (int x = -2; x <= 1; x++)
	{
		for (int y = -2; y <= 1; y++)
		{
			float dy = 0.0f;

			if (x == 0 && y == 0)
				dy += 2.0f;

			if (x == 1 && y == 1)
				continue;
			if (x == -2 && y == 1)
				continue;

			m_gi->AddEnvprobe(x * 5 + 2.5f, dy + 2.0f, y * 5 + 2.5f);
			m_gi->AddEnvprobe(x * 5 + 2.5f, dy + 7.0f, y * 5 + 2.5f);
		}
	}
#endif
#if BEFORE_ALPHA4_METHOD
	for (size_t i = 0; i < m_envmap_descriptors.size(); i++)
	{
		const KCL::Vector3D &pos = m_envmap_descriptors[i].m_position;
		m_gi->AddEnvprobe(pos.x, pos.z, -pos.y);
	}
#else


	std::string filename("probes.json");
	KCL::AssetFile file(filename, true);
	if (!file.Opened())
	{
		INFO("Can not load probes: %s", filename.c_str());
		return;
	}

	std::string json_input = file.ReadToStdString();
	JsonSerializer s(false);
	ng::Result result;
	s.JsonValue.fromString(json_input.c_str(), result);
	m_selected_probes.Serialize(s);

	m_debug_nodes2 = m_selected_probes.m_sampling_position;

	m_gi->save_textures = false;

	UpdateGIProbes(command_buffer);
#endif
}


void Scene5::UpdateGIProbes(KCL::uint32 command_buffer)
{
	for (size_t i = 0; i < m_rooms.size(); i++)
	{
		m_rooms[i]->m_probes.clear();
	}

	for (size_t i = 0; i < m_probes.size(); i++)
	{
		delete m_probes[i];
	}

	m_probes.clear();

	KCL::uint32 probe_counter = 0;

	for (size_t i = 0; i < m_selected_probes.m_position.size(); ++i)
	{
		KCL::EnvProbe *probe = new KCL::EnvProbe();
		probe->m_pos = m_selected_probes.m_position[i];
		probe->m_sampling_pos = m_selected_probes.m_sampling_position[i];
		probe->m_half_extent = m_selected_probes.m_half_extent[i];
		probe->m_aabb.SetWithHalfExtentCenter(m_selected_probes.m_half_extent[i], m_selected_probes.m_position[i]);
		probe->m_index = probe_counter++;
		m_probes.push_back(probe);
	}

	RoomSetup::PlaceProbes(this);

	m_gi->ClearEnvProbes();

	for (size_t i = 0; i < m_probes.size(); i++)
	{
		m_gi->AddEnvProbe(m_probes[i]);
	}

	{
		Transitions::Get().TextureBarrier(m_gi->m_envprobe_indirect_uv_map, NGL_COLOR_ATTACHMENT).Execute(command_buffer);

		nglBegin(m_gi->m_envprobe_uv_refresh.m_clear_job, command_buffer);
		nglEnd(m_gi->m_envprobe_uv_refresh.m_clear_job);
	}

	std::vector<uint32_t> probe_indices;

	for (size_t i = 0; i < m_gi->m_envprobes.size(); i++)
	{
		probe_indices.push_back((uint32_t)i);
	}

	m_gi->envprobe_uv_refresh(command_buffer, probe_indices);
}


/*!
clear gi's probes
upload from scene's m_selected_probes to gi
save into file
*/
void Scene5::UpdateGIStuff(KCL::uint32 command_buffer)
{
	UpdateGIProbes(command_buffer);

	JsonSerializer s(true);
	m_selected_probes.Serialize(s);

	std::string filename("probes.json");
	KCL::File file(filename, KCL::Write, KCL::RWDir, true);
	if (!file.Opened())
	{
		INFO("Can not save probes: %s", filename.c_str());
		return;
	}
	file.Printf("%s", s.JsonValue.toString().c_str());
	file.Close();
}
