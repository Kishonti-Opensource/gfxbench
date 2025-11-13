/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_compute_motion_blur.h"
#include "mtl_scene_40.h"
#include "mtl_quadBuffer.h"
#include <ng/log.h>

#include <math.h>

using namespace MetalRender;

#define NEIGHTBOR_TEXTURE_RGBA8 1

/*
TODO:
- Set reduction kernel size correctly
- Set m_tile_tex_width, m_tile_tex_height correctly
- Downsample with linear filter instead of separate pass
- Init members, like shaders!
*/
ComputeMotionBlur::ComputeMotionBlur(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, KCL::uint32 sample_count, AlgorithmMode mode)
{
	m_device = device;
#if NEIGHTBOR_TEXTURE_RGBA8
	m_neighbor_texture_type = MTLPixelFormatRGBA8Unorm;
    m_neighbor_image_type = MTLPixelFormatRGBA8Unorm;
#else
    m_neighbor_texture_type = GL_RG16F;
    m_neighbor_image_type = GL_R32UI;
#endif

    m_mode = mode;

    m_width = width;
    m_height = height;
	
	uint32_t short_side_length = KCL::Min(width,height);

    m_k = 30; // tile size
    m_sample_count = sample_count;

    // Adjust the values to the current resolution
    m_k = (m_k * short_side_length) / 1080;
    m_k = KCL::Max(m_k, 4u);

    if (mode != Adaptive)
    {
        // Sample count should be rounded to 4 because the shader is vectorized
        float fsamples = (sample_count * short_side_length) / 1080.0f;
        m_sample_count = KCL::uint32(floor(fsamples / 4.0f + 0.5f) * 4.0f);
    }
    else
    {
        // Scale the sample count according to the resoltion
        m_sample_count = KCL::uint32((sample_count * short_side_length) / 1080.0f);
    }
    m_sample_count = KCL::Max(m_sample_count, 4u);

    std::vector<KCL::int32> m_vectorized_samples(m_sample_count);
    for (KCL::int32 i = 0; i < m_sample_count; i++)
    {
        m_vectorized_samples[i] = (i + 2) / 4;
        m_vectorized_samples[i] = KCL::Min(m_vectorized_samples[i], 4);
    }

    m_tile_tex_width = width / m_k;
    m_tile_tex_height = height / m_k;
    m_tile_tex_width = KCL::Max(m_tile_tex_width, 1u);
    m_tile_tex_height = KCL::Max(m_tile_tex_height, 1u);

    m_tile_in_tex_bind = 0;
    m_tile_max_buffer_bind = 1;
    m_tile_neighbor_tex_bind = 2;

    NGLOG_INFO("ComputeMotionBlur: sample count: %s -> %s", sample_count, m_sample_count);

    m_tap_offsets.resize(m_sample_count);
    for (KCL::uint32 i = 0; i < m_sample_count; i++)
    {
        float a = float(i + 1) / float(m_sample_count + 1);
        m_tap_offsets[i] = -1.0f * (1.0f - a) + 1.0f * a;
    }
    
    m_tap_offsets_buffer = [m_device newBufferWithBytes:m_tap_offsets.data() length:m_sample_count * sizeof(float) options:STORAGE_MODE_MANAGED_OR_SHARED];

    m_is_warmup_scene_inited = false;

	// Create the sampler
	MTLSamplerDescriptor * sampler_desc = [[MTLSamplerDescriptor alloc] init];

	sampler_desc.sAddressMode = MTLSamplerAddressModeRepeat;
	sampler_desc.tAddressMode = MTLSamplerAddressModeRepeat;
	sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.mipFilter = MTLSamplerMipFilterNotMipmapped;

	m_linear_sampler = [device newSamplerStateWithDescriptor:sampler_desc];
	releaseObj(sampler_desc);
}

ComputeMotionBlur::~ComputeMotionBlur()
{

}

void ComputeMotionBlur::SetupNeighborTexture()
{
//	m_neighbormax_texture = GFXB4::CreateRenderTarget(m_device, 1, m_tile_tex_width, m_tile_tex_height, MTLPixelFormatRGBA8Unorm);

	MTLTextureDescriptor * tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
																						 width:m_tile_tex_width
																						height:m_tile_tex_height
																					 mipmapped:NO];
	tex_desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
#if !TARGET_OS_EMBEDDED
	tex_desc.storageMode = MTLStorageModePrivate;
#endif

	m_neighbormax_texture = [m_device newTextureWithDescriptor:tex_desc];
}

KCL::KCL_Status ComputeMotionBlur::Init(WarmUpHelper *warm_up)
{
    SetupNeighborTexture();

    const size_t m_tilemax_buffer_size = m_tile_tex_height * m_tile_tex_width * 2 * sizeof(float);

	m_tilemax_buffer = [m_device newBufferWithLength:m_tilemax_buffer_size options:MTLResourceStorageModePrivate];

    // Load the shaders
    KCL::KCL_Status status = LoadFragmentShaders();
    if (status != KCL::KCL_TESTERROR_NOERROR)
    {
        return status;
    }

    if (warm_up)
    {
        WarmUpHelper::WarmUpConfig *cfg = warm_up->GetConfig(WarmUpHelper::MOTION_BLUR_TILE_MAX);
        if (cfg)
        {
            NGLOG_INFO("ComputeMotionBlur: using pre-defined workgroup size: %s for tile_max shader", cfg->m_wg_config.size_x);
            status = LoadTileMaxShader(cfg->m_wg_config.size_x);
        }
        else
        {
            status = WarmupTileMaxShader(warm_up);
        }
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            return status;
        }

        cfg = warm_up->GetConfig(WarmUpHelper::MOTION_BLUR_NEIGHBOR_MAX);
        if (cfg)
        {
            NGLOG_INFO("ComputeMotionBlur: using pre-defined workgroup size: %s for neighbor_max shader", cfg->m_wg_config.size_x);
            status = LoadNeighborMax(cfg->m_wg_config.size_x);
        }
        else
        {
            status = WarmupNeighborMaxShader(warm_up);
        }
    }
    else
    {
        KCL::uint32 tile_max_ws_size = 64;
        KCL::uint32 neighbor_max_ws_size = 8;
        status = LoadTileMaxShader(tile_max_ws_size);
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            return status;
        }

        status = LoadNeighborMax(neighbor_max_ws_size);
    }
    return status;
}

KCL::KCL_Status ComputeMotionBlur::LoadTileMaxShader(KCL::uint32 ws_size)
{
    m_reduction_work_group_size = ws_size;
    m_reduction_dispatch_x = m_tile_tex_width;
    m_reduction_dispatch_y = m_tile_tex_height;

    KCL::uint32 pass_count = (m_k / VELOCITY_BUFFER_DOWNSAMPLE * m_k / VELOCITY_BUFFER_DOWNSAMPLE + m_reduction_work_group_size - 1) / m_reduction_work_group_size;

    NGLOG_INFO("ComputeMotionBlur: pass count: %s", pass_count);

    KCL::KCL_Status error;
    MTLPipeLineBuilder sb;
    SetupBuilder(sb);

    sb.AddDefineInt("IN_TEX_BIND", m_tile_in_tex_bind);
    sb.AddDefineInt("THREAD_COUNT", m_reduction_work_group_size);
    sb.AddDefineInt("PASS_COUNT", pass_count);
	sb.ForceHighp(true);
    sb.AddDefineVec2("STEP_UV", KCL::Vector2D(float(VELOCITY_BUFFER_DOWNSAMPLE) / float(m_width), float(VELOCITY_BUFFER_DOWNSAMPLE) / float(m_height)));

	sb.ShaderFile("mb_tile_max_reduction.cshader");
    m_tile_max_shader = sb.Build(error);

    return error;
}

KCL::KCL_Status ComputeMotionBlur::LoadNeighborMax(KCL::uint32 ws_size)
{
    m_neighbor_max_work_group_size = ws_size;
    m_neighbor_max_dispatch_x = (m_tile_tex_width + m_neighbor_max_work_group_size - 1) / m_neighbor_max_work_group_size;
    m_neighbor_max_dispatch_y = (m_tile_tex_height + m_neighbor_max_work_group_size - 1) / m_neighbor_max_work_group_size;

    KCL::KCL_Status error;
    MTLPipeLineBuilder sb;
    SetupBuilder(sb);
    sb.AddDefineInt("OUT_NEIGHBOR_TEX_BIND", m_tile_neighbor_tex_bind);
    sb.AddDefineInt("WORK_GROUP_SIZE", m_neighbor_max_work_group_size);
	sb.ForceHighp(true);
	sb.ShaderFile("mb_tile_neighbour_max.cshader");

    m_tile_neighbor_max_shader = sb.Build(error);
    return error;
}

KCL::KCL_Status ComputeMotionBlur::LoadFragmentShaders()
{
    KCL::KCL_Status error;
    MTLPipeLineBuilder sb;
    SetupBuilder(sb);
    sb.ShaderFile("pp_motion_blur.shader");
    sb.AddDefineInt("VP_WIDTH", m_width);
    sb.AddDefineInt("SAMPLE_COUNT", m_sample_count);
    sb.AddDefineInt("MODE", int(m_mode));
//	sb.SetType(kShaderTypeNoColorAttachment)
	sb.SetVertexLayout(MetalRender::QuadBuffer::GetVertexLayout());

    m_blur_shader = sb.Build(error);
    if (error != KCL::KCL_TESTERROR_NOERROR)
    {
        return error;
    }

    return error;
}

void ComputeMotionBlur::SetupBuilder(MTLPipeLineBuilder & sb)
{
    sb.AddDefineInt("K", m_k);

    sb.AddDefineInt("MAX_BUFFER_BIND", m_tile_max_buffer_bind);

    sb.AddDefineInt("NEIGHBOR_MAX_TEX_WIDTH", m_tile_tex_width);
    sb.AddDefineInt("NEIGHBOR_MAX_TEX_HEIGHT", m_tile_tex_height);

    sb.AddDefineIVec2("IMAGE_SIZE", m_width, m_height);

#if NEIGHTBOR_TEXTURE_RGBA8
    sb.AddDefine("NEIGHTBOR_TEXTURE_RGBA8");
#endif
}

void ComputeMotionBlur::Execute(id<MTLCommandBuffer> command_buffer, id<MTLTexture> velocity_buffer)
{
	id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
	encoder.label = @"Compute Motion Blur";

	{
		m_tile_max_shader->SetAsCompute(encoder);

		MTLSize threadsPerGroup = { m_reduction_work_group_size, 1, 1 };
		MTLSize numThreadgroups = { m_reduction_dispatch_x, m_reduction_dispatch_y, 1};

		[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
		[encoder setTexture:velocity_buffer atIndex:0];
		[encoder setSamplerState:m_linear_sampler atIndex:0];

		[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
	}

	{
		m_tile_neighbor_max_shader->SetAsCompute(encoder);

		MTLSize threadsPerGroup = { m_neighbor_max_work_group_size, m_neighbor_max_work_group_size, 1 };
		MTLSize numThreadgroups = { m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1};

		[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
		[encoder setTexture:m_neighbormax_texture atIndex:0];

		[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
	}

	[encoder endEncoding];
}

void ComputeMotionBlur::InitSceneForWarmup(WarmUpHelper *warm_up)
{
    if (!m_is_warmup_scene_inited)
    {
        MTL_Scene_40 *scene = warm_up->GetScene();

        // Quite motion blur heavy frame. Animate 40ms to get the velocity buffer correctly
        scene->m_animation_time = 50184;
        scene->Animate();

        scene->m_animation_time = 50224;
        scene->Animate();
		
		// dynamic data pool handled by the render function
		scene->Render();

        m_is_warmup_scene_inited = true;
    }
}

KCL::KCL_Status ComputeMotionBlur::WarmupTileMaxShader(WarmUpHelper *warm_up)
{
    NGLOG_INFO("Warm up TileMax shader...");
    InitSceneForWarmup(warm_up);

	MTL_Scene_40 *scene = warm_up->GetScene();

    double best_time = INT_MAX;
    KCL::uint32 best_wg_size = 0;
    KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256, m_k / VELOCITY_BUFFER_DOWNSAMPLE * m_k / VELOCITY_BUFFER_DOWNSAMPLE };
    for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
    {
        KCL::uint32 wg_size = sizes[i];
        if (wg_size > 256)
        {
            // Limit work group sizes to 256 for shaders wit barriers
            continue;
        }

        if (!warm_up->GetValidator()->Validate(wg_size))
        {
            continue;
        }

        KCL::uint32 mem_size = wg_size * sizeof(KCL::Vector4D);
        if (mem_size > warm_up->GetValidator()->m_max_shared_memory)
        {
            continue;
        }

        KCL::KCL_Status status = LoadTileMaxShader(wg_size);
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            continue;
        }

		if (wg_size > m_tile_max_shader->GetMaxThreadCount())
		{
			continue;
		}

		id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
		id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

		m_tile_max_shader->SetAsCompute(encoder);

		[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
		[encoder setTexture:scene->GetVelocityBuffer() atIndex:0];
		[encoder setSamplerState:m_linear_sampler atIndex:0];

        NGLOG_INFO("Workgroup size: %s", wg_size);

        // First try with 5 iterations
        KCL::uint32 iterations = 5;
        KCL::uint64 dt = 0;
        double avg_time = 0.0;
        warm_up->BeginTimer();
        for (KCL::uint32 j = 0; j < iterations; j++)
        {
			const MTLSize threadsPerGroup = { m_reduction_work_group_size, 1, 1 };
			const MTLSize numThreadgroups = { m_reduction_dispatch_x, m_reduction_dispatch_y, 1};

			[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
		}
		[encoder endEncoding];
		[command_buffer commit];
		[command_buffer waitUntilCompleted];

        dt = warm_up->EndTimer();
        avg_time = double(dt) / double(iterations);

        NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

        if (dt < 300)
        {
			id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
			id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];
			m_tile_max_shader->SetAsCompute(encoder);

			[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
			[encoder setTexture:scene->GetVelocityBuffer() atIndex:0];
			[encoder setSamplerState:m_linear_sampler atIndex:0];

            // Warm up until 500ms but maximalize the max iteration count
            iterations = avg_time > 0.01 ? KCL::uint32(500.0 / avg_time) : 200;

            iterations = KCL::Max(iterations, 5u);
            iterations = KCL::Min(iterations, 200u);

            NGLOG_INFO("  warmup %s iterations...", iterations);
            warm_up->BeginTimer();
            for (KCL::uint32 j = 0; j < iterations; j++)
            {
				const MTLSize threadsPerGroup = { m_reduction_work_group_size, 1, 1 };
				const MTLSize numThreadgroups = { m_reduction_dispatch_x, m_reduction_dispatch_y, 1};

				[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
			}
			[encoder endEncoding];
			[command_buffer commit];
			[command_buffer waitUntilCompleted];

            dt = warm_up->EndTimer();
            avg_time = double(dt) / double(iterations);

            NGLOG_INFO("  result: sum: %sms, avg time: %sms", float(dt), float(avg_time));
        }

        if (avg_time < best_time)
        {
            best_time = avg_time;
            best_wg_size = wg_size;
        }
    }


    NGLOG_INFO("Best result: %s -> %sms (avg)", best_wg_size, float(best_time));
    if (best_wg_size == 0)
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    WarmUpHelper::WarmUpConfig *cfg = new WarmUpHelper::WarmUpConfig();
    cfg->m_wg_config.size_x = best_wg_size;
    warm_up->SetConfig(WarmUpHelper::MOTION_BLUR_TILE_MAX, cfg);

    return LoadTileMaxShader(best_wg_size);
}

KCL::KCL_Status ComputeMotionBlur::WarmupNeighborMaxShader(WarmUpHelper *warm_up)
{
    NGLOG_INFO("Warm up NeighborMax shader...");
    InitSceneForWarmup(warm_up);

	MTL_Scene_40 *scene = warm_up->GetScene();

    double best_time = INT_MAX;
    KCL::uint32 best_wg_size = 0;
    KCL::uint32 sizes[] = { 8, 16, 32, 64 };
    for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
    {
        KCL::uint32 wg_size = sizes[i];
        if (!warm_up->GetValidator()->Validate(wg_size, wg_size))
        {
            continue;
        }

        KCL::KCL_Status status = LoadNeighborMax(wg_size);
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            continue;
        }

		if (wg_size * wg_size > m_tile_neighbor_max_shader->GetMaxThreadCount())
		{
			continue;
		}

		id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
		id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

		m_tile_neighbor_max_shader->SetAsCompute(encoder);

		[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
		[encoder setTexture:m_neighbormax_texture atIndex:0];

        NGLOG_INFO("Workgroup size: %sx%s", wg_size, wg_size);

        // First try with 5 iterations
        KCL::uint32 iterations = 5;
        KCL::uint64 dt = 0;
        double avg_time = 0.0;
        warm_up->BeginTimer();
        for (KCL::uint32 j = 0; j < iterations; j++)
        {
			const MTLSize threadsPerGroup = { m_neighbor_max_work_group_size, m_neighbor_max_work_group_size, 1 };
			const MTLSize numThreadgroups = { m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1};

			[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
		}
		[encoder endEncoding];
		[command_buffer commit];
		[command_buffer waitUntilCompleted];

        dt = warm_up->EndTimer();
        avg_time = double(dt) / double(iterations);

        NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

        if (dt < 50)
        {
			id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
			id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

			m_tile_neighbor_max_shader->SetAsCompute(encoder);
			[encoder setBuffer:m_tilemax_buffer offset:0 atIndex:0];
			[encoder setTexture:m_neighbormax_texture atIndex:0];

            // Warm up until 200ms but maximalize the max iteration count
            iterations = avg_time > 0.01 ? KCL::uint32(200.0 / avg_time) : 200;

            iterations = KCL::Max(iterations, 5u);
            iterations = KCL::Min(iterations, 200u);

            NGLOG_INFO("  warmup %s iterations...", iterations);
            warm_up->BeginTimer();
            for (KCL::uint32 j = 0; j < iterations; j++)
            {
				const MTLSize threadsPerGroup = { m_neighbor_max_work_group_size, m_neighbor_max_work_group_size, 1 };
				const MTLSize numThreadgroups = { m_neighbor_max_dispatch_x, m_neighbor_max_dispatch_y, 1};

				[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
			}
			[encoder endEncoding];
			[command_buffer commit];
			[command_buffer waitUntilCompleted];

            dt = warm_up->EndTimer();
            avg_time = double(dt) / double(iterations);

            NGLOG_INFO("  result: sum: %sms, avg time: %sms", float(dt), float(avg_time));
        }

        if (avg_time < best_time)
        {
            best_time = avg_time;
            best_wg_size = wg_size;
        }
    }
    NGLOG_INFO("Best result: %sx%s -> %sms (avg)", best_wg_size, best_wg_size, float(best_time));
    if (best_wg_size == 0)
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    WarmUpHelper::WarmUpConfig *cfg = new WarmUpHelper::WarmUpConfig();
    cfg->m_wg_config.size_x = best_wg_size;
    warm_up->SetConfig(WarmUpHelper::MOTION_BLUR_NEIGHBOR_MAX, cfg);

    return LoadNeighborMax(best_wg_size);
}
