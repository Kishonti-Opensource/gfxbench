/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_GBUFFER_H
#define GLB_GBUFFER_H

#include <kcl_base.h>

namespace GLB
{
    class GLBShader2;
};

namespace GFXB4
{

class GBuffer
{
    static const KCL::uint32 DEPTH_HIZ_LEVELS = 4;

public:
    static const bool VELOCITY_BUFFER_RGBA8 = false;
    static const bool NORMAL_BUFFER_RGBA8 = false;
    static const bool HIZ_DEPTH_ENABLED = true;

    GBuffer();
    ~GBuffer();

    bool Init( KCL::uint32 w, KCL::uint32 h);
    bool InitShaders();

    void DownsampleDepth(const KCL::Camera2 *camera, KCL::uint32 quad_vao);

    void BindWorldBuffer();
    void BindTransparentAccumBuffer();

    void ClearWorldBuffer();
    void ClearVelocityBuffer();

    KCL::uint32 m_fbo;
    KCL::uint32 m_transparent_fbo;

    KCL::uint32 m_albedo_map; //GL_RGBA8, albedoRGB + emissive
    KCL::uint32 m_depth_texture; //GL_DEPTH_COMPONENT24
    KCL::uint32 m_velocity_buffer; //GL_RGBA8
    KCL::uint32 m_normal_map; //GL_RGBA8, normalXY
    KCL::uint32 m_params_map; //GL_RGBA8, specColRGB, smoothness
    KCL::uint32 m_transparent_accum_map; //GL_RGBA8, to accumulate transparents, so that we can do programmable blending with the RGBM buffer

    KCL::uint32 m_viewport_width;
    KCL::uint32 m_viewport_height;

    KCL::uint32 m_draw_buffers[4];

    KCL::uint32 m_hiz_depth_levels;

    KCL::uint32 m_depth_hiz_texture; //GL_RGBA8, encoded linear 0...1 depth with [DEPTH_HIZ_LEVELS] mipmaps
    GLB::GLBShader2 *m_linearize_shader;
    GLB::GLBShader2 *m_downsample_shader;
    KCL::int32 m_texture_size_pos;

    KCL::uint32 m_downsample_fbos[DEPTH_HIZ_LEVELS];
};

}

#endif
