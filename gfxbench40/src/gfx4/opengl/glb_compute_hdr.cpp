/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_compute_hdr.h"

#include "platform.h"
#include "glb_scene_.h"

#include "opengl/glb_shader2.h"
#include "opengl/glb_filter.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_scene_opengl4_support.h"
#include "opengl/fragment_blur.h"
#include "opengl/compute_reduction4.h"

#include "opengl/ubo_cpp_defines.h"
#include "ubo_luminance.h"

#include <ng/log.h>

using namespace GLB;

ComputeHDR::ComputeHDR()
{
    m_reduction = NULL;
    m_bright_pass = NULL;
    m_fragment_blur = NULL;

    m_input_texture = 0;
}

ComputeHDR::~ComputeHDR()
{
    glDeleteSamplers(1, &m_input_sampler);
    glDeleteSamplers(1, &m_bloom_sampler);

    glDeleteTextures(1, &m_bloom_texture);

    delete m_reduction;
    delete m_bright_pass;
    delete m_fragment_blur;
}

void ComputeHDR::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, bool use_compute_bright_pass, KCL::uint32 quad_vao, KCL::uint32 quad_vbo, GLB_Scene_ES2_*scene)
{
    m_width = width;
    m_height = height;

    // blur kernel size for FullHD
    KCL::uint32 blur_kernel_size = 9;
    // resize blur kernel for actual resolution
    blur_kernel_size = blur_kernel_size * m_width / 1920;

    m_fragment_blur = new GLB::FragmentBlur();
    m_fragment_blur->Init(quad_vao, quad_vbo, width / DOWNSAMPLE, height / DOWNSAMPLE, blur_kernel_size, bloom_texture_gl_type, 4, scene);

    // Init reduction pass
	m_reduction = new GFXB4::ComputeReduction();
    m_reduction->Init(m_width, m_height, scene);

    // Init bright pass
    if (use_compute_bright_pass)
    {
        m_bright_pass = new ComputeBrightPass();
    }
    else
    {
        m_bright_pass = new FragmentBrightPass();
    }
    m_bright_pass->Init(width / DOWNSAMPLE, height / DOWNSAMPLE, bloom_texture_gl_type, quad_vao, quad_vbo);

    // Create a linear sampler for input
    glGenSamplers(1, &m_input_sampler);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_input_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Create the bright texture
    glGenTextures(1, &m_bloom_texture);
    glBindTexture(GL_TEXTURE_2D, m_bloom_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, BLUR_TEXTURE_COUNT);

    KCL::uint32 bloom_width = width / DOWNSAMPLE;
    KCL::uint32 bloom_height = height / DOWNSAMPLE;
    glTexStorage2D(GL_TEXTURE_2D, BLUR_TEXTURE_COUNT, bloom_texture_gl_type, bloom_width, bloom_height);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create a sampler to help bloom sampling
    glGenSamplers(1, &m_bloom_sampler);
    glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void ComputeHDR::SetInputTexture(KCL::uint32 in_texture)
{
    m_input_texture = in_texture;
    m_reduction->SetInputTexture(m_input_texture);
}

KCL::uint32 ComputeHDR::GetBloomTexture() const
{
    return m_fragment_blur->GetOutputTexture();
}

KCL::uint32 ComputeHDR::GetBloomSampler() const
{
    return m_bloom_sampler;
}

KCL::uint32 ComputeHDR::GetLuminanceBuffer() const
{
    return m_reduction->GetLuminanceBuffer();
}

GFXB4::ComputeReduction *ComputeHDR::GetComputeReduction() const
{
    return m_reduction;
}

void ComputeHDR::Execute()
{
    // Execture the reduction and calculate the luminance values
    m_reduction->Execute();

    // Execute the bright pass
    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, GetLuminanceBuffer());
    m_bright_pass->Execute(m_input_texture, m_input_sampler, m_bloom_texture);
    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, 0);

    // Downsample the bright texture
    glBindTexture(GL_TEXTURE_2D, m_bloom_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Blur the bright texture
    m_fragment_blur->SetInputTexture(m_bloom_texture);
    m_fragment_blur->Execute();
}


ComputeBrightPass::ComputeBrightPass()
{
    m_work_group_size_x = 0;
    m_work_group_size_y = 0;
    m_dispatch_count_x = 0;
    m_dispatch_count_y = 0;

    m_bloom_texture_gl_type = 0;
}

ComputeBrightPass::~ComputeBrightPass()
{
}

void ComputeBrightPass::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo)
{
    NGLOG_INFO("ComputeBrightPass - Init: %sx%s", width, height);

    m_bloom_texture_gl_type = bloom_texture_gl_type;

    SetWorkGroupSize();
    NGLOG_INFO("ComputeBrightPass - Work groups: %sx%s", m_work_group_size_x, m_work_group_size_y);

    m_dispatch_count_x = (width + m_work_group_size_x - 1) / m_work_group_size_x;
    m_dispatch_count_y = (height + m_work_group_size_y - 1) / m_work_group_size_y;

    KCL::KCL_Status error;
    GLB::GLBShaderBuilder sb;
    sb.FileCs("hdr_luminance.h").FileCs("hdr_common.h").FileCs("bright_pass.shader");
    sb.AddDefine("OUT_TEX_TYPE " + GLB::GLBShader2::GLSLTextureFormat(bloom_texture_gl_type));
    sb.AddDefineInt("WORK_GROUP_SIZE_X", m_work_group_size_x);
    sb.AddDefineInt("WORK_GROUP_SIZE_Y", m_work_group_size_y);
    sb.AddDefineInt("OUT_TEX_LEVEL0_BIND", 1);
    sb.AddDefineVec2("STEP_UV", KCL::Vector2D(1.0f / float(width) / float(ComputeHDR::DOWNSAMPLE), 1.0f / float(height) / float(ComputeHDR::DOWNSAMPLE)));
    m_bright_pass = sb.Build(error);
}

void ComputeBrightPass::SetWorkGroupSize()
{
    std::vector<WorkGroupValidator::WGConfig> configs;
    configs.push_back(WorkGroupValidator::WGConfig(32, 32));
    configs.push_back(WorkGroupValidator::WGConfig(16, 16));
    configs.push_back(WorkGroupValidator::WGConfig(8, 8));

    std::vector<WorkGroupValidator::WGConfig> result;
    WorkGroupValidator validator;
    if (validator.Validate(configs, result))
    {
        m_work_group_size_x = result[0].size_x;
        m_work_group_size_y = result[0].size_y;
    }
    else
    {
        NGLOG_ERROR("ComputeBrightPass - No suitable WG configuration!");
        m_work_group_size_x = 1;
        m_work_group_size_y = 1;
    }
}

void ComputeBrightPass::Execute(KCL::uint32 input_texture, KCL::uint32 input_sampler, KCL::uint32 output_texture)
{
    GLB::OpenGLStateManager::GlUseProgram(m_bright_pass->m_p);

    // Bind the input texture and the avg luminance
    GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_texture);
    glBindSampler(0, input_sampler);

    // Bind the output as image
    glBindImageTextureProc(1, output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_bloom_texture_gl_type);

    // Execute the bright pass
    glDispatchComputeProc(m_dispatch_count_x, m_dispatch_count_y, 1);
    glMemoryBarrierProc(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);
}


FragmentBrightPass::FragmentBrightPass()
{
    m_output_texture = 0;
}

FragmentBrightPass::~FragmentBrightPass()
{
    delete m_filter;
}

void FragmentBrightPass::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo)
{
    NGLOG_INFO("FragmentBrightPass - Init: %sx%s", width, height);
    m_filter = new GLBFilter();
    m_filter->Init(quad_vao, quad_vbo, 0, width, height, false, 1, 0, bloom_texture_gl_type);

    KCL::KCL_Status error;
    GLBShaderBuilder sb;
    m_filter->m_shader = sb.ShaderFile("bright_pass_fs.shader").Build(error);
}

void FragmentBrightPass::Execute(KCL::uint32 input_texture, KCL::uint32 input_sampler, KCL::uint32 output_texture)
{
    if (m_output_texture != output_texture)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_filter->GetFramebufferObject(0));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture, 0);
        m_output_texture = output_texture;
    }

    m_filter->m_input_textures[0] = input_texture;
    m_filter->m_input_samplers[0] = input_sampler;
    m_filter->Render();
}
