/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "fragment_blur.h"
#include "gauss_blur_helper.h"

#include "platform.h"
#include "opengl/glb_discard_functions.h"
#include "glb_scene_.h"
#include "opengl/glb_opengl_state_manager.h"

using namespace GLB;

FragmentBlur::FragmentBlur()
{
	m_width = 0;
    m_height = 0;
    m_lod_levels = 0;
    m_has_lod = false;

	m_sampler = 0;

	m_blur_shader_h = NULL;
	m_blur_shader_v = NULL;

    //for statistics
    m_scene = NULL;
}

FragmentBlur::~FragmentBlur()
{
	glDeleteSamplers(1,&m_sampler);
}

void FragmentBlur::Init(KCL::uint32 vao, KCL::uint32 vbo, KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, KCL::uint32 texture_format, KCL::uint32 lod_levels, GLB_Scene_ES2_* scene)
{
    m_scene = scene;
    m_width = width;
    m_height = height;
    m_lod_levels = lod_levels;
    m_has_lod = m_lod_levels > 1;

    blur_kernel_size = KCL::Max(blur_kernel_size, 2u);
    
	// Generate the shader constants
    std::vector<float> gauss_weights = COMMON31::GaussBlurHelper::GetGaussWeights(blur_kernel_size, true);
    std::vector<float> packed_weights = COMMON31::GaussBlurHelper::CalcPackedWeights(gauss_weights);

    std::vector<float> horizontal_packed_offsets;
    std::vector<float> vertical_packed_offsets;    
    std::string shader_file;
	std::vector<KCL::Vector2D> stepuv;
    if (m_has_lod)
    {   
        horizontal_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(1, blur_kernel_size, gauss_weights);
        vertical_packed_offsets = horizontal_packed_offsets;

         // Calc step values for gauss	   
	    stepuv.resize(lod_levels);

        KCL::uint32 k = 1;
	    for (KCL::uint32 i = 0; i < lod_levels; i++)
	    {
		    stepuv[i].x = 1.0 / float(std::max(m_width / k,  1u));
		    stepuv[i].y = 1.0 / float(std::max(m_height / k, 1u));
		    k *= 2;
	    }
        shader_file = "gauss_blur_fs_lod.shader";
    }
    else
    {
        vertical_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_height, blur_kernel_size, gauss_weights);
        horizontal_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_width, blur_kernel_size, gauss_weights);
        shader_file = "gauss_blur_fs.shader";
    }

    // Compile the vertical shader
    KCL::KCL_Status error;
	GLBShaderBuilder sb;
    sb.ShaderFile(shader_file.c_str());
    sb.AddDefine("VERTICAL");        
    sb.AddDefineInt("KS", blur_kernel_size + 1);
	if (m_has_lod)
	{
		sb.AddDefineInt("LOD_LEVEL_COUNT", lod_levels) ;
	}
    m_blur_shader_v = sb.Build(error);

    // Compile the horizontal shader
    sb.ShaderFile(shader_file.c_str());
    sb.AddDefine("HORIZONTAL");        
    sb.AddDefineInt("KS", blur_kernel_size + 1);
	if (m_has_lod)
	{
		sb.AddDefineInt("LOD_LEVEL_COUNT", lod_levels) ;
	}
    m_blur_shader_h = sb.Build(error);

    // Setup the vertical shader
    GLB::OpenGLStateManager::GlUseProgram(m_blur_shader_v->m_p) ;
	glUniform1fv(m_blur_shader_v->m_uniform_locations[uniforms::gauss_weights], packed_weights.size(), &packed_weights[0] ) ;
	glUniform1fv(m_blur_shader_v->m_uniform_locations[uniforms::gauss_offsets], vertical_packed_offsets.size(), &vertical_packed_offsets[0] ) ;
	if (m_has_lod)
	{
		glUniform2fv(m_blur_shader_v->m_uniform_locations[uniforms::inv_resolution], lod_levels, stepuv[0].v) ;
	}

    // Setup the horizontal shader
	GLB::OpenGLStateManager::GlUseProgram(m_blur_shader_h->m_p) ;
	glUniform1fv(m_blur_shader_h->m_uniform_locations[uniforms::gauss_weights], packed_weights.size(), &packed_weights[0] ) ;
	glUniform1fv(m_blur_shader_h->m_uniform_locations[uniforms::gauss_offsets], horizontal_packed_offsets.size(), &horizontal_packed_offsets[0] ) ;
	if (m_has_lod)
	{
		glUniform2fv(m_blur_shader_h->m_uniform_locations[uniforms::inv_resolution], lod_levels, stepuv[0].v) ;
	}

   	glGenSamplers(1, &m_sampler);
	glSamplerParameteri(m_sampler,GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glSamplerParameteri(m_sampler,GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glSamplerParameteri(m_sampler,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_sampler,GL_TEXTURE_MIN_FILTER, m_has_lod ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

	// Setup the filters
    m_blur_filter_v.Init(vao, vbo, 0, m_width, m_height, false, lod_levels, 0, texture_format);
	m_blur_filter_v.m_shader = m_blur_shader_v;
	m_blur_filter_v.m_input_samplers[0] = m_sampler;
    m_blur_filter_v.m_scene = m_scene;

    m_blur_filter_h.Init(vao, vbo, 0, m_width, m_height, false, lod_levels, 0, texture_format);
	m_blur_filter_h.m_shader = m_blur_shader_h;
	m_blur_filter_h.m_input_samplers[0] = m_sampler;
    m_blur_filter_h.m_input_textures[0] = m_blur_filter_v.m_color_texture;
    m_blur_filter_h.m_scene = m_scene;
}

void FragmentBlur::Execute() 
{
    // Execute the vertical pass
    for (KCL::uint32 i = 0; i < m_lod_levels; i++)
    {            		
        m_blur_filter_v.m_gauss_lod_level = i;
        m_blur_filter_v.m_render_target_level = i;
#ifdef OCCLUSION_QUERY_BASED_STAT
        m_blur_filter_v.Render(m_scene->m_glGLSamplesPassedQuery);
#else
        m_blur_filter_v.Render();
#endif
    }

    // Execute the horizontal pass
    for (KCL::uint32 i = 0; i < m_lod_levels; i++)
    {
        m_blur_filter_h.m_gauss_lod_level = i;
        m_blur_filter_h.m_render_target_level = i;
#ifdef OCCLUSION_QUERY_BASED_STAT
	    m_blur_filter_h.Render(m_scene->m_glGLSamplesPassedQuery);
#else
	    m_blur_filter_h.Render();
#endif
    }
}

void FragmentBlur::SetInputTexture(KCL::uint32 in_tex)
{
	m_blur_filter_v.m_input_textures[0] = in_tex;
}

KCL::uint32 FragmentBlur::GetOutputTexture() const
{
    return m_blur_filter_h.m_color_texture;
}