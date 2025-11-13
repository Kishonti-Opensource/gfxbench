/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_pipeline.h"
#include "gfxb_scene5_mesh_filters.h"
#include "gfxb_barrier.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_fragment_blur.h"
#include "common/gfxb_tools.h"
#include "common/gfxb_shot_handler.h"
#include "common/gfxb_light.h"
#include "common/gfxb_shadow_map.h"
#include "gi/gi.h"

using namespace GFXB;


bool COLOR_DEPTH_NEEDED()
{
	return ((nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS)) && nglGetInteger(NGL_SUBPASS_ENABLED);
}


Scene5Pipeline::Scene5Pipeline()
{
	m_command_buffer_shadow = 0;
	m_command_buffer_gi = 0;
	m_command_buffer_main = 0;

	m_deferred_render = 0;
	m_forward_render = 0;

	m_ssao_apply_shader = 0;

	m_gbuffer_color_depth_texture = 0;

	m_gbuffer_color_depth_attachment_id = 0;
	m_lighting_attachment_id = 0;
	m_lighting_weight_attachment_id = 0;
	
	m_deffered_render_run = false;
}


Scene5Pipeline::~Scene5Pipeline()
{
}


void Scene5Pipeline::RenderPipeline()
{
	{
		nglBeginCommandBuffer(m_command_buffer_shadow);

		RenderShadow(m_command_buffer_shadow);

		nglEndCommandBuffer(m_command_buffer_shadow);
		nglSubmitCommandBuffer(m_command_buffer_shadow);
	}

	{
		nglBeginCommandBuffer(m_command_buffer_gi);

		RenderGIProbes(m_command_buffer_gi);

		nglEndCommandBuffer(m_command_buffer_gi);
		nglSubmitCommandBuffer(m_command_buffer_gi);
	}

	nglBeginCommandBuffer(m_command_buffer_main);

	{
		Transitions &transitions = Transitions::Get();

		// Shadow map transitions
		for (size_t i = 0; i < m_main_frustum_cull->m_visible_lights.size(); i++)
		{
			Light *light = (Light*)m_main_frustum_cull->m_visible_lights[i];
			if (light->GetShadowMap() != nullptr)
			{
				transitions.TextureBarriers(light->GetShadowMap()->GetRenderTargets(), NGL_SHADER_RESOURCE);
			}
		}

		if (RenderFlagEnabled(RenderOpts::FLAG_IRRADIANCE_LIGHTING) && m_gi != nullptr)
		{
			if (m_gi->m_use_texture_sh_atlas)
			{
				transitions.TextureBarrier(m_gi->m_envprobe_sh_atlas_texture, NGL_SHADER_RESOURCE);
			}
			else
			{
				transitions.BufferBarrier(m_gi->m_envprobe_sh_atlas, NGL_SHADER_RESOURCE);
			}
		}

		transitions.TextureBarriers(m_subpass_begin_transitions);

		transitions.Execute(m_command_buffer_main);

		nglBegin(m_deferred_render, m_command_buffer_main);
		m_deffered_render_run = true;

		// G-buffer sub-pass
		{
			if (RenderFlagDisabled(RenderOpts::FLAG_WIREFRAME))
			{
				nglBlendStateAll(m_deferred_render, NGL_BLEND_DISABLED);
				nglDepthState(m_deferred_render, NGL_DEPTH_LESS, true);
				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE]);
				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE_EMISSIVE]);
				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_ALPHA_TESTED]);
			}

			// Wireframe render
			if (RenderFlagEnabled(RenderOpts::FLAG_WIREFRAME) || RenderFlagEnabled(RenderOpts::FLAG_WIREFRAME_SOLID))
			{
				// Turn on line mode
				nglCustomAction(0, 300);

				m_wireframe_uniform = 1.0f;

				nglBlendStateAll(m_deferred_render, NGL_BLEND_DISABLED);

				if (RenderFlagEnabled(RenderOpts::FLAG_WIREFRAME_SOLID))
				{
					nglDepthState(m_deferred_render, NGL_DEPTH_LESS_WITH_OFFSET, false);
					nglLineWidth(m_deferred_render, 2.0f);
				}
				else
				{
					nglDepthState(m_deferred_render, NGL_DEPTH_LESS, true);
					nglLineWidth(m_deferred_render, 1.0f);
				}

				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE]);
				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_OPAQUE_EMISSIVE]);
				RenderCamera(m_deferred_render, m_active_camera, m_main_frustum_cull->m_visible_meshes[Scene5MainMeshFilter::MESH_ALPHA_TESTED]);

				m_wireframe_uniform = 0.0f;

				// Turn off line mode
				nglCustomAction(0, 301);
			}
		}

		// light sub-pass
		{
			nglNextSubpass(m_deferred_render);

			nglBlendStateAll(m_deferred_render, NGL_BLEND_ADDITIVE);

			RenderIrradianceLights(m_deferred_render);

			nglNextSubpass(m_deferred_render);

			nglBlendStateAll(m_deferred_render, NGL_BLEND_MODULATIVE);

			NormalizeIndirectLighting(m_deferred_render);

			nglBlendStateAll(m_deferred_render, NGL_BLEND_ADDITIVE);

			RenderDirectLights(m_deferred_render);

			RenderIBL(m_deferred_render);
		}

		nglEnd(m_deferred_render);
		m_deffered_render_run = false;

		transitions.UpdateTextureStates(m_subpass_end_transitions);
	}

	DownsampleDepthBuffer(m_command_buffer_main);

	RenderHalfResTransparents(m_command_buffer_main);

	RenderSSAO(m_command_buffer_main);

	// Forward render
	{
		Transitions &transitions = Transitions::Get()
			.TextureBarrier(m_lighting_texture, NGL_COLOR_ATTACHMENT)
			.TextureBarrier(m_gbuffer_depth_texture, NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
			.TextureBarrier(m_ssao_blur->GetOutputTexture(), NGL_SHADER_RESOURCE);
		if (m_half_res_transparents_enabled)
		{
			transitions.TextureBarrier(m_transparent_blur->GetOutputTexture(), NGL_SHADER_RESOURCE);
		}
		transitions.Execute(m_command_buffer_main);

		nglBegin(m_forward_render, m_command_buffer_main);

		// Mix ssao to lighting
		nglBlendStateAll(m_forward_render, NGL_BLEND_MODULATIVE);
		nglDepthState(m_forward_render, NGL_DEPTH_DISABLED, false);
		RenderApplySSAO(m_forward_render);

		// Sky
		nglBlendStateAll(m_forward_render, NGL_BLEND_DISABLED);
		nglDepthState(m_forward_render, NGL_DEPTH_TO_FAR, false);
		RenderCamera(m_forward_render, m_active_camera, m_sky_mesh);

		nglBlendStateAll(m_forward_render, NGL_BLEND_ADDITIVE);
		RenderEmissiveLights(m_forward_render);

		// Render transparents
		RenderTransparents(m_forward_render);

		nglEnd(m_forward_render);
	}

	DispatchHDR(m_command_buffer_main);

	RenderBloom(m_command_buffer_main);

	// Tonemapper pass: color correction and tone mapping
	RenderTonemapperPass(m_command_buffer_main);

	RenderDebugProbes(m_command_buffer_main);

	DispatchMotionBlur(m_command_buffer_main);

	RenderMotionBlur(m_command_buffer_main);

	RenderDOF(m_command_buffer_main);
}


void Scene5Pipeline::GetCommandBufferConfiguration(KCL::uint32 &buffers_in_frame, KCL::uint32 &prerendered_frame_count)
{
	buffers_in_frame = 3;
	prerendered_frame_count = 3;
}


void Scene5Pipeline::SetCommandBuffers(const std::vector<KCL::uint32> &buffers)
{
	m_command_buffer_shadow = buffers[0];
	m_command_buffer_gi = buffers[1];
	m_command_buffer_main = buffers[2];

	m_command_buffer_default = buffers[0];
}


KCL::uint32 Scene5Pipeline::GetLastCommandBuffer()
{
	return m_command_buffer_main;
}


KCL::KCL_Status Scene5Pipeline::ReloadShaders()
{
	KCL::KCL_Status status = Scene5::ReloadShaders();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	nglDeletePipelines(m_deferred_render);
	nglDeletePipelines(m_forward_render);

	return KCL::KCL_TESTERROR_NOERROR;
}


void Scene5Pipeline::Resize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h)
{
	Scene5::Resize(x, y, w, h);

	KCL::int32 vp[4] = { 0, 0, (KCL::int32)w, (KCL::int32)h };
	nglViewportScissor(m_deferred_render, vp, vp);
	nglViewportScissor(m_forward_render, vp, vp);
}


void Scene5Pipeline::InitShaderFactory(ShaderFactory *shader_factory)
{
	Scene5::InitShaderFactory(shader_factory);
	InitSubpassDefines();
}


void Scene5Pipeline::InitSubpassDefines()
{
	ShaderFactory *shader_factory = ShaderFactory::GetInstance();

	if ((nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS))
	{
		shader_factory->AddGlobalDefineInt("LIGHTING_RT_INDEX", m_lighting_attachment_id);
		shader_factory->AddGlobalDefineInt("LIGHTING_WEIGHT_RT_INDEX", m_lighting_weight_attachment_id);
	}
	else
	{
		shader_factory->AddGlobalDefineInt("LIGHTING_RT_INDEX", 0);
		shader_factory->AddGlobalDefineInt("LIGHTING_WEIGHT_RT_INDEX", 1);
	}

	if (COLOR_DEPTH_NEEDED())
	{
		shader_factory->AddGlobalDefineInt("GBUFFER_COLOR_DEPTH_RT_INDEX", m_gbuffer_color_depth_attachment_id);
	}
}


void Scene5Pipeline::InitShaders()
{
	Scene5::InitShaders();
}


KCL::KCL_Status Scene5Pipeline::Init()
{
	INFO("Init render pipeline 1...");

	KCL::KCL_Status status = Scene5::Init();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	int32_t scene_viewport[4] =
	{
		0, 0, (int32_t)m_viewport_width, (int32_t)m_viewport_height
	};

	// GBuffer color depth texture
	if (COLOR_DEPTH_NEEDED())
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "gbuffer_color_depth";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_format = NGL_R32_FLOAT;
		texture_layout.m_is_renderable = true;
		texture_layout.m_memoryless = true;
		texture_layout.SetAllClearValue(1.0f);

		GenRenderTarget(m_gbuffer_color_depth_texture, texture_layout, nullptr);
	}

	// Deferred job
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_color_texture; // 0
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_normal_texture; // 1
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_specular_texture; // 2
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_velocity_texture; // 3
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_depth_texture; // 4
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_lighting_texture; // 5
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
			m_lighting_attachment_id = (KCL::uint32)rrd.m_attachments.size() - 1;
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_indirect_light_weight_texture; // 6
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
			rrd.m_attachments.push_back(ad);
			m_lighting_weight_attachment_id = (KCL::uint32)rrd.m_attachments.size() - 1;
		}
		if (COLOR_DEPTH_NEEDED())
		{
			INFO("Create color depth attachment.");
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_color_depth_texture; // 7
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
			rrd.m_attachments.push_back(ad);
			m_gbuffer_color_depth_attachment_id = (KCL::uint32)rrd.m_attachments.size() - 1;
		}

		{
			NGL_subpass sp;
			sp.m_name = "main";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // albedo
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // normal
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // specular
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // velocity
			sp.m_usages.push_back(NGL_DEPTH_ATTACHMENT); // depth
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT); // lighting
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT); // weight
			if (COLOR_DEPTH_NEEDED())
			{
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // color depth
			}
			rrd.m_subpasses.push_back(sp);
		}
		{
			NGL_subpass sp;
			sp.m_name = "indirect lighting";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // albedo
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // normal
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // specular
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT); // velocity
			sp.m_usages.push_back(NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE); // depth
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // lighting
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // weight
			if (COLOR_DEPTH_NEEDED())
			{
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // color depth
			}
			rrd.m_subpasses.push_back(sp);
		}
		{
			NGL_subpass sp;
			sp.m_name = "direct lighting";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // albedo
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // normal
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // specular
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT); // velocity
			sp.m_usages.push_back(NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE); // depth
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT); // lighting
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // weight
			if (COLOR_DEPTH_NEEDED())
			{
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE); // color depth
			}
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_deferred_render = nglGenJob(rrd);

		Transitions::CreateSubpassTransitions(rrd, m_subpass_begin_transitions, m_subpass_end_transitions);

		nglDepthState(m_deferred_render, NGL_DEPTH_LESS, true);
		nglViewportScissor(m_deferred_render, scene_viewport, scene_viewport);
	}

	// Forward job
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_lighting_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_depth_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "forward";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			sp.m_usages.push_back(NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_forward_render = nglGenJob(rrd);

		nglViewportScissor(m_forward_render, scene_viewport, scene_viewport);
	}

	InitSubpassDefines();

	return KCL::KCL_TESTERROR_NOERROR;
}


void Scene5Pipeline::BindGBuffer(const void **p)
{
	p[UNIFORM_GBUFFER_COLOR_TEX] = &m_gbuffer_color_texture;
	p[UNIFORM_GBUFFER_SPECULAR_TEX] = &m_gbuffer_specular_texture;
	p[UNIFORM_GBUFFER_NORMAL_TEX] = &m_gbuffer_normal_texture;
	p[UNIFORM_GBUFFER_VELOCITY_TEX] = &m_gbuffer_velocity_texture;
	p[UNIFORM_GBUFFER_DEPTH_TEX] = (COLOR_DEPTH_NEEDED() && m_deffered_render_run)?&m_gbuffer_color_depth_texture:&m_gbuffer_depth_texture;
}

