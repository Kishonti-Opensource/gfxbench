/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gi.h"
#include "kcl_light2.h"
#include "kcl_mesh.h"
#include "kcl_camera2.h"
#include "kcl_aabb.h"
#include "kcl_envprobe.h"
#include "gfxb_barrier.h"
#include "common/gfxb_mesh_shape.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_light.h"
#include "common/gfxb_shadow_map.h"


using namespace GFXB;

#define nglBindUniform(L,P) p[L]=(void*)P

const char* GI2::GI_COMPUTE_SH_WGS_NAME = "gi_gen_sh";

void GI2::DebugRender(uint32_t command_buffer, int32_t viewport[4], KCL::Camera2 *c, uint32_t sphere_vbid, uint32_t sphere_ibid)
{
	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_color_texture, NGL_COLOR_ATTACHMENT)
		.TextureBarrier(m_depth_texture, NGL_READ_ONLY_DEPTH_ATTACHMENT)
		.TextureBarrier(m_envprobe_envmap_atlas, NGL_SHADER_RESOURCE);

	if (m_use_texture_sh_atlas)
	{
		transitions.TextureBarrier(m_envprobe_sh_atlas_texture, NGL_SHADER_RESOURCE);
	}
	else
	{
		transitions.BufferBarrier(m_envprobe_sh_atlas, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS);
	}

	transitions.Execute(command_buffer);

	float envprobe_index;
	KCL::Matrix4x4 mvp;
	const void *p[UNIFORM_MAX];
	NGL_buffer_subresource envprobe_sh_atlas_bsr(m_envprobe_sh_atlas);

	nglBindUniform(UNIFORM_MVP, mvp.v);
	nglBindUniform(UNIFORM_envprobe_index, &envprobe_index);

	if (m_use_texture_sh_atlas)
	{
		nglBindUniform(UNIFORM_envprobe_sh_atlas_texture, &m_envprobe_sh_atlas_texture);
	}
	else
	{
		nglBindUniform(UNIFORM_envprobe_sh_atlas, &envprobe_sh_atlas_bsr);
	}

	nglBindUniform(UNIFORM_envprobe_envmap_atlas, &m_envprobe_envmap_atlas);

	nglViewportScissor(m_envprobe_visualize.m_job, viewport, viewport);

	nglBegin(m_envprobe_visualize.m_job, command_buffer);

	for (size_t i = 0; i < m_envprobes.size(); i++)
	{
		KCL::Matrix4x4 pom;
		KCL::EnvProbe *ep = m_envprobes[i];

		pom.identity();
		pom.translate(ep->m_sampling_pos);
		pom.scale(KCL::Vector3D(0.25f, 0.25f, 0.25f));

		mvp = pom * c->GetViewProjection();

		envprobe_index = (float)i;

		envprobe_sh_atlas_bsr.m_offset = sizeof(KCL::Vector4D) * 16 * (uint32_t)i;
		envprobe_sh_atlas_bsr.m_size = sizeof(KCL::Vector4D) * 9;

		nglDrawFrontSided(m_envprobe_visualize.m_job, m_envprobe_visualize.m_shader, sphere_vbid, sphere_ibid, p);
	}

	nglEnd(m_envprobe_visualize.m_job);
}


void GI2::Init(uint32_t max_num_envprobes, uint32_t color_texture, uint32_t depth_texture, bool compute_generate_sh, bool use_texture_sh_atlas)
{
	m_max_num_envprobes = max_num_envprobes;

	m_active_envprobe_ids.resize(m_max_num_envprobes * 4, 0);

	m_render_shadows = true;

	m_color_texture = color_texture;
	m_depth_texture = depth_texture;
	m_compute_generate_sh = compute_generate_sh;
	m_use_texture_sh_atlas = use_texture_sh_atlas;

	m_lightmap_viewport[0] = 0;
	m_lightmap_viewport[1] = 0;
	m_lightmap_viewport[2] = 1024;
	m_lightmap_viewport[3] = 1024;
	m_envprobe_size = 16;
	m_envprobe_atlas_viewport[0] = 0;
	m_envprobe_atlas_viewport[1] = 0;
	m_envprobe_atlas_viewport[2] = m_envprobe_size * 6;
	m_envprobe_atlas_viewport[3] = m_envprobe_size * m_max_num_envprobes;

	//lightmap texture
	{
		NGL_texture_descriptor desc;
		desc.m_name = "gi_lightmap";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_LINEAR;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = m_lightmap_viewport[2];
		desc.m_size[1] = m_lightmap_viewport[3];
		desc.m_format = NGL_R16_G16_B16_A16_FLOAT;
		desc.m_is_renderable = true;

		m_lightmap = 0;
		nglGenTexture(m_lightmap, desc, nullptr);

		Transitions::Get().Register(m_lightmap, desc);
	}
	//SH atlas texture
	if (m_use_texture_sh_atlas)
	{
		NGL_texture_descriptor desc;
		desc.m_name = "gi_sh_atlas";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;
		desc.m_unordered_access = m_compute_generate_sh;

		desc.m_size[0] = 9;
		desc.m_size[1] = m_max_num_envprobes;
		desc.m_format = NGL_R16_G16_B16_A16_FLOAT;
		desc.m_is_renderable = !m_compute_generate_sh;

		m_envprobe_sh_atlas_texture = 0;
		nglGenTexture(m_envprobe_sh_atlas_texture, desc, nullptr);

		Transitions::Get().Register(m_envprobe_sh_atlas_texture, desc);
	}
	else
	{
		NGL_vertex_descriptor vl;
		vl.m_stride = sizeof(KCL::Vector4D);
		vl.m_unordered_access = true;

		m_envprobe_sh_atlas = 0;
		nglGenVertexBuffer(m_envprobe_sh_atlas, vl, 16 * m_max_num_envprobes, nullptr);

		Transitions::Get().Register(m_envprobe_sh_atlas, vl);
	}
	//envmap atlas texture
	{
		NGL_texture_descriptor desc;
		desc.m_name = "gi_envmap_atlas";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = m_envprobe_atlas_viewport[2];
		desc.m_size[1] = m_envprobe_atlas_viewport[3];
		desc.m_format = NGL_R16_G16_B16_A16_FLOAT;
		desc.m_is_renderable = true;

		m_envprobe_envmap_atlas = 0;
		nglGenTexture(m_envprobe_envmap_atlas, desc, nullptr);

		Transitions::Get().Register(m_envprobe_envmap_atlas, desc);
	}
	//indirect uv texture
	{
		NGL_texture_descriptor desc;
		desc.m_name = "gi_indirect_uv";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = m_envprobe_atlas_viewport[2];
		desc.m_size[1] = m_envprobe_atlas_viewport[3];
		desc.m_format = NGL_R8_G8_B8_A8_UNORM;
		desc.m_is_renderable = true;

		m_envprobe_indirect_uv_map = 0;
		nglGenTexture(m_envprobe_indirect_uv_map, desc, nullptr);

		Transitions::Get().Register(m_envprobe_indirect_uv_map, desc);
	}
	//indirect uv depth texture
	{
		NGL_texture_descriptor desc;
		desc.m_name = "gi_indirect_depth";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;

		desc.m_size[0] = m_envprobe_atlas_viewport[2];
		desc.m_size[1] = m_envprobe_atlas_viewport[3];
		desc.m_format = NGL_D24_UNORM;
		desc.m_is_renderable = true;
		desc.m_clear_value[0] = 1.0f;

		m_envprobe_indirect_uv_depth_map = 0;
		nglGenTexture(m_envprobe_indirect_uv_depth_map, desc, nullptr);

		Transitions::Get().Register(m_envprobe_indirect_uv_depth_map, desc);
	}
	//indirect uv clear
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_envprobe_indirect_uv_map;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "gi::m_envprobe_uv_clear";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_envprobe_uv_refresh.m_clear_job = nglGenJob(rrd);
	}
	//indirect uv refresh
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_envprobe_indirect_uv_map;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_envprobe_indirect_uv_depth_map;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "gi::m_envprobe_uv_refresh";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			sp.m_usages.push_back(NGL_DEPTH_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_envprobe_uv_refresh.m_job = nglGenJob(rrd);

		nglDepthState(m_envprobe_uv_refresh.m_job, NGL_DEPTH_LESS, true);

		ShaderDescriptor sd;

		sd.SetVSFile("deferred_irradiance_volumes/m_envprobe_uv_refresh.shader");
		sd.SetFSFile("deferred_irradiance_volumes/m_envprobe_uv_refresh.shader");

		m_envprobe_uv_refresh.m_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
	}
	//envprobe + lightmap transfer
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_envprobe_envmap_atlas;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "gi::m_envprobe_lightmap_atlas_transfer";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_envprobe_lightmap_atlas_transfer.m_job = nglGenJob(rrd);

		ShaderDescriptor sd;

		sd.SetVSFile("deferred_irradiance_volumes/m_envprobe_lightmap_atlas_transfer.shader");
		sd.SetFSFile("deferred_irradiance_volumes/m_envprobe_lightmap_atlas_transfer.shader");
		sd.AddDefineFloat("MAX_NUM_OF_PROBES", float(m_max_num_envprobes));

		m_envprobe_lightmap_atlas_transfer.m_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
	}
	//generate SH
	{
		if (m_compute_generate_sh)
		{
			NGL_job_descriptor cd;
			{
				NGL_subpass sp;
				sp.m_name = "gi::m_envprobe_generate_sh_compute";
				cd.m_subpasses.push_back(sp);
			}

			cd.m_is_compute = true;
			cd.m_load_shader_callback = LoadShader;
			m_envprobe_generate_sh.m_job = nglGenJob(cd);

			LoadGenerateSHShader(8);
		}
		else
		{
			NGL_job_descriptor rrd;
			{
				NGL_attachment_descriptor ad;
				ad.m_attachment.m_idx = m_envprobe_sh_atlas_texture;
				ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
				ad.m_attachment_store_op = NGL_STORE_OP_STORE;
				rrd.m_attachments.push_back(ad);
			}
			{
				NGL_subpass sp;
				sp.m_name = "gi::m_envprobe_generate_sh";
				sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
				rrd.m_subpasses.push_back(sp);
			}

			rrd.m_load_shader_callback = LoadShader;
			m_envprobe_generate_sh.m_job = nglGenJob(rrd);

			ShaderDescriptor sd;

			sd.SetVSFile("deferred_irradiance_volumes/m_envprobe_generate_sh.shader");
			sd.SetFSFile("deferred_irradiance_volumes/m_envprobe_generate_sh.shader");
			sd.AddDefineFloat("MAX_NUM_OF_PROBES", float(m_max_num_envprobes));

			m_envprobe_generate_sh.m_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
		}

		// clear sh atlas, for debug only
#if 0
		{
			NGL_attachment_descriptor ad;
			NGL_job_descriptor jd;

			ad.m_attachment = m_envprobe_sh_atlas_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			jd.m_color_attachments.push_back(ad);

			jd.m_name = "gi_clear_envprobe_sh_atlas";
			jd.m_load_shader_callback = LoadShader;
			jd.m_user_data = this;

			KCL::uint32 clear_sh_atlas_job = nglGenJob(jd);

			nglBegin(clear_sh_atlas_job);
			nglEnd(clear_sh_atlas_job);
			nglSubmit(clear_sh_atlas_job);
		}
#endif
	}
	//direct light eval
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_lightmap;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "gi::m_direct_light_evaluation";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_direct_light_evaluation.m_job = nglGenJob(rrd);

		nglViewportScissor(m_direct_light_evaluation.m_job, m_lightmap_viewport, m_lightmap_viewport);
		nglBlendState0(m_direct_light_evaluation.m_job, NGL_BLEND_ADDITIVE);

		std::string shader_name = "deferred_irradiance_volumes/m_direct_light_evaluation.shader";

		{
			ShaderDescriptor sd;

			sd.SetVSFile(shader_name.c_str());
			sd.SetFSFile(shader_name.c_str());
			sd.AddDefineInt("SHADER_CODE", 1);

			m_direct_light_evaluation.m_shaders[KCL::LightShape::DIRECTIONAL] = ShaderFactory::GetInstance()->AddDescriptor(sd);

			sd.AddDefineInt("HAS_SHADOW", 1);
			m_direct_light_evaluation.m_shadow_shaders[KCL::LightShape::DIRECTIONAL] = ShaderFactory::GetInstance()->AddDescriptor(sd);
		}
		{
			ShaderDescriptor sd;

			sd.SetVSFile(shader_name.c_str());
			sd.SetFSFile(shader_name.c_str());
			sd.AddDefineInt("SHADER_CODE", 0);

			m_direct_light_evaluation.m_shaders[KCL::LightShape::OMNI] = ShaderFactory::GetInstance()->AddDescriptor(sd);

			sd.AddDefineInt("HAS_SHADOW", 1);
			m_direct_light_evaluation.m_shadow_shaders[KCL::LightShape::OMNI] = ShaderFactory::GetInstance()->AddDescriptor(sd);
		}
	}
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_color_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_depth_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "gi::m_envprobe_visualize";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			sp.m_usages.push_back(NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_envprobe_visualize.m_job = nglGenJob(rrd);

		nglDepthState(m_envprobe_visualize.m_job, NGL_DEPTH_LESS, true);

		ShaderDescriptor sd;

		sd.SetVSFile("deferred_irradiance_volumes/m_envprobe_visualize.shader");
		sd.SetFSFile("deferred_irradiance_volumes/m_envprobe_visualize.shader");
		sd.AddDefineFloat("MAX_NUM_OF_PROBES", float(m_max_num_envprobes));
		sd.AddDefineInt("MAX_NUM_OF_PROBES_INT", m_max_num_envprobes);
		if (m_use_texture_sh_atlas)
		{
			sd.AddDefineInt("USE_TEXTURE_SH_ATLAS", 1);
		}

		m_envprobe_visualize.m_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
	}

	// Set uniform defaults
	m_sky_color = KCL::Vector4D(148.0f, 164.0f, 192.0f, 0.0f) / 255.0f * 0.001f;
	m_indirect_lighting_factor = 1.0;
}


void GI2::LoadGenerateSHShader(uint32_t wg_size)
{
	ShaderDescriptor sd;
	nglDeletePipelines(m_envprobe_generate_sh.m_job);

	sd.SetCSFile("deferred_irradiance_volumes/m_envprobe_generate_sh_compute.shader");
	sd.SetWorkgroupSize(wg_size, 1, 1);
	sd.AddDefineFloat("MAX_NUM_OF_PROBES", float(m_max_num_envprobes));
	sd.AddDefineInt("MAX_NUM_OF_PROBES_INT", m_max_num_envprobes);
	if (m_use_texture_sh_atlas)
	{
		sd.AddDefineInt("USE_TEXTURE_SH_ATLAS", 1);
	}

	m_envprobe_generate_sh.m_shader = ShaderFactory::GetInstance()->AddDescriptor(sd);
}


void GI2::WarmupGenerateSH(WarmupHelper* warmup_helper, KCL::uint32 command_buffer)
{
	if (!m_compute_generate_sh) return;

	// early out if workgroup size specified manually
	if (warmup_helper->At(GI_COMPUTE_SH_WGS_NAME).x != 0)
	{
		m_envprobe_generate_sh.m_workgroup_size_x = warmup_helper->At(GI_COMPUTE_SH_WGS_NAME).x;
		INFO("generate sh workgroup size manually set to %d", warmup_helper->At(GI_COMPUTE_SH_WGS_NAME).x);
		LoadGenerateSHShader(m_envprobe_generate_sh.m_workgroup_size_x);
		return;
	}

	nglBeginCommandBuffer(command_buffer);

	Transitions &transitions = Transitions::Get()
		.TextureBarrier(m_envprobe_envmap_atlas, NGL_SHADER_RESOURCE);

	if (m_use_texture_sh_atlas)
	{
		transitions.TextureBarrier(m_envprobe_sh_atlas_texture, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS);
	}
	else
	{
		transitions.BufferBarrier(m_envprobe_sh_atlas, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS);
	}

	transitions.Execute(command_buffer);

	nglEndCommandBuffer(command_buffer);
	nglSubmitCommandBuffer(command_buffer);

	INFO("\nWarm up Generate SH shader...\n");


	// setup generate sh pass
	const void* p[UNIFORM_MAX];
	NGL_buffer_subresource envprobe_sh_atlas_bsr(m_envprobe_sh_atlas);

	nglBindUniform(UNIFORM_envprobe_envmap_atlas, &m_envprobe_envmap_atlas);

	if (m_use_texture_sh_atlas)
	{
		nglBindUniform(UNIFORM_envprobe_sh_atlas_texture, &m_envprobe_sh_atlas_texture);
	}
	else
	{
		nglBindUniform(UNIFORM_envprobe_sh_atlas, &envprobe_sh_atlas_bsr);
	}

	std::vector<uint32_t> warmup_active_envprobe_ids(4*m_max_num_envprobes);

	KCL::uint32 visible_probe_count = m_max_num_envprobes / 2;
	for (uint32_t i = 0; i < visible_probe_count; i++)
	{
		warmup_active_envprobe_ids[4 * i] = 2*i;
	}
	nglBindUniform(UNIFORM_envprobe_index, warmup_active_envprobe_ids.data());

	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256 };  // max 256 thread in workgroup
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		KCL::uint32 wg_size = sizes[i];
		if (!warmup_helper->ValidateWorkGroupSize(wg_size))
		{
			continue;
		}

		LoadGenerateSHShader(wg_size);

		INFO("Workgroup size: %d", wg_size);

		// compile the shader
		{
			nglBeginCommandBuffer(command_buffer);
			nglBegin(m_envprobe_generate_sh.m_job, command_buffer);
			bool s = nglDispatch(m_envprobe_generate_sh.m_job, m_envprobe_generate_sh.m_shader, 1, visible_probe_count, 1, p);
			nglEnd(m_envprobe_generate_sh.m_job);
			nglEndCommandBuffer(command_buffer);
			nglSubmitCommandBuffer(command_buffer);

			if (!s)
			{
				INFO("Invalid workgroup size: %d\n", wg_size);
				continue;
			}
		}

		// First try with 5 iterations
		KCL::uint32 iterations = 5;
		KCL::uint64 dt = 0;
		double avg_time = 0.0;
		warmup_helper->BeginTimer();

		{
			nglBeginCommandBuffer(command_buffer);
			nglBegin(m_envprobe_generate_sh.m_job, command_buffer);
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				nglDispatch(m_envprobe_generate_sh.m_job, m_envprobe_generate_sh.m_shader, 1, visible_probe_count, 1, p);
			}
			nglEnd(m_envprobe_generate_sh.m_job);
			nglEndCommandBuffer(command_buffer);
			nglSubmitCommandBuffer(command_buffer);
		}

		dt = warmup_helper->EndTimer();
		avg_time = double(dt) / double(iterations);

		INFO("  result after %d interations: sum: %fms, avg time: %fms", iterations, float(dt), float(avg_time));

		if (dt < 50)
		{
			// Warm up until 200ms but maximalize the max iteration count
			iterations = avg_time > 0.01 ? KCL::uint32(200.0 / avg_time) : 200;

			iterations = KCL::Max(iterations, 5u);
			iterations = KCL::Min(iterations, 200u);

			INFO("  warmup %d iterations...", iterations);
			warmup_helper->BeginTimer();

			{
				nglBeginCommandBuffer(command_buffer);
				nglBegin(m_envprobe_generate_sh.m_job, command_buffer);
				for (KCL::uint32 j = 0; j < iterations; j++)
				{
					nglDispatch(m_envprobe_generate_sh.m_job, m_envprobe_generate_sh.m_shader, 1, visible_probe_count, 1, p);
				}
				nglEnd(m_envprobe_generate_sh.m_job);
				nglEndCommandBuffer(command_buffer);
				nglSubmitCommandBuffer(command_buffer);
			}

			dt = warmup_helper->EndTimer();
			avg_time = double(dt) / double(iterations);

			INFO("  result: sum: %fms, avg time: %fms\n", float(dt), float(avg_time));
		}

		if (avg_time < best_time)
		{
			best_time = avg_time;
			best_wg_size = wg_size;
		}
	}
	INFO("Best result: %d -> %fms (avg)", best_wg_size, float(best_time));

	warmup_helper->At(GI_COMPUTE_SH_WGS_NAME).x = best_wg_size;

	// create the shaderdescriptor with the best workgroup size
	{
		m_envprobe_generate_sh.m_workgroup_size_x = warmup_helper->At(GI_COMPUTE_SH_WGS_NAME).x;
		LoadGenerateSHShader(m_envprobe_generate_sh.m_workgroup_size_x);
	}
}


void GI2::ClearEnvProbes()
{
	m_envprobes.clear();
}


void GI2::AddEnvProbe(KCL::EnvProbe *probe)
{
	m_envprobes.push_back(probe);
}


void GI2::ReloadShaders()
{
	nglDeletePipelines(m_envprobe_visualize.m_job);
	nglDeletePipelines(m_envprobe_generate_sh.m_job);
	nglDeletePipelines(m_direct_light_evaluation.m_job);
	nglDeletePipelines(m_envprobe_lightmap_atlas_transfer.m_job);
	nglDeletePipelines(m_envprobe_uv_refresh.m_job);
}


void GI2::FrustumCull(KCL::Camera2 *c)
{
	/*
	m_in_envprobes.clear();
	m_out_envprobes.clear();
	m_visible_envprobes.clear();

	for (size_t i = 0; i < m_envprobes.size(); i++)
	{
		KCL::Vector3D &envprobe = m_envprobes[i]->m_pos;
		KCL::AABB aabb;

		aabb.SetWithHalfExtentCenter(KCL::Vector3D(5.0f, 5.0f, 5.0f), envprobe);

		if (c->IsVisible(&aabb))
		{
			//NOTE: check with sphere not with an AABB!
			const float envprobe_sphere_radius = 5.0f * 1.7321f;
			float D;

			D = KCL::Vector3D::distance2(c->GetEye(), envprobe);

			if (D < (envprobe_sphere_radius* envprobe_sphere_radius))
			{
				m_in_envprobes.push_back((uint32_t)i);
			}
			else
			{
				m_out_envprobes.push_back((uint32_t)i);
			}

			m_visible_envprobes.push_back((uint32_t)i);
		}
	}

	//printf("visible: %u(%u+%u), all: %u\n", m_visible_envprobes.size(), m_in_envprobes.size(), m_out_envprobes.size(), m_envprobe_positions.size());
	*/
}


void GI2::envprobe_generate_sh(uint32_t command_buffer, uint32_t fullscreen_vbid, uint32_t fullscreen_ibid, std::vector<KCL::EnvProbe*> &visible_probes)
{
	if (m_compute_generate_sh)
	{
		if (visible_probes.size() > 0)
		{
			Transitions &transitions = Transitions::Get()
				.TextureBarrier(m_envprobe_envmap_atlas, NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE);

			if (m_use_texture_sh_atlas)
			{
				transitions.TextureBarrier(m_envprobe_sh_atlas_texture, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE);
			}
			else
			{
				transitions.BufferBarrier(m_envprobe_sh_atlas, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE);
			}

			transitions.Execute(command_buffer);

			const void* p[UNIFORM_MAX];
			NGL_buffer_subresource envprobe_sh_atlas_bsr(m_envprobe_sh_atlas);

			nglBindUniform(UNIFORM_envprobe_envmap_atlas, &m_envprobe_envmap_atlas);

			if (m_use_texture_sh_atlas)
			{
				nglBindUniform(UNIFORM_envprobe_sh_atlas_texture, &m_envprobe_sh_atlas_texture);
			}
			else
			{
				nglBindUniform(UNIFORM_envprobe_sh_atlas, &envprobe_sh_atlas_bsr);
			}

			nglBegin(m_envprobe_generate_sh.m_job, command_buffer);

			for (size_t i = 0; i < visible_probes.size(); i++)
			{
				m_active_envprobe_ids[4 * i] = visible_probes[i]->m_index;
			}

			nglBindUniform(UNIFORM_envprobe_index, m_active_envprobe_ids.data());

			nglDispatch(m_envprobe_generate_sh.m_job, m_envprobe_generate_sh.m_shader, 1, (uint32_t)visible_probes.size(), 1, p);

			nglEnd(m_envprobe_generate_sh.m_job);
		}
	}
	else
	{
		Transitions &transitions = Transitions::Get()
			.TextureBarrier(m_envprobe_envmap_atlas, NGL_SHADER_RESOURCE);

		if (m_use_texture_sh_atlas)
		{
			transitions.TextureBarrier(m_envprobe_sh_atlas_texture, NGL_COLOR_ATTACHMENT);
		}
		else
		{
			transitions.BufferBarrier(m_envprobe_sh_atlas, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS);
		}

		transitions.Execute(command_buffer);

		const void* p[UNIFORM_MAX];

		nglBindUniform(UNIFORM_envprobe_envmap_atlas, &m_envprobe_envmap_atlas);

		nglBegin(m_envprobe_generate_sh.m_job, command_buffer);

		for (size_t i = 0; i < visible_probes.size(); i++)
		{
			uint32_t envprobe_index_ui = visible_probes[i]->m_index;
			int32_t viewport[4] =
			{
				0,
				(int32_t)envprobe_index_ui,
				9,
				1,
			};

			nglViewportScissor(m_envprobe_generate_sh.m_job, viewport, viewport);

			nglDrawTwoSided(m_envprobe_generate_sh.m_job, m_envprobe_generate_sh.m_shader, fullscreen_vbid, fullscreen_ibid, p);
		}

		nglEnd(m_envprobe_generate_sh.m_job);
	}

	if (save_textures && m_use_texture_sh_atlas)
	{
		uint32_t w, h;
		std::vector<uint8_t> data;
		char tmp[512];

		nglGetTextureContent(m_envprobe_sh_atlas_texture, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, Transitions::Get().GetTextureState(m_envprobe_sh_atlas_texture), w, h, data);

		if (w && h)
		{
			sprintf(tmp, "envprobe_generate_sh.tga");

			KCL::Image::saveTga((KCL::File::GetScenePath() + tmp).c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);
		}
	}
}


void GI2::direct_light_evaluation(uint32_t command_buffer)
{
	const void* p[UNIFORM_MAX];

	Transitions &transitions = Transitions::Get().TextureBarrier(m_lightmap, NGL_COLOR_ATTACHMENT);
	for (size_t i = 0; i < m_lights.size(); i++)
	{
		Light* light = (Light*)m_lights[i];
		if (light->GetShadowMap() != nullptr)
		{
			transitions.TextureBarriers(light->GetShadowMap()->GetRenderTargets(), NGL_SHADER_RESOURCE);
		}
	}
	transitions.Execute(command_buffer);

	nglBegin(m_direct_light_evaluation.m_job, command_buffer);

	for (size_t j = 0; j < m_lights.size(); j++)
	{
		Light* light = (Light*)m_lights[j];

		KCL::LightShape* light_shape = light->m_light_shape;

		if (light_shape->m_light_type == KCL::LightShape::SPOT)
		{
			continue;
		}

		if (light->GetShadowMap() != nullptr)
		{
			nglBindUniform(UNIFORM_SHADOW_MATRIX, light->GetShadowMap()->GetUniformShadowMatrix());
			nglBindUniform(UNIFORM_SHADOW_MAP, light->GetShadowMap()->GetUniformShadowTexture());
			nglBindUniform(UNIFORM_SHADOW_LIGHT_POS,light->GetShadowMap()->GetLight()->m_uniform_pos.v);
		}

		nglBindUniform(UNIFORM_LIGHT_POS, light->m_uniform_pos.v);
		nglBindUniform(UNIFORM_LIGHT_DIR, light->m_uniform_dir.v);
		nglBindUniform(UNIFORM_LIGHT_COLOR, light->m_uniform_color.v);
		nglBindUniform(UNIFORM_ATTENUATION_PARAMETERS, light->m_uniform_attenuation_parameters.v);

		bool has_shadow = light_shape->m_is_shadow_caster && m_render_shadows;
		KCL::uint32 shader_code = has_shadow ? m_direct_light_evaluation.m_shadow_shaders[light_shape->m_light_type] : m_direct_light_evaluation.m_shaders[light_shape->m_light_type];

		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			KCL::Mesh *mesh = m_meshes[i];
			Mesh3 *mesh3 = (Mesh3 *)mesh->m_mesh;

			nglBindUniform(UNIFORM_MODEL, mesh->m_world_pom.v);
			nglDrawTwoSided(m_direct_light_evaluation.m_job, shader_code, mesh3->m_vbid, mesh3->m_ibid, p);
		}
	}

	nglEnd(m_direct_light_evaluation.m_job);

	if (save_textures)
	{
		uint32_t w, h;
		std::vector<uint8_t> data;
		char tmp[512];

		nglGetTextureContent(m_lightmap, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, Transitions::Get().GetTextureState(m_lightmap), w, h, data);

		if (w && h)
		{
			sprintf(tmp, "direct_light_evaluation.tga");

			KCL::Image::saveTga((KCL::File::GetScenePath() + tmp).c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);
		}
	}
}


void GI2::envprobe_lightmap_atlas_transfer(uint32_t command_buffer, uint32_t fullscreen_vbid, uint32_t fullscreen_ibid, std::vector<KCL::EnvProbe*> &visible_probes)
{
	Transitions::Get().TextureBarrier(m_envprobe_envmap_atlas, NGL_COLOR_ATTACHMENT)
		.TextureBarrier(m_lightmap, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_envprobe_indirect_uv_map, NGL_SHADER_RESOURCE).Execute(command_buffer);

	const void* p[UNIFORM_MAX];

	nglBindUniform(UNIFORM_direct_lightmap, &m_lightmap);
	nglBindUniform(UNIFORM_envprobe_indirect_uv_map, &m_envprobe_indirect_uv_map);
	nglBindUniform(UNIFORM_SKY_COLOR, m_sky_color.v);

	nglBegin(m_envprobe_lightmap_atlas_transfer.m_job, command_buffer);

	for (size_t i = 0; i < visible_probes.size(); i++)
	{
		uint32_t envprobe_index_ui = visible_probes[i]->m_index;
		int32_t viewport[4] =
		{
			0,
			(int32_t)(envprobe_index_ui * m_envprobe_size),
			(int32_t)(m_envprobe_size * 6),
			(int32_t)m_envprobe_size
		};

		nglViewportScissor(m_envprobe_lightmap_atlas_transfer.m_job, viewport, viewport);

		nglDrawTwoSided(m_envprobe_lightmap_atlas_transfer.m_job, m_envprobe_lightmap_atlas_transfer.m_shader, fullscreen_vbid, fullscreen_ibid, p);
	}

	nglEnd(m_envprobe_lightmap_atlas_transfer.m_job);

	if (save_textures)
	{
		uint32_t w, h;
		std::vector<uint8_t> data;
		char tmp[512];

		nglGetTextureContent(m_envprobe_envmap_atlas, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, Transitions::Get().GetTextureState(m_envprobe_envmap_atlas), w, h, data);

		if (w && h)
		{
			sprintf(tmp, "envprobe_lightmap_atlas_transfer.tga");

			KCL::Image::saveTga((KCL::File::GetScenePath() + tmp).c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);
		}
	}
}


void GI2::envprobe_uv_refresh(uint32_t command_buffer, std::vector<uint32_t> &indices)
{
	KCL::Camera2 c;
	KCL::Matrix4x4 mvp;
	const void* p[UNIFORM_MAX];

	c.Perspective(90.0f, m_envprobe_size, m_envprobe_size, 0.01f, 1024.0f);

	nglBindUniform(UNIFORM_MVP, mvp.v);

	nglBegin(m_envprobe_uv_refresh.m_job, command_buffer);

	for (size_t i = 0; i < indices.size(); i++)
	{
		uint32_t idx = indices[i];

		for (uint32_t dir = 0; dir < 6; dir++)
		{
			int32_t viewport[4];

			viewport[0] = dir * m_envprobe_size;
			viewport[1] = idx * m_envprobe_size;
			viewport[2] = m_envprobe_size;
			viewport[3] = m_envprobe_size;

			c.LookAtOmni(m_envprobes[idx]->m_sampling_pos, dir);

			c.Update();

			nglViewportScissor(m_envprobe_uv_refresh.m_job, viewport, viewport);

			for (size_t i = 0; i < m_meshes.size(); i++)
			{
				KCL::Mesh *mesh = m_meshes[i];
				Mesh3 *mesh3 = (Mesh3 *)mesh->m_mesh;

				mvp = mesh->m_world_pom * c.GetViewProjection();

				NGL_cull_mode cull_mode = (nglGetInteger(NGL_RASTERIZATION_CONTROL_MODE) == NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP) ? NGL_BACK_SIDED : NGL_FRONT_SIDED;

				nglDraw(m_envprobe_uv_refresh.m_job, NGL_TRIANGLES, m_envprobe_uv_refresh.m_shader, 1, &mesh3->m_vbid, mesh3->m_ibid, cull_mode, p);
			}
		}
	}

	nglEnd(m_envprobe_uv_refresh.m_job);

	if (save_textures)
	{
		uint32_t w, h;
		std::vector<uint8_t> data;
		char tmp[512];

		nglGetTextureContent(m_envprobe_indirect_uv_map, 0, 0, 0, NGL_R8_G8_B8_A8_UNORM, Transitions::Get().GetTextureState(m_envprobe_indirect_uv_map), w, h, data);

		if (w && h)
		{
			sprintf(tmp, "envprobe_uv_refresh.tga");

			KCL::Image::saveTga((KCL::File::GetScenePath() + tmp).c_str(), w, h, &data[0], KCL::Image_RGBA8888, false);
		}
	}
}


void GI2::LoadDeferredIrradianceVolumesShader(NGL_job_descriptor &jd, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms)
{
#if 0
	GI2 *This = (GI2 *)jd.m_user_data;
	std::string defines;

	defines = "#version 440\n";

	_io io("data://shaders/glsl/deferred_irradiance_volumes/" + jd.m_name + ".shader");

	if (!io.IsOpened())
	{
		return;
	}

	{
		char tmp[512];

		sprintf(tmp, "#define SHADER_CODE %u\n", shader_code);
		defines += tmp;
	}
	{
		char tmp[512];

		sprintf(tmp, "#define envprobe_atlas_inv_size vec2(%.11f, %.11f)\n", 1.0f / This->m_envprobe_atlas_viewport[2], 1.0f / This->m_envprobe_atlas_viewport[3]);
		defines += tmp;
	}

	ssd[NGL_VERTEX_SHADER].m_source_data = defines + "#define vertex_main\n" + io.Data();
	ssd[NGL_FRAGMENT_SHADER].m_source_data = defines + "#define fragment_main\n" + io.Data();

	ssd[NGL_VERTEX_SHADER].m_info_string = io.Filename();
	ssd[NGL_FRAGMENT_SHADER].m_info_string = io.Filename();

	application_uniforms.resize(SU_MAX);
	application_uniforms[SU_mvp].m_name = "mvp";
	application_uniforms[SU_model].m_name = "model";
	application_uniforms[SU_m_direct_lightmap].m_name = "m_direct_lightmap";
	application_uniforms[SU_m_envprobe_indirect_uv_map].m_name = "m_envprobe_indirect_uv_map";
	application_uniforms[SU_light_pos].m_name = "light_pos";
	application_uniforms[SU_light_color].m_name = "light_color";
	application_uniforms[SU_light_atten].m_name = "light_atten";
	application_uniforms[SU_m_envprobe_envmap_atlas].m_name = "m_envprobe_envmap_atlas";
	application_uniforms[SU_m_envprobe_sh_atlas].m_name = "m_envprobe_sh_atlas";
	application_uniforms[SU_envprobe_index].m_name = "envprobe_index";
	application_uniforms[SU_shadow_matrix].m_name = "shadow_matrix";
	application_uniforms[SU_shadow_texture].m_name = "shadow_texture";
#endif
}
