/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_compute_motion_blur.h"
#include <kcl_camera2.h>
#include <kcl_os.h>
#include <ngl.h>

#include "gfxb_barrier.h"
#include "gfxb_shader.h"
#include "gfxb_shapes.h"
#include "gfxb_warmup_helper.h"
#include "gfxb_scene_base.h"

using namespace GFXB;


const std::string ComputeMotionBlur::MB_NEIGHBOR_MAX_WGS_NAME = "mb_neighbor_max";
const std::string ComputeMotionBlur::MB_TILE_MAX_WGS_NAME = "mb_tile_max";


ComputeMotionBlur::ComputeMotionBlur()
{
	m_mode = Adaptive;

	m_shapes = nullptr;

	m_width = 0;
	m_height = 0;

	m_enabled = true;

	m_original_sample_count = 0;
	m_sample_count = 0;
	m_k = 0; // tile size

	m_neighbormax_texture = 0;
	m_neighbormax_texture_width = 0;
	m_neighbormax_texture_height = 0;

	m_tilemax_buffer = 0;

	m_reduction_workgroup_size = 0;
	m_reduction_dispatch_x = 0;
	m_reduction_dispatch_y = 0;

	m_neighbor_max_workgroup_size = 0;
	m_neighbor_max_dispatch_x = 0;
	m_neighbor_max_dispatch_y = 0;

	m_reduction_job = 0;
	m_neighbor_max_job = 0;
	m_blur_job = 0;

	m_reduction_shader = 0;
	m_neighbor_max_shader = 0;
	m_blur_shader = 0;

	m_input_color_texture = 0;
	m_input_velocity_texture = 0;
	m_input_depth_texture = 0;
	m_output_texture = 0;

	// init debug members
#if DEBUG_MB_TILE_MAX_UV
	m_test_texture = 0;
	m_test_texture_clear_job = 0;
#endif
}


ComputeMotionBlur::~ComputeMotionBlur()
{

}


void ComputeMotionBlur::Init(Shapes *shapes, KCL::uint32 width, KCL::uint32 height, KCL::uint32 sample_count, AlgorithmMode mode)
{
	m_shapes = shapes;

	m_original_sample_count = sample_count;

	m_mode = mode;

	Resize(width, height);

	LoadTilemaxComputeShader(8);
	LoadNeighbormaxComputeShader(8);
}


KCL::uint32 ComputeMotionBlur::ExecuteReduction(KCL::uint32 command_buffer)
{
	if (m_enabled)
	{
		// clear tile uv debug texture
#if DEBUG_MB_TILE_MAX_UV
		nglBegin(m_test_texture_clear_job, command_buffer);
		nglEnd(m_test_texture_clear_job);
#endif

		Transitions::Get()
			.BufferBarrier(m_tilemax_buffer, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
			.TextureBarrier(m_input_velocity_texture, NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
			.Execute(command_buffer);

		KCL::uint32 num_workgroups[] = { m_reduction_dispatch_x , m_reduction_dispatch_y, 1, 0 };

		// Dispatch reduction pass
		const void *p[UNIFORM_MAX];
		p[UNIFORM_GBUFFER_VELOCITY_TEX] = &m_input_velocity_texture;
		p[UNIFORM_TILE_MAX_BUFFER] = &m_tilemax_buffer;
		p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_velocity_params.v;
		p[UNIFORM_NUM_WORK_GROUPS] = num_workgroups;
#if DEBUG_MB_TILE_MAX_UV
		p[UNIFORM_MB_TILE_UV_TEST_TEXTURE] = &m_test_texture;
#endif

		nglBegin(m_reduction_job, command_buffer);
		nglDispatch(m_reduction_job, m_reduction_shader, m_reduction_dispatch_x, m_reduction_dispatch_y, 1, p);
		nglEnd(m_reduction_job);

#if DEBUG_MB_TILE_MAX_BUFFER_INFO
		DumpTileMaxBufferInfo();
#endif
	}
	return m_reduction_job;
}


KCL::uint32 ComputeMotionBlur::ExecuteNeighborMax(KCL::uint32 command_buffer)
{
	if (m_enabled)
	{
		Transitions::Get()
			.TextureBarrier(m_neighbormax_texture, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
			.BufferBarrier(m_tilemax_buffer, NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
			.Execute(command_buffer);

		KCL::uint32 num_workgroups[] = { m_neighbor_max_dispatch_x , m_neighbor_max_dispatch_y, 1, 0 };

		// Dispatch neighbor max pass
		const void *p[UNIFORM_MAX];
		p[UNIFORM_NEIGHBOR_MAX_TEXTURE] = &m_neighbormax_texture;
		p[UNIFORM_TILE_MAX_BUFFER] = &m_tilemax_buffer;
		p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_velocity_params.v;
		p[UNIFORM_NUM_WORK_GROUPS] = num_workgroups;

		nglBegin(m_neighbor_max_job, command_buffer);
		nglDispatch(m_neighbor_max_job, m_neighbor_max_shader, m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1, p);
		nglEnd(m_neighbor_max_job);
	}
	return m_neighbor_max_job;
}


KCL::uint32 ComputeMotionBlur::RenderMotionBlur(KCL::uint32 command_buffer, KCL::Camera2 *camera)
{
	Transitions::Get()
		.TextureBarrier(m_output_texture, NGL_COLOR_ATTACHMENT)
		.TextureBarrier(m_input_color_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_input_velocity_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_input_depth_texture, NGL_SHADER_RESOURCE)
		.TextureBarrier(m_neighbormax_texture, NGL_SHADER_RESOURCE)
		.Execute(command_buffer);

	// Dispatch neighbor max pass
	const void *p[UNIFORM_MAX];
	p[UNIFORM_INPUT_TEXTURE] = &m_input_color_texture;
	p[UNIFORM_GBUFFER_VELOCITY_TEX] = &m_input_velocity_texture;
	p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_input_depth_texture;
	p[UNIFORM_NEIGHBOR_MAX_TEXTURE] = &m_neighbormax_texture;
	p[UNIFORM_DEPTH_PARAMETERS] = camera->m_depth_linearize_factors.v;
	p[UNIFORM_TAP_OFFSETS] = m_tap_offsets.data();
	p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_velocity_params.v;

	nglBegin(m_blur_job, command_buffer);
	nglDrawTwoSided(m_blur_job, m_blur_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(m_blur_job);

	return m_blur_job;
}

void ComputeMotionBlur::DeletePipelines()
{
	nglDeletePipelines(m_reduction_job);
	nglDeletePipelines(m_neighbor_max_job);
	nglDeletePipelines(m_blur_job);
}


void ComputeMotionBlur::Resize(KCL::uint32 width, KCL::uint32 height)
{
	m_width = width;
	m_height = height;

	uint32_t short_side_length = KCL::Min(width, height);
	uint32_t long_side_length = KCL::Max(width, height);

	// Adjust the values to the current resolution
	m_k = 30;
	m_k = (m_k * short_side_length) / 1080;
	m_k = KCL::Max(m_k, 4u);

	if (m_mode != Adaptive)
	{
		// Sample count should be rounded to 4 because the shader is vectorized
		float fsamples = (m_original_sample_count * short_side_length) / 1080.0f;
		m_sample_count = KCL::uint32(floor(fsamples / 4.0f + 0.5f) * 4.0f);
	}
	else
	{
		// Scale the sample count according to the resoltion
		m_sample_count = KCL::uint32((m_original_sample_count * short_side_length) / 1080.0f);
	}
	m_sample_count = KCL::Max(m_sample_count, 4u);

	m_tap_offsets.resize(m_sample_count);
	for (KCL::uint32 i = 0; i < m_sample_count; i++)
	{
		float a = float(i + 1) / float(m_sample_count + 1);
		m_tap_offsets[i].x = -1.0f * (1.0f - a) + 1.0f * a;
	}

	// Velocity uniform params
	float max_velocity = float(m_k) / float(long_side_length) * 1.25f;
	m_velocity_params.x = 0.5f / float(long_side_length);
	m_velocity_params.y = max_velocity;
	m_velocity_params.z = float(m_sample_count) / max_velocity;
	m_velocity_params.w = m_enabled ? 1.0f : 0.0f;

	// Neighbor texture size
	m_neighbormax_texture_width = KCL::Max(m_width / m_k, 1u);
	m_neighbormax_texture_height = KCL::Max(m_height / m_k, 1u);

	// the correct values would be
	// m_neighbormax_texture_width = KCL::Max((m_width - 1) / m_k + 1, 1u);
	// m_neighbormax_texture_height = KCL::Max((m_height - 1) / m_k + 1, 1u);

	// Neighbor texture
	if (m_neighbormax_texture == 0)
	{
		NGL_texture_descriptor desc;
		desc.m_name = "motion blur: neighbormax texture";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;
		desc.m_size[0] = m_neighbormax_texture_width;
		desc.m_size[1] = m_neighbormax_texture_height;
		desc.m_format = NGL_R8_G8_B8_A8_UNORM;
		desc.m_unordered_access = true;
		desc.SetAllClearValue(0.0f);
		nglGenTexture(m_neighbormax_texture, desc, nullptr);
		Transitions::Get().Register(m_neighbormax_texture, desc);
	}
	else
	{
		KCL::uint32 texture_size[3] = { m_neighbormax_texture_width, m_neighbormax_texture_height, 0 };
		nglResizeTextures(1, &m_neighbormax_texture, texture_size);
	}

	// Output texture
	if (m_output_texture == 0)
	{
		NGL_texture_descriptor desc;
		desc.m_name = "motion blur: output texture";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_LINEAR;
		desc.m_wrap_mode = NGL_CLAMP_ALL;
		desc.m_size[0] = m_width;
		desc.m_size[1] = m_height;
		desc.m_format = NGL_R8_G8_B8_UNORM;
		desc.m_is_renderable = true;
		desc.SetAllClearValue(0.0f);
		nglGenTexture(m_output_texture, desc, nullptr);
		Transitions::Get().Register(m_output_texture, desc);
	}
	else
	{
		KCL::uint32 texture_size[3] = { m_width, m_height, 0 };
		nglResizeTextures(1, &m_output_texture, texture_size);
	}

	// Tile max SSBO
	{
		const bool need_to_register = m_tilemax_buffer == 0;

		NGL_vertex_descriptor vl;
		vl.m_stride = sizeof(KCL::Vector2D);
		vl.m_unordered_access = true;
		nglGenVertexBuffer(m_tilemax_buffer, vl, m_neighbormax_texture_width * m_neighbormax_texture_height, nullptr);

		if (need_to_register)
		{
			Transitions::Get().Register(m_tilemax_buffer, vl);
		}
	}

	// Tile max compute job
	if (m_reduction_job == 0)
	{
		NGL_job_descriptor cd;
		{
			NGL_subpass sp;
			sp.m_name = "motion blur::reduction";
			cd.m_subpasses.push_back(sp);
		}
		cd.m_load_shader_callback = LoadShader;
		cd.m_is_compute = true;
		m_reduction_job = nglGenJob(cd);
	}

	// Neighbor max compute job
	if (m_neighbor_max_job == 0)
	{
		NGL_job_descriptor cd;
		{
			NGL_subpass sp;
			sp.m_name = "motion blur::neighbor max";
			cd.m_subpasses.push_back(sp);
		}
		cd.m_load_shader_callback = LoadShader;
		cd.m_is_compute = true;
		m_neighbor_max_job = nglGenJob(cd);
	}

	// Blur shader
	{
		ShaderDescriptor shader_desc;
		shader_desc.AddDefineInt("VP_WIDTH", m_width);
		shader_desc.AddDefineInt("SAMPLE_COUNT", m_sample_count);
		shader_desc.AddDefineInt("MODE", KCL::int32(m_mode));
		shader_desc.AddHeaderFile("velocity.h");
		shader_desc.SetVSFile("pp_motion_blur.vert").SetFSFile("pp_motion_blur.frag");
		m_blur_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
	}

	// Blur job
	if (m_blur_job == 0)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_output_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "motion blur::blur";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_blur_job = nglGenJob(rrd);

		nglDepthState(m_blur_job, NGL_DEPTH_DISABLED, false);
	}

	// Resize blur job
	int32_t viewport[4] =
	{
		0, 0, (int32_t)m_width, (int32_t)m_height
	};
	nglViewportScissor(m_blur_job, viewport, viewport);

#if DEBUG_MB_TILE_MAX_UV
	ResizeTileUVDebug();
#endif

	// Reload the shaders
	DeletePipelines();
}


void ComputeMotionBlur::LoadTilemaxComputeShader(KCL::uint32 wg_size)
{
	nglDeletePipelines(m_reduction_job);

	m_reduction_dispatch_x = m_neighbormax_texture_width;
	m_reduction_dispatch_y = m_neighbormax_texture_height;
	m_reduction_workgroup_size = wg_size;

	KCL::uint32 k_per_ds = KCL::Max((m_k - 1) / VELOCITY_BUFFER_DOWNSAMPLE + 1, 1u);

	KCL::uint32 sample_count = k_per_ds * k_per_ds;
	KCL::uint32 pass_count = (sample_count + wg_size - 1) / wg_size;

	ShaderDescriptor shader_desc;
	shader_desc.AddDefineUInt("K", m_k);
	shader_desc.AddDefineUInt("PASS_COUNT", pass_count);
	shader_desc.AddDefineUInt("GBUFFER_WIDTH", m_width);
	shader_desc.AddDefineUInt("GBUFFER_HEIGHT", m_height);
	shader_desc.AddDefineVec2("STEP_UV", KCL::Vector2D(float(VELOCITY_BUFFER_DOWNSAMPLE) / float(m_width), float(VELOCITY_BUFFER_DOWNSAMPLE) / float(m_height)));;

	shader_desc.AddDefineUInt("VELOCITY_DOWNSAMPLE", VELOCITY_BUFFER_DOWNSAMPLE);
	shader_desc.AddDefineUInt("K_PER_DS", k_per_ds);
	shader_desc.AddDefineUInt("SAMPLE_COUNT", sample_count);
#if DEBUG_MB_TILE_MAX_UV
	shader_desc.AddDefineUInt("DEBUG_MB_TILE_MAX_UV", m_k);
#endif

	shader_desc.SetWorkgroupSize(wg_size, 1, 1);
	shader_desc.AddHeaderFile("velocity.h");
	shader_desc.SetCSFile("mb_tile_max_reduction.comp");
	m_reduction_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
}


void ComputeMotionBlur::WarmupTilemaxCompute(GFXB::WarmupHelper* warmup_helper, KCL::uint32 command_buffer)
{
	// early out if workgroup size specified manually
	if (warmup_helper->At(MB_TILE_MAX_WGS_NAME).x != 0)
	{
		INFO("motion blur tilemax workgroup size manually set to %d", warmup_helper->At(MB_TILE_MAX_WGS_NAME).x);
		LoadTilemaxComputeShader(warmup_helper->At(MB_TILE_MAX_WGS_NAME).x);
		return;
	}

	INFO("Warm up TileMax shader...");

	// execute the transitions
	nglBeginCommandBuffer(command_buffer);
	Transitions::Get()
		.BufferBarrier(m_tilemax_buffer, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.TextureBarrier(m_input_velocity_texture, NGL_SHADER_RESOURCE)
		.Execute(command_buffer);
	nglEndCommandBuffer(command_buffer);
	nglSubmitCommandBuffer(command_buffer);

	// setup tile max pass
	KCL::uint32 num_workgroups[] = { m_reduction_dispatch_x , m_reduction_dispatch_y, 1, 0 };
	const void *p[UNIFORM_MAX];
	p[UNIFORM_GBUFFER_VELOCITY_TEX] = &m_input_velocity_texture;
	p[UNIFORM_TILE_MAX_BUFFER] = &m_tilemax_buffer;
	p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_velocity_params.v;
	p[UNIFORM_NUM_WORK_GROUPS] = num_workgroups;

#if DEBUG_MB_TILE_MAX_UV
	p[UNIFORM_MB_TILE_UV_TEST_TEXTURE] = &m_test_texture;
#endif

	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256 };
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		KCL::uint32 wg_size = sizes[i];
		if (!warmup_helper->ValidateWorkGroupSize(wg_size))
		{
			continue;
		}

		LoadTilemaxComputeShader(wg_size);

		INFO("Workgroup size: %d", wg_size);

		// compile the shader
		{
			nglBeginCommandBuffer(command_buffer);
			nglBegin(m_reduction_job, command_buffer);
			bool s = nglDispatch(m_reduction_job, m_reduction_shader, m_reduction_dispatch_x, m_reduction_dispatch_y, 1, p);
			nglEnd(m_reduction_job);
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
			nglBegin(m_reduction_job, command_buffer);
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				nglDispatch(m_reduction_job, m_reduction_shader, m_reduction_dispatch_x, m_reduction_dispatch_y, 1, p);
			}
			nglEnd(m_reduction_job);
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
				nglBegin(m_reduction_job, command_buffer);
				for (KCL::uint32 j = 0; j < iterations; j++)
				{
					nglDispatch(m_reduction_job, m_reduction_shader, m_reduction_dispatch_x, m_reduction_dispatch_y, 1, p);
				}
				nglEnd(m_reduction_job);
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

	// create the shaderdescriptor with the best workgroup size
	{
		SetTilemaxComputeWorkgroupSize(warmup_helper, best_wg_size);

		nglBeginCommandBuffer(command_buffer);
		nglBegin(m_reduction_job, command_buffer);
		nglDispatch(m_reduction_job, m_reduction_shader, m_reduction_dispatch_x, m_reduction_dispatch_y, 1, p);
		nglEnd(m_reduction_job);
		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);
	}
}


void ComputeMotionBlur::LoadNeighbormaxComputeShader(KCL::uint32 wg_size)
{
	nglDeletePipelines(m_neighbor_max_job);

	m_neighbor_max_workgroup_size = wg_size;
	m_neighbor_max_dispatch_x = (m_neighbormax_texture_width + wg_size - 1) / wg_size;
	m_neighbor_max_dispatch_y = (m_neighbormax_texture_height + wg_size - 1) / wg_size;

	ShaderDescriptor shader_desc;
	shader_desc.AddDefineInt("NEIGHBOR_MAX_TEX_WIDTH", m_neighbormax_texture_width);
	shader_desc.AddDefineInt("NEIGHBOR_MAX_TEX_HEIGHT", m_neighbormax_texture_height);
	shader_desc.SetWorkgroupSize(wg_size, wg_size, 1);
	shader_desc.AddHeaderFile("velocity.h");
	shader_desc.SetCSFile("mb_tile_neighbor_max.comp");
	m_neighbor_max_shader = ShaderFactory::GetInstance()->AddDescriptor(shader_desc);
}


void ComputeMotionBlur::WarmupNeighbormaxCompute(GFXB::WarmupHelper* warmup_helper, KCL::uint32 command_buffer)
{
	// early out if workgroup size specified manually
	if (warmup_helper->At(MB_NEIGHBOR_MAX_WGS_NAME).x != 0)
	{
		INFO("motion blur neighbor max workgroup size manually set to %d", warmup_helper->At(MB_NEIGHBOR_MAX_WGS_NAME).x);
		LoadNeighbormaxComputeShader(warmup_helper->At(MB_NEIGHBOR_MAX_WGS_NAME).x);
		return;
	}

	INFO("\nWarm up NeighborMax shader...\n");

	// execute the transitions
	nglBeginCommandBuffer(command_buffer);
	Transitions::Get()
		.TextureBarrier(m_neighbormax_texture, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
		.BufferBarrier(m_tilemax_buffer, NGL_SHADER_RESOURCE)
		.Execute(command_buffer);
	nglEndCommandBuffer(command_buffer);
	nglSubmitCommandBuffer(command_buffer);

	// setup neighbor max pass
	KCL::uint32 num_workgroups[] = { m_neighbor_max_dispatch_x , m_neighbor_max_dispatch_y, 1, 0 };
	const void *p[UNIFORM_MAX];
	p[UNIFORM_NEIGHBOR_MAX_TEXTURE] = &m_neighbormax_texture;
	p[UNIFORM_TILE_MAX_BUFFER] = &m_tilemax_buffer;
	p[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = m_velocity_params.v;
	p[UNIFORM_NUM_WORK_GROUPS] = num_workgroups;

	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64 };
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		KCL::uint32 wg_size = sizes[i];
		if (!warmup_helper->ValidateWorkGroupSize(wg_size, wg_size))
		{
			continue;
		}

		LoadNeighbormaxComputeShader(wg_size);

		INFO("Workgroup size: %dx%d", wg_size, wg_size);

		// compile the shader
		{
			nglBeginCommandBuffer(command_buffer);
			nglBegin(m_neighbor_max_job, command_buffer);
			bool s = nglDispatch(m_neighbor_max_job, m_neighbor_max_shader, m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1, p);
			nglEnd(m_neighbor_max_job);
			nglEndCommandBuffer(command_buffer);
			nglSubmitCommandBuffer(command_buffer);

			if (!s)
			{
				INFO("Invalid workgroup size: %dx%d\n", wg_size, wg_size);
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
			nglBegin(m_neighbor_max_job, command_buffer);
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				nglDispatch(m_neighbor_max_job, m_neighbor_max_shader, m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1, p);
			}
			nglEnd(m_neighbor_max_job);
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
				nglBegin(m_neighbor_max_job, command_buffer);
				for (KCL::uint32 j = 0; j < iterations; j++)
				{
					nglDispatch(m_neighbor_max_job, m_neighbor_max_shader, m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1, p);
				}
				nglEnd(m_neighbor_max_job);
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
	INFO("Best result: %dx%d -> %fms (avg)", best_wg_size, best_wg_size, float(best_time));

	// create the shaderdescriptor with the best workgroup size
	{
		SetNeighbormaxComputeWorkgroupSize(warmup_helper, best_wg_size);

		nglBeginCommandBuffer(command_buffer);
		nglBegin(m_neighbor_max_job, command_buffer);
		nglDispatch(m_neighbor_max_job, m_neighbor_max_shader, m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1, p);
		nglEnd(m_neighbor_max_job);
		nglEndCommandBuffer(command_buffer);
		nglSubmitCommandBuffer(command_buffer);
	}
}


void ComputeMotionBlur::InitSceneForWarmup(GFXB::SceneBase* scene, KCL::uint32 time)
{
	// Quite motion blur heavy frame. Animate 40ms to get the velocity buffer correctly
	scene->m_animation_time = time;
	scene->Animate();
	scene->RenderAndClose();
	nglFinish();
}


void ComputeMotionBlur::SetTilemaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper, KCL::uint32 wg_size)
{
	warmup_helper->At(MB_TILE_MAX_WGS_NAME).x = wg_size;
	LoadTilemaxComputeShader(wg_size);
}


void ComputeMotionBlur::SetNeighbormaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper, KCL::uint32 wg_size)
{
	warmup_helper->At(MB_NEIGHBOR_MAX_WGS_NAME).x = wg_size;
	LoadNeighbormaxComputeShader(wg_size);
}


KCL::uint32 ComputeMotionBlur::GetTilemaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper) const
{
	return warmup_helper->At(MB_TILE_MAX_WGS_NAME).x;
}


KCL::uint32 ComputeMotionBlur::GetNeighbormaxComputeWorkgroupSize(GFXB::WarmupHelper* warmup_helper) const
{
	return warmup_helper->At(MB_NEIGHBOR_MAX_WGS_NAME).x;
}


void ComputeMotionBlur::SetInputTextures(KCL::uint32 color_texture, KCL::uint32 velocity_texture, KCL::uint32 depth_texture)
{
	m_input_color_texture = color_texture;
	m_input_velocity_texture = velocity_texture;
	m_input_depth_texture = depth_texture;
}


KCL::uint32 &ComputeMotionBlur::GetOutputTexture()
{
	return m_output_texture;
}


KCL::uint32 ComputeMotionBlur::GetReductionJob() const
{
	return m_reduction_job;
}


KCL::uint32 ComputeMotionBlur::GetNeighborMaxJob() const
{
	return m_neighbor_max_job;
}


KCL::uint32 ComputeMotionBlur::GetBlurJob() const
{
	return m_blur_job;
}


KCL::uint32 &ComputeMotionBlur::GetNeighborMaxTexture()
{
	return m_neighbormax_texture;
}


KCL::Vector4D &ComputeMotionBlur::GetVelocityParams()
{
	return m_velocity_params;
}


void ComputeMotionBlur::SetEnabled(bool enabled)
{
	m_enabled = enabled;
	m_velocity_params.w = m_enabled ? 1.0f : 0.0f;
}


bool ComputeMotionBlur::IsEnabled() const
{
	return m_enabled;
}


#if DEBUG_MB_TILE_MAX_UV
void ComputeMotionBlur::ResizeTileUVDebug()
{
	// create test texture
	if (m_test_texture == 0)
	{
		NGL_texture_descriptor desc;
		desc.m_name = "motion blur: test texture";
		desc.m_type = NGL_TEXTURE_2D;
		desc.m_filter = NGL_NEAREST;
		desc.m_wrap_mode = NGL_CLAMP_ALL;
		desc.m_size[0] = m_width;
		desc.m_size[1] = m_height;
		desc.m_format = NGL_R8_G8_B8_A8_UNORM;
		desc.m_unordered_access = true;
		desc.m_is_renderable = true;
		desc.SetAllClearValue(0.0f);
		nglGenTexture(m_test_texture, desc, nullptr);
		Transitions::Get().Register(m_test_texture, desc);
	}

	if (m_test_texture_clear_job == 0)
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_test_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}

		{
			NGL_subpass sp;
			sp.m_name = "motion blur::test clear";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_test_texture_clear_job = nglGenJob(rrd);

		nglDepthState(m_test_texture_clear_job, NGL_DEPTH_DISABLED, false);
	}
	else
	{
		KCL::uint32 texture_size[3] = { m_width, m_height, 0 };
		nglResizeTextures(1, &m_test_texture, texture_size);
	}

	int32_t viewport_clear[4] =
	{
		0, 0, (int32_t)m_width, (int32_t)m_height
	};
	nglViewportScissor(m_test_texture_clear_job, viewport_clear, viewport_clear);
}
#endif // DEBUG_MB_TILE_MAX_UV


#if DEBUG_MB_TILE_MAX_BUFFER_INFO
void ComputeMotionBlur::DumpTileMaxBufferInfo()
{
	static int tick = 0;
	const int dump_window = 100;
	tick++;

	if (tick % dump_window == 0)
	{
		std::vector<uint8_t> buffer;
		nglGetVertexBufferContent(m_tilemax_buffer, NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE, buffer);

		const KCL::uint32 buffer_size = m_neighbormax_texture_width * m_neighbormax_texture_height;

		double sum_x = 0.0;
		double sum_y = 0.0;
		float max_x = -100000.0f;
		float min_x = 100000.0f;
		float max_y = -100000.0f;
		float min_y = 100000.0f;

		if (buffer.size() == sizeof(KCL::Vector2D) * buffer_size)
		{
			const KCL::Vector2D *ptr = (KCL::Vector2D*)buffer.data();

			for (KCL::uint32 i = 0; i < buffer_size; i++)
			{
				sum_x += (double)ptr[i].x;
				sum_y += (double)ptr[i].y;

				max_x = KCL::Max(max_x, ptr[i].x);
				min_x = KCL::Min(min_x, ptr[i].x);

				max_y = KCL::Max(max_y, ptr[i].y);
				min_y = KCL::Min(min_y, ptr[i].y);
			}

			printf("\nbuffer info: avg: ( %f, %f )\t\tmin: ( %f, %f )\t\t max: ( %f, %f )\n\n", sum_x / buffer_size, sum_y / buffer_size, min_x, min_y, max_x, max_y);
		}
		else
		{
			printf("invalid buffer size!\n");
		}
	}
}
#endif // DEBUG_MB_TILE_MAX_BUFFER_INFO

