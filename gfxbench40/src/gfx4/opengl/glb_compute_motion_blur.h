/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_COMPUTE_MOTION_BLUR_H
#define GLB_COMPUTE_MOTION_BLUR_H

#include <kcl_base.h>
#include "glb_scene_opengl4_support.h"
#include "opengl/glb_shader2.h"
#include "opengl/compute_blur.h"
#include "opengl/glb_filter.h"

class WarmUpHelper;
namespace GLB
{

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

    ComputeMotionBlur(KCL::uint32 m_fullscreen_vao, KCL::uint32 m_fullscreen_vbo, KCL::uint32 width, KCL::uint32 height, KCL::uint32 sample_count, AlgorithmMode mode);
    ~ComputeMotionBlur();

    KCL::KCL_Status Init(WarmUpHelper *warm_up);
    void Execute(KCL::uint32 velocity_buffer);

    GLB::GLBShader2 *GetBlurShader() const
    {
        return m_blur_shader;
    }
    KCL::uint32 GetNeighborMaxTexture() const
    {
        return m_neighbormax_texture;
    }
    KCL::uint32 GetTileMaxBuffer() const
    {
        return m_tilemax_buffer;
    }
    KCL::uint32 GetTileSize() const
    {
        return m_k;
    }
    KCL::uint32 GetSampleCount() const
    {
        return m_sample_count;
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

    GLB::GLBShader2 *m_tile_max_shader;
    GLB::GLBShader2 *m_tile_neighbor_max_shader;
    GLB::GLBShader2 *m_blur_shader;

    KCL::uint32 m_linear_sampler;

    KCL::uint32 m_tilemax_buffer;
    KCL::uint32 m_neighbormax_texture;

    KCL::uint32 m_reduction_work_group_size;
    KCL::uint32 m_reduction_dispatch_x;
    KCL::uint32 m_reduction_dispatch_y;

    KCL::uint32 m_neighbor_max_work_group_size;
    KCL::uint32 m_neighbor_max_dispatch_x;
    KCL::uint32 m_neighbor_max_dispatch_y;

    bool m_is_warmup_scene_inited;

    void SetupBuilder(GLB::GLBShaderBuilder & sb);
    void SetupNeighborTexture();
    void LoadShaders();

    KCL::KCL_Status LoadTileMaxShader(KCL::uint32 ws_size);
    KCL::KCL_Status LoadNeighborMax(KCL::uint32 ws_size);
    KCL::KCL_Status LoadFragmentShaders();

    void InitSceneForWarmup(WarmUpHelper *warm_up);
    KCL::KCL_Status WarmupTileMaxShader(WarmUpHelper *warm_up);
    KCL::KCL_Status WarmupNeighborMaxShader(WarmUpHelper *warm_up);

    std::vector<float> m_tap_offsets;
};

}

#endif // GLB_COMPUTE_MOTION_BLUR_H
