/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compute_hdr31.h"

#include "platform.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"

#include "opengl/ubo_cpp_defines.h"
#include "ubo_luminance.h"

ComputeHDR31::ComputeHDR31()
{   
	m_bright_pass_work_group_size = 8;    
    m_bright_pass_dispatch_count_x = 0;
    m_bright_pass_dispatch_count_y = 0;

	m_in_texture_bind = 1;    
    m_out_level0_bind = 0; 

    m_fragment_blur = NULL;
    m_reduction = NULL;

    m_input_texture = 0;    
    m_bright_texture_type = 0;   
}

ComputeHDR31::~ComputeHDR31()
{
    glDeleteSamplers(1, &m_input_sampler);
    glDeleteSamplers(1, &m_bloom_sampler);

	glDeleteTextures(1, &m_bright_texture);
    
    delete m_fragment_blur;
    delete m_reduction;
}

void ComputeHDR31::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 bloom_texture_gl_type, KCL::uint32 quad_vao, KCL::uint32 quad_vbo, GLB_Scene_ES2_*scene)
{
    m_width = width;
    m_height = height;

    m_bright_texture_type = bloom_texture_gl_type;
    m_bright_pass_dispatch_count_x = ((m_width / DOWNSAMPLE) + m_bright_pass_work_group_size - 1) / m_bright_pass_work_group_size;
    m_bright_pass_dispatch_count_y = ((m_height / DOWNSAMPLE) + m_bright_pass_work_group_size - 1) / m_bright_pass_work_group_size;
        
	KCL::uint32 blur_kernel_size = 9; // blur kernel size for FullHD
	blur_kernel_size = blur_kernel_size * m_width / 1920; // resize blur kernel for actual resolution

    m_fragment_blur = new GLB::FragmentBlur();
    m_fragment_blur->Init(quad_vao, quad_vbo, width / DOWNSAMPLE, height / DOWNSAMPLE, blur_kernel_size, bloom_texture_gl_type, 4, scene);

    // Init reduction pass
    m_reduction = new GLB::ComputeReduction();
    m_reduction->Init(m_width, m_height, scene);
    
    // Init bright pass
	InitBrightPass();
}

void ComputeHDR31::SetInputTexture(KCL::uint32 in_texture)
{
    m_input_texture = in_texture; 
    m_reduction->SetInputTexture(m_input_texture);
}

KCL::uint32 ComputeHDR31::GetBloomTexture() const
{
    return m_fragment_blur->GetOutputTexture();    
}

KCL::uint32 ComputeHDR31::GetBloomSampler() const
{
    return m_bloom_sampler;
}

KCL::uint32 ComputeHDR31::GetLuminanceBuffer() const
{
    return m_reduction->GetLuminanceBuffer();
}

void ComputeHDR31::Execute()
{    
    m_reduction->Execute();
	BrightPass();
}

void ComputeHDR31::InitBrightPass()
{
    CompileShader();
    SetupBrightTexture();
}

void ComputeHDR31::CompileShader()
{
	KCL::KCL_Status error;
	GLB::GLBShaderBuilder sb;
	sb.FileCs("hdr_luminance.h").FileCs("hdr_common.h").FileCs("bright_pass.shader");
    sb.AddDefine("OUT_TEX_TYPE "+GLB::GLBShader2::GLSLTextureFormat(m_bright_texture_type));
	sb.AddDefineInt("WORK_GROUP_SIZE",m_bright_pass_work_group_size);
	sb.AddDefineInt("IN_TEX_BIND",m_in_texture_bind);
	sb.AddDefineInt("OUT_TEX_LEVEL0_BIND",m_out_level0_bind);
    sb.AddDefineVec2("STEP_UV", KCL::Vector2D(1.0f / m_width, 1.0f / m_height));
	m_bright_pass = sb.Build(error);
}

void ComputeHDR31::SetupBrightTexture()
{    
    // Create a linear sampler for input
	glGenSamplers(1, &m_input_sampler);
	glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(m_input_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(m_input_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_input_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Create the bright texture
    glGenTextures(1, &m_bright_texture);
	glBindTexture(GL_TEXTURE_2D, m_bright_texture);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, BLUR_TEXTURE_COUNT);
        
    unsigned int width = m_width / DOWNSAMPLE;
	unsigned int height = m_height / DOWNSAMPLE;
	glTexStorage2D(GL_TEXTURE_2D, BLUR_TEXTURE_COUNT, m_bright_texture_type, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
    
    // Create a sampler to help bloom sampling
	glGenSamplers(1,& m_bloom_sampler);
	glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_bloom_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void ComputeHDR31::BrightPass()
{
    //GLbitfield barriers = GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT;
	//glMemoryBarrierProc(barriers);

	GLB::OpenGLStateManager::GlUseProgram(m_bright_pass->m_p);
    
    // Bind the input texture and the avg luminance
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_input_texture);
	glBindSampler(0, m_input_sampler);
    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, GetLuminanceBuffer());

    // Bind the output as image
	glBindImageTextureProc(m_out_level0_bind, m_bright_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_bright_texture_type);
    
    // Execute the bright pass   
    glDispatchComputeProc(m_bright_pass_dispatch_count_x, m_bright_pass_dispatch_count_y, 1);
    glMemoryBarrierProc(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);    

    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, 0);

    // Downsample the bright texture
    glBindTexture(GL_TEXTURE_2D, m_bright_texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

    // Blur the bright texture
    m_fragment_blur->SetInputTexture(m_bright_texture);
    m_fragment_blur->Execute();    
}