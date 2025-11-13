/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined HAVE_GLES3 || defined HAVE_GLEW
#include "gbuffer.h"
#include "platform.h"
#include "opengl/fbo.h"
#include "kcl_os.h"

#include <vector>
#include <algorithm>

using namespace GLB;

void PP::Clear()
{
	GLenum e;
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &m_color_map); 
	e = glGetError();

	glDeleteTextures(1, &m_normal_map);
	e = glGetError();
	
	glDeleteTextures(1, &m_reflection_map);
	e = glGetError();

	glDeleteTextures(1, &m_final_texture);
	e = glGetError();

	glDeleteTextures(1, &m_param_map);
	e = glGetError();

	glDeleteTextures(1, &m_depth_texture);
	e = glGetError();


	glDeleteRenderbuffers(1, &m_color_map2); 
	e = glGetError();

	glDeleteRenderbuffers(1, &m_normal_map2);
	e = glGetError();
	
	glDeleteRenderbuffers(1, &m_reflection_map2);
	e = glGetError();

	glDeleteRenderbuffers(1, &m_param_map2);
	e = glGetError();

	glDeleteRenderbuffers(1, &m_depth_texture2);
	e = glGetError();


	FBO::bind(0);
	glDeleteFramebuffers( 1, &m_fbo);
	glDeleteFramebuffers( 1, &m_msaa_fbo);
	e = glGetError();

	memset( this, 0, sizeof( PP));
}


bool PP::Init( KCL::uint32 w, KCL::uint32 h, KCL::uint32 samples, bool final_texture_fp)
{
	m_viewport_width = w;
	m_viewport_height = h;
	m_samples = samples;

	m_depth_texture = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_DEPTH_COMPONENT24);
	m_color_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8);
	m_normal_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8);
	m_reflection_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8);
	m_param_map = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, GL_RGBA8);
	m_final_texture = Create2DTexture( 1, false, m_viewport_width, m_viewport_height, final_texture_fp ? GL_RGB10_A2 : GL_RGBA8); //do not require GL_RGBA16F - GL_EXT_color_buffer_half for ES 3.1

	if( m_samples)
	{
		m_depth_texture2 = CreateRenderbuffer( m_samples, m_viewport_width, m_viewport_height, GL_DEPTH_COMPONENT24);
		m_color_map2 = CreateRenderbuffer( m_samples, m_viewport_width, m_viewport_height, GL_RGBA8);
		m_normal_map2 = CreateRenderbuffer( m_samples, m_viewport_width, m_viewport_height, GL_RGBA8);
		m_reflection_map2 = CreateRenderbuffer( m_samples, m_viewport_width, m_viewport_height, GL_RGBA8);
		m_param_map2 = CreateRenderbuffer( m_samples, m_viewport_width, m_viewport_height, GL_RGBA8);

		if( !m_depth_texture2 || !m_color_map2 || !m_normal_map2 || !m_reflection_map2 || !m_param_map2)
		{
			return false;
		}
	}

	glGenFramebuffers( 1, &m_fbo);
	glGenFramebuffers( 1, &m_msaa_fbo);

	BindGBuffer();
	FBO::bind( 0);

	return true;
}


static GLenum b[] = 
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3
};


void PP::BindGBuffer()
{
	glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normal_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_reflection_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_param_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);


	if( m_samples)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_msaa_fbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_color_map2);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, m_normal_map2);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, m_reflection_map2);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, m_param_map2);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_texture2);
	}

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

	glDrawBuffers( 4, b);

	int status;

	status = glCheckFramebufferStatus( GL_FRAMEBUFFER);
}


void PP::BindFinalBuffer()
{
	if( m_samples)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaa_fbo);
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_color_map2);
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, m_normal_map2);
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, m_reflection_map2);
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, m_param_map2);
		//glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_texture2);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_map, 0);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normal_map, 0);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_reflection_map, 0);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_param_map, 0);
		//glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

		glReadBuffer( GL_COLOR_ATTACHMENT0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_map, 0);
		glBlitFramebuffer( 0, 0, m_viewport_width, m_viewport_height, 0, 0, m_viewport_width, m_viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer( GL_COLOR_ATTACHMENT1);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_normal_map, 0);
		glBlitFramebuffer( 0, 0, m_viewport_width, m_viewport_height, 0, 0, m_viewport_width, m_viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer( GL_COLOR_ATTACHMENT2);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_reflection_map, 0);
		glBlitFramebuffer( 0, 0, m_viewport_width, m_viewport_height, 0, 0, m_viewport_width, m_viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer( GL_COLOR_ATTACHMENT3);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_param_map, 0);
		glBlitFramebuffer( 0, 0, m_viewport_width, m_viewport_height, 0, 0, m_viewport_width, m_viewport_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBlitFramebuffer( 0, 0, m_viewport_width, m_viewport_height, 0, 0, m_viewport_width, m_viewport_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}
 

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_final_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);
	glDrawBuffers( 1, b);

	int status;

	status = glCheckFramebufferStatus( GL_FRAMEBUFFER);

}

uint32 PP::Create2DTexture( KCL::uint32 max_mipmaps, bool linear, uint32 w, uint32 h, GLint format)
{
	uint32 texture_object;

	KCL::uint32 m_uiMipMapCount = 1;

    if( max_mipmaps == 0) //0 means complete mipchain
	{
		KCL::uint32 kk = std::max( w, h);

		while( kk > 1)
		{
			m_uiMipMapCount++;
			kk /= 2;
		}

	}
    else
    {
        m_uiMipMapCount = max_mipmaps;
    }

    bool mipmapped = m_uiMipMapCount > 1;
	
	glGenTextures(1, &texture_object);

	glBindTexture( GL_TEXTURE_2D, texture_object);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if( linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
	}

	glTexStorage2D( GL_TEXTURE_2D, m_uiMipMapCount, format, w, h);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_uiMipMapCount);
	
	glBindTexture( GL_TEXTURE_2D, 0);

	int e = glGetError();
	if( e)
	{
		INFO( "GL error (%x) - Create2DTexture", e);
	}

	return texture_object;
}

uint32 PP::CreateRenderbuffer( int samples, uint32 w, uint32 h, GLint format)
{
	int32 num_configs = 0;
	std::vector<int32> configs;
	KCL::uint32 rbo;

	glGetInternalformativ( GL_RENDERBUFFER, format, GL_NUM_SAMPLE_COUNTS, 1, &num_configs);
	configs.resize( num_configs);
	glGetInternalformativ( GL_RENDERBUFFER, format, GL_SAMPLES, num_configs, &configs[0]);

	std::vector<int32>::iterator config_found = std::find( configs.begin(), configs.end(), samples);

	if( config_found == configs.end())
	{
		return 0;
	}

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer( GL_RENDERBUFFER, rbo);
	if( samples)
	{
		glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, format, w, h);
	}
	else
	{
		glRenderbufferStorage( GL_RENDERBUFFER, format, w, h);
	}
	glBindRenderbuffer( GL_RENDERBUFFER, 0);

	return rbo;
}
#endif
