/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_COMPUTE_MOTION_BLUR_H
#define MTL_COMPUTE_MOTION_BLUR_H

#include <kcl_base.h>
#include "mtl_scene_40_support.h"
#include "metal/mtl_pipeline_builder.h"
#include <Metal/Metal.h>

class WarmUpHelper;
namespace MetalRender
{
class Pipeline;

class ComputeMotionBlur
{
public:
    enum AlgorithmMode
    {
        Adaptive = 0,
        VectorizedAdaptive = 1,
        Vectorized = 2
    };

    static const KCL::uint32 VELOCITY_BUFFER_DOWNSAMPLE = 2;

    KCL::uint32 m_tile_in_tex_bind, m_tile_max_buffer_bind, m_tile_neighbor_tex_bind;

    ComputeMotionBlur(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, KCL::uint32 sample_count, AlgorithmMode mode);
    ~ComputeMotionBlur();

    KCL::KCL_Status Init(WarmUpHelper *warm_up);
	void Execute(id<MTLCommandBuffer> command_buffer, id<MTLTexture> velocity_buffer);

    Pipeline *GetBlurShader() const
    {
        return m_blur_shader;
    }
    id<MTLTexture> GetNeighborMaxTexture() const
    {
        return m_neighbormax_texture;
    }
    id<MTLBuffer> GetTileMaxBuffer() const
    {
        return m_tilemax_buffer;
    }
    id<MTLBuffer> GetTapOffsetsBuffer() const
    {
        return m_tap_offsets_buffer;
    }
    KCL::uint32 GetTileSize() const
    {
        return m_k;
    }
    KCL::uint32 GetSampleCount() const
    {
        return m_sample_count;
    }
	const std::vector<float> & GetTapOffsets() const
	{
		return m_tap_offsets;
	}
    
private:
    AlgorithmMode m_mode;

    KCL::uint32 m_k;

    KCL::uint32 m_sample_count;

    KCL::uint32 m_neighbor_texture_type;
    KCL::uint32 m_neighbor_image_type;

    KCL::uint32 m_width;
    KCL::uint32 m_height;

    KCL::uint32 m_tile_tex_width;
    KCL::uint32 m_tile_tex_height;

    Pipeline * m_tile_max_shader;
    Pipeline * m_tile_neighbor_max_shader;
    Pipeline * m_blur_shader;

    id<MTLSamplerState> m_linear_sampler;

    id<MTLBuffer> m_tilemax_buffer;
    id<MTLBuffer> m_tap_offsets_buffer;
    id<MTLTexture> m_neighbormax_texture;

    KCL::uint32 m_reduction_work_group_size;
    KCL::uint32 m_reduction_dispatch_x;
    KCL::uint32 m_reduction_dispatch_y;

    KCL::uint32 m_neighbor_max_work_group_size;
    KCL::uint32 m_neighbor_max_dispatch_x;
    KCL::uint32 m_neighbor_max_dispatch_y;

    bool m_is_warmup_scene_inited;

    void SetupBuilder(MTLPipeLineBuilder & sb);
    void SetupNeighborTexture();
    void LoadShaders();

    KCL::KCL_Status LoadTileMaxShader(KCL::uint32 ws_size);
    KCL::KCL_Status LoadNeighborMax(KCL::uint32 ws_size);
    KCL::KCL_Status LoadFragmentShaders();

    void InitSceneForWarmup(WarmUpHelper *warm_up);
    KCL::KCL_Status WarmupTileMaxShader(WarmUpHelper *warm_up);
    KCL::KCL_Status WarmupNeighborMaxShader(WarmUpHelper *warm_up);

    std::vector<float> m_tap_offsets;

	id<MTLDevice> m_device;
};

}

#endif // MTL_COMPUTE_MOTION_BLUR_H
