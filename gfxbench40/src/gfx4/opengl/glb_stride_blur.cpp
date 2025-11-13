/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform.h"
#include "glb_stride_blur.h"
#include "opengl/fbo.h"
#include "opengl/glb_opengl_state_manager.h"

#include <ng/log.h>

#include <sstream>
#include <iomanip> // std::setprecision

using namespace GLB;

StrideBlur::StrideBlur()
{
	m_shader_h = 0;
	m_shader_v = 0;

	m_fbo1 = 0;
	m_fbo2 = 0;
	m_sampler = 0;
    m_depth_sampler = 0;

	m_temp_texture = 0;
	m_color_texture = 0;	
}

StrideBlur::~StrideBlur()
{
	glDeleteFramebuffers(1, &m_fbo1);
	glDeleteFramebuffers(1, &m_fbo2);

	glDeleteSamplers(1, &m_sampler);
    glDeleteSamplers(1, &m_depth_sampler);

	glDeleteTextures(1, &m_temp_texture);
	glDeleteTextures(1, &m_color_texture);
}

void StrideBlur::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 stride, KCL::uint32 kernel_size)
{
	kernel_size = KCL::Max(kernel_size, 2u);

	NGLOG_INFO("Stride blur kernel: %s", kernel_size);

	glGenSamplers(1, &m_sampler);
	glSamplerParameterf(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameterf(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameterf(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameterf(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenSamplers(1, &m_depth_sampler);
    glSamplerParameterf(m_depth_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameterf(m_depth_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameterf(m_depth_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameterf(m_depth_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_temp_texture) ;
	glBindTexture(GL_TEXTURE_2D, m_temp_texture);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height); //BA = linear depth

	glGenTextures(1, &m_color_texture) ;
	glBindTexture(GL_TEXTURE_2D, m_color_texture);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG8, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &m_fbo1);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_temp_texture, 0);

	glGenFramebuffers(1, &m_fbo2);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo2);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

	KCL::KCL_Status status;
	GLBShaderBuilder sb;
	sb.AddDefineInt("KS", kernel_size);
	sb.AddDefine("GAUSS_WEIGHTS " + GetGaussWeightsString(kernel_size));
	sb.AddDefine("OFFSETS " + GetOffsetsString(kernel_size, stride, width, false));
	sb.AddDefine("PASS_DEPTH");
	m_shader_h = sb.ShaderFile("pp_stride_blur.shader").Build(status);

	sb.AddDefineInt("KS", kernel_size);
	sb.AddDefine("GAUSS_WEIGHTS " + GetGaussWeightsString(kernel_size));
	sb.AddDefine("OFFSETS " + GetOffsetsString(kernel_size, stride, height, true));	
	m_shader_v = sb.ShaderFile("pp_stride_blur.shader").Build(status);

	OpenGLStateManager::GlUseProgram(m_shader_h->m_p) ;	
	glUniform1i(m_shader_h->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
	glUniform1i(m_shader_h->m_uniform_locations[GLB::uniforms::depth_unit0], 1);

	OpenGLStateManager::GlUseProgram(m_shader_v->m_p) ;	
	glUniform1i(m_shader_v->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
    glUniform1i(m_shader_v->m_uniform_locations[GLB::uniforms::depth_unit0], 1);
}

void StrideBlur::Render(KCL::uint32 fullscreen_vao, KCL::Camera2 * camera, KCL::uint32 input_texture, KCL::uint32 depth_texture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_temp_texture, 0);

	OpenGLStateManager::GlActiveTexture(GL_TEXTURE1);
    glBindSampler(1, m_depth_sampler);
    glBindTexture(GL_TEXTURE_2D, depth_texture);

	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);	
	glBindSampler(0, m_sampler);
	glBindTexture(GL_TEXTURE_2D, input_texture);

	OpenGLStateManager::GlUseProgram(m_shader_h->m_p);
    glUniform4fv(m_shader_h->m_uniform_locations[GLB::uniforms::depth_parameters], 1, camera->m_depth_linearize_factors.v);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(fullscreen_vao);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo2);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_temp_texture);
	
    OpenGLStateManager::GlUseProgram(m_shader_v->m_p);
    glUniform4fv(m_shader_v->m_uniform_locations[GLB::uniforms::depth_parameters], 1, camera->m_depth_linearize_factors.v);

    glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    
    glBindSampler(0, 0);
    glBindSampler(1, 0);
	glBindVertexArray(0);    
	//glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName());
}

std::vector<float> StrideBlur::GetGaussWeights(unsigned int kernel_size)
{
	std::vector<float> gauss_weights ;

	double sigma = (1.0*kernel_size)/3.0 ;

	float sum_weights = 1.0f;
	for (int i = -1*kernel_size; i <= (int)kernel_size ;++i)
    {
        if (i != 0)
        {
            float w = exp(-0.5*i*i / (sigma*sigma));
            gauss_weights.push_back(w);
            sum_weights += w;
        }
	}

	// Normalize weights
    /*
	for (KCL::uint32 i = 0; i < gauss_weights.size(); i++)
	{
		gauss_weights[i] /= sum_weights;
	}
    */

	return gauss_weights ;
}

std::string StrideBlur::GetGaussWeightsString(unsigned int kernel_size)
{
	std::stringstream sstream ;

	std::vector<float> gauss_weights = GetGaussWeights(kernel_size) ;
	std::vector<float>::iterator it = gauss_weights.begin() ;

	sstream<<std::fixed<<std::setprecision(10)<<*it ;
	it++ ;

	for(  ; it != gauss_weights.end() ; it++)
	{
		sstream<<", "<<*it ;
	}

	return sstream.str() ;
}

std::string StrideBlur::GetOffsetsString(KCL::uint32 kernel_size, float stride, KCL::uint32 width, bool y)
{
	std::stringstream sstream;

	sstream<<std::fixed<<std::setprecision(10) ;

	for (KCL::uint32 i = 0; i < kernel_size * 2 + 1; i++)
	{
        if (i == kernel_size)
        {
            continue;
        }

		float offset = (float(i) - float(kernel_size)) * stride / width;
		sstream << "vec2(";
		if (y)
		{
			sstream << "0.0, " << offset << ")";
		}
		else
		{
			sstream << offset << ", 0.0)";
		}

		if (i != kernel_size * 2)
		{
			sstream << ", ";
		}
	}
	return sstream.str();
}