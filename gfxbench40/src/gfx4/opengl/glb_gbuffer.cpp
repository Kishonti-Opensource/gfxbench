/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_gbuffer.h"
#include "platform.h"
#include "opengl/fbo.h"
#include "opengl/glb_scene_opengl4_support.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_opengl_state_manager.h"

using namespace GFXB4;

static const GLfloat velocity_clear_rgba8[] = { 0.499f, 0.499f, 0.499f, 0.499f };
static const GLfloat velocity_clear_rgba10_2[] = { 0.5f, 0.5f, 0.5f, 0.5f };

#define USE_HIZ_GEN_MIPMAPS 1

GBuffer::GBuffer()
{
    m_fbo = 0;

    m_albedo_map = 0;
    m_depth_texture = 0;
    m_velocity_buffer = 0;
    m_normal_map = 0;
    m_params_map = 0;
    m_transparent_accum_map = 0;

    m_viewport_width = 0;
    m_viewport_height = 0;

    m_draw_buffers[0] = GL_COLOR_ATTACHMENT0;
    m_draw_buffers[1] = GL_COLOR_ATTACHMENT1;
    m_draw_buffers[2] = GL_COLOR_ATTACHMENT2;
    m_draw_buffers[3] = GL_COLOR_ATTACHMENT3;

    for ( KCL::uint32 i = 0; i < DEPTH_HIZ_LEVELS; i++)
    {
        m_downsample_fbos[i] = 0;
    }
    m_hiz_depth_levels = 0;
    m_depth_hiz_texture = 0;
    m_linearize_shader = NULL;
    m_downsample_shader = NULL;
}

GBuffer::~GBuffer()
{
    glDeleteTextures( 1, &m_albedo_map);
    glDeleteTextures( 1, &m_depth_texture);
    glDeleteTextures( 1, &m_velocity_buffer);
    glDeleteTextures( 1, &m_normal_map);
    glDeleteTextures( 1, &m_params_map);
    glDeleteTextures( 1, &m_transparent_accum_map);

    glDeleteFramebuffers( 1, &m_fbo);
    glDeleteFramebuffers( 1, &m_transparent_fbo);

    glDeleteTextures( 1, &m_depth_hiz_texture);

    for ( KCL::uint32 i = 0; i < DEPTH_HIZ_LEVELS; i++)
    {
        if ( m_downsample_fbos[i])
        {
            glDeleteFramebuffers( 1, &m_downsample_fbos[i]);
        }
    }

    GLB::FBO::bind( 0);
}

bool GBuffer::Init( KCL::uint32 w, KCL::uint32 h)
{
    m_viewport_width = w;
    m_viewport_height = h;

    m_depth_texture = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_DEPTH_COMPONENT24);
    m_albedo_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8); //albedoRGB + emissive
    m_velocity_buffer = Create2DTexture( 1,false,m_viewport_width,m_viewport_height, VELOCITY_BUFFER_RGBA8 ? GL_RGBA8 : GL_RGB10_A2);
    m_normal_map  = Create2DTexture( 1,false,m_viewport_width,m_viewport_height, NORMAL_BUFFER_RGBA8 ? GL_RGBA8 : GL_RGB10_A2); //normalXY
    m_params_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8); //specColRGB,smoothness
    m_transparent_accum_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8); //to accumulate transparents, so that we can do programmable blending with the RGBM buffer

    glGenFramebuffers( 1, &m_fbo);
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_albedo_map, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_velocity_buffer, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_normal_map, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_params_map, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

    glGenFramebuffers( 1, &m_transparent_fbo);
    glBindFramebuffer( GL_FRAMEBUFFER, m_transparent_fbo);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_transparent_accum_map, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

    KCL::uint32 max_texture_levels = KCL::uint32( KCL::texture_levels( m_viewport_width, m_viewport_height));
    m_hiz_depth_levels = max_texture_levels < DEPTH_HIZ_LEVELS ? max_texture_levels : DEPTH_HIZ_LEVELS;

    m_depth_hiz_texture = Create2DTexture( m_hiz_depth_levels, true, m_viewport_width, m_viewport_height, GL_RGBA8);

    glGenFramebuffers( 1, &m_downsample_fbos[0]);
    glBindFramebuffer( GL_FRAMEBUFFER, m_downsample_fbos[0]);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_depth_hiz_texture, 0);

#if !USE_HIZ_GEN_MIPMAPS
    for ( KCL::uint32 i = 1; i < m_hiz_depth_levels; i++)
    {
        glGenFramebuffers( 1, &m_downsample_fbos[i]);
        glBindFramebuffer( GL_FRAMEBUFFER, m_downsample_fbos[i]);
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_depth_hiz_texture, i);
    }
#endif

    GLB::FBO::bind( 0);

    return InitShaders();
}

bool GBuffer::InitShaders()
{
    KCL::KCL_Status status;
    GLB::GLBShaderBuilder sb;

    INFO("Enable depth hiz: %d", HIZ_DEPTH_ENABLED);

    m_linearize_shader = sb.ShaderFile( "linearize_depth.shader").Build( status);
    if ( status != KCL::KCL_TESTERROR_NOERROR)
    {
        return false;
    }
    GLB::OpenGLStateManager::GlUseProgram(m_linearize_shader->m_p);
    glUniform1i(m_linearize_shader->m_uniform_locations[GLB::uniforms::depth_unit0], 0);

#if !USE_HIZ_GEN_MIPMAPS
    m_downsample_shader = sb.ShaderFile( "downsample_depth.shader").Build( status);
    if ( status != KCL::KCL_TESTERROR_NOERROR)
    {
        return false;
    }

    GLB::OpenGLStateManager::GlUseProgram(m_downsample_shader->m_p);
    glUniform1i(m_downsample_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
#endif

    GLB::OpenGLStateManager::GlUseProgram(0);

    return true;
}

void GBuffer::BindWorldBuffer()
{
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);
    glDrawBuffers( 4, m_draw_buffers);
    glViewport( 0, 0, m_viewport_width, m_viewport_height);
}

void GBuffer::BindTransparentAccumBuffer()
{
    glBindFramebuffer( GL_FRAMEBUFFER, m_transparent_fbo);
    glDrawBuffers( 1, m_draw_buffers);

    glViewport( 0, 0, m_viewport_width, m_viewport_height);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT);
}

void GBuffer::ClearWorldBuffer()
{
    static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const GLfloat one[] =  { 1.0f };

    glClearBufferfv(GL_COLOR, 0, zeros);
    glClearBufferfv(GL_COLOR, 1, VELOCITY_BUFFER_RGBA8 ? velocity_clear_rgba8 : velocity_clear_rgba10_2);
    glClearBufferfv(GL_COLOR, 2, zeros);
    glClearBufferfv(GL_COLOR, 3, zeros);
    glClearBufferfv(GL_DEPTH, 0, one);
}

void GBuffer::ClearVelocityBuffer()
{
    glDrawBuffers(2, m_draw_buffers);
    glClearBufferfv(GL_COLOR, 1, VELOCITY_BUFFER_RGBA8 ? velocity_clear_rgba8 : velocity_clear_rgba10_2);
}

void GBuffer::DownsampleDepth(const KCL::Camera2 *camera, KCL::uint32 quad_vao)
{
    if (!HIZ_DEPTH_ENABLED)
    {
        return;
    }

    GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0);
    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
    GLB::OpenGLStateManager::Commit();

    glBindVertexArray( quad_vao);

    glBindFramebuffer( GL_FRAMEBUFFER, m_downsample_fbos[0]);
    glClear( GL_COLOR_BUFFER_BIT);

    GLB::OpenGLStateManager::GlUseProgram( m_linearize_shader->m_p);
    glBindTexture( GL_TEXTURE_2D, m_depth_texture);
    glBindSampler( 0, 0);

    KCL::Vector4D depth_linearize_factors;
    depth_linearize_factors.x = camera->GetNear() - camera->GetFar();
    depth_linearize_factors.z = camera->GetNear();
    depth_linearize_factors.w = camera->GetFar();

    glUniform4fv( m_linearize_shader->m_uniform_locations[GLB::uniforms::depth_parameters], 1, depth_linearize_factors.v);

    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture( GL_TEXTURE_2D, m_depth_hiz_texture);

#if USE_HIZ_GEN_MIPMAPS
    glGenerateMipmap( GL_TEXTURE_2D);
#else

    GLB::OpenGLStateManager::GlUseProgram( m_downsample_shader->m_p);

    KCL::uint32 width = m_viewport_width;
    KCL::uint32 height = m_viewport_height;

    for ( KCL::uint32 i = 1; i < m_hiz_depth_levels; i++)
    {
        width = KCL::Max(width / 2, 1u);
        height = KCL::Max(height / 2, 1u);

        glViewport( 0, 0, width, height);

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i - 1);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1);

        glBindFramebuffer( GL_FRAMEBUFFER, m_downsample_fbos[i]);

        glClear( GL_COLOR_BUFFER_BIT);

        glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    }

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_hiz_depth_levels - 1);
#endif

    glBindTexture( GL_TEXTURE_2D, 0);

    glBindVertexArray( 0);
}