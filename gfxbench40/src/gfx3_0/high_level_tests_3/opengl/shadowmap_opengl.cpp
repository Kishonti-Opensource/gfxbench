/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "shadowmap.h"
#include "platform.h"
#include "opengl/shader.h"
#include "opengl/misc2_opengl.h"
#include <string>
#include "opengl/fbo.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/ext.h"
#include "kcl_io.h"
#include "ng/log.h"

using namespace std;
using namespace KCL;
using namespace GLB;


static void CreateTexture565( uint32 size, bool immutable)
{
	if( immutable)
	{
#if defined HAVE_GLES3 || defined __glew_h__
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB565, size, size);
#else
		INFO("immutable not supported!");
#endif
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
	}
}


static void CreateTexture888( uint32 size, bool immutable)
{
	if( immutable)
	{
#if defined HAVE_GLES3 || defined __glew_h__
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, size, size);
#else
		INFO("immutable not supported!");
#endif
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
}


static void CreateTextureDepth( uint32 size, bool immutable)
{
	if( immutable)
	{
#if defined HAVE_GLES3 || defined __glew_h__
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, size, size);
#else
		INFO("immutable not supported!");
#endif
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	}
}


ShadowMap* ShadowMap::Create( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled, bool is_immutable)
{
	if( m == "simple projective")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled, is_immutable);
	}
	else if( m == "depth map(depth)")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled, is_immutable);
	}
	else if( m == "depth map(color)")
	{
		return new ShadowMap( size, m, fullscreen_quad_vbo, is_blur_enabled, is_immutable);
	}
	else
	{
		return 0;
	}
}


ShadowMap::ShadowMap( int size, const std::string &m, uint32 &fullscreen_quad_vbo, bool is_blur_enabled, bool is_immutable)
:
m_size( size),
m_rboid( 0),
m_clear_mask( 0),
m_blur( is_blur_enabled && m == "simple projective" ? new Blur(fullscreen_quad_vbo, m_size, is_immutable) : 0)
{
	glGenFramebuffers( 1, &m_fboid );
	
	glGenTextures(1, &m_tboid);
#if defined HAVE_GLES3 || defined __glew_h__

	if (GLB::g_extension->hasFeature(GLB::GLBFEATURE_sampler_object))
	{
		glGenSamplers(1, &m_sampler_id) ;
	}
#endif
	glBindTexture(GL_TEXTURE_2D, m_tboid);

	GLuint texture_min_filter = 0 ;
	GLuint texture_mag_filter = 0 ;

	GLuint texture_compare_mode = 0 ;
	GLuint texture_compare_func = 0 ;

	if( m == "simple projective")
	{
		texture_min_filter = GL_LINEAR ;
		texture_mag_filter = GL_LINEAR ;

		CreateTexture565( m_size, is_immutable);

		m_clear_mask = GL_COLOR_BUFFER_BIT;
	}
	else if( m == "depth map(depth)")
	{
		texture_compare_mode = GL_COMPARE_REF_TO_TEXTURE ;
		texture_compare_func = GL_LEQUAL ;

		texture_min_filter = GL_LINEAR ;
		texture_mag_filter = GL_LINEAR ;

		CreateTextureDepth( m_size, is_immutable);

		m_clear_mask = GL_DEPTH_BUFFER_BIT;
	}
	else if( m == "depth map(color)")
	{
		texture_min_filter = GL_NEAREST ;
		texture_mag_filter = GL_NEAREST ;

		CreateTexture888( m_size, is_immutable);

		glGenRenderbuffers(1, &m_rboid);

		glBindRenderbuffer(GL_RENDERBUFFER, m_rboid);
		glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_size, m_size);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		m_clear_mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	}
	else
	{
		NGLOG_ERROR("Shadowmap parameters no correctly set!") ;
	}

	GLuint texture_wrap_s = GL_CLAMP_TO_EDGE ;
	GLuint texture_wrap_t = GL_CLAMP_TO_EDGE ;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_mag_filter);

#if defined HAVE_GLES3 || defined __glew_h__
	if (texture_compare_mode != 0)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,texture_compare_mode);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,texture_compare_func);
	}
#endif

	
#if defined HAVE_GLES3 || defined __glew_h__
	if (GLB::g_extension->hasFeature(GLB::GLBFEATURE_sampler_object))
	{
		glSamplerParameteri(m_sampler_id,GL_TEXTURE_WRAP_S, texture_wrap_s);
		glSamplerParameteri(m_sampler_id,GL_TEXTURE_WRAP_T, texture_wrap_t);

		glSamplerParameteri(m_sampler_id,GL_TEXTURE_MIN_FILTER, texture_min_filter) ;
		glSamplerParameteri(m_sampler_id,GL_TEXTURE_MAG_FILTER, texture_mag_filter) ;

		if (texture_compare_mode != 0)
		{
			glSamplerParameteri(m_sampler_id,GL_TEXTURE_COMPARE_MODE,texture_compare_mode);
			glSamplerParameteri(m_sampler_id,GL_TEXTURE_COMPARE_FUNC,texture_compare_func);
		}
	}
#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	Bind();

	if( m == "simple projective")
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tboid, 0);
	}
	else if( m == "depth map(depth)")
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tboid, 0);
#ifdef __glew_h__
		glDrawBuffer( GL_NONE);
		glReadBuffer( GL_NONE);
#endif
	}
	else if( m == "depth map(color)")
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tboid, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboid);
	}
	
	Unbind();
}


ShadowMap::~ShadowMap()
{
	glDeleteTextures(1, &m_tboid);
	glDeleteFramebuffers(1, &m_fboid);
	if( m_rboid)
	{
		glDeleteRenderbuffers(1, &m_rboid);
	}

#if defined HAVE_GLES3 || defined __glew_h__
	if (GLB::g_extension && GLB::g_extension->hasFeature(GLB::GLBFEATURE_sampler_object))
	{
		glDeleteSamplers(1,&m_sampler_id) ;
	}
#endif
	
	delete m_blur;
}


void ShadowMap::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboid);
}
	
void ShadowMap::Unbind()
{
	if( m_clear_mask != GL_DEPTH_BUFFER_BIT)
	{
		DiscardDepthAttachment();
	}
	ApplyBlur();
	glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );
}


void ShadowMap::Clear()
{
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f);
	glClear( m_clear_mask);
}


void ShadowMap::BindShadowMap(unsigned int texture_unit_id)
{
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_unit_id);
	glBindTexture( GL_TEXTURE_2D, GetTextureId() );

#if defined HAVE_GLES3 || defined __glew_h__
	if (GLB::g_extension->hasFeature(GLB::GLBFEATURE_sampler_object))
	{
		glBindSampler(texture_unit_id, m_sampler_id );
	}
#endif
}

// TODO: Not sure its working with sampler objects 
void ShadowMap::ApplyBlur()
{
	if(m_blur == 0) return;
	
	OpenGLStateManager::GlUseProgram( m_blur->m_shader->m_p);
	
	OpenGLStateManager::GlActiveTexture( GL_TEXTURE0);
	glUniform1i(m_blur->m_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
	
	
	glBindBuffer( GL_ARRAY_BUFFER, m_blur->m_fullscreen_quad_vbo);
	OpenGLStateManager::GlEnableVertexAttribArray(m_blur->m_shader->m_attrib_locations[attribs::in_position]);
	OpenGLStateManager::Commit();
	glVertexAttribPointer(m_blur->m_shader->m_attrib_locations[attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_blur->m_aux_fbo[0]);
	glClear( GL_COLOR_BUFFER_BIT);
	glViewport( 0, 0, m_size, m_size);
	glBindTexture( GL_TEXTURE_2D, m_tboid);
	glUniform2f(m_blur->m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 1.0f / m_size, 0.0f);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
	
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_blur->m_aux_fbo[1]);
	glClear( GL_COLOR_BUFFER_BIT);
	glViewport( 0, 0, m_size, m_size);
	glBindTexture( GL_TEXTURE_2D, m_blur->m_aux_texture[0]);
	glUniform2f(m_blur->m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 0.0f, 1.0f / m_size);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
	
	OpenGLStateManager::GlDisableVertexAttribArray(m_blur->m_shader->m_attrib_locations[attribs::in_position]);
	glBindBuffer( GL_ARRAY_BUFFER, 0);
}


ShadowMap::Blur::Blur(uint32 &blur_fullscreen_quad_vbo, int size, bool is_immutable)
:
m_fullscreen_quad_vbo( blur_fullscreen_quad_vbo)
{
	glGenFramebuffers( 2, m_aux_fbo );
	glGenTextures( 2, m_aux_texture);

	for(int i=0; i<2; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_aux_texture[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		CreateTexture565( size, is_immutable);

		glBindTexture(GL_TEXTURE_2D, 0);


		glBindFramebuffer( GL_FRAMEBUFFER, m_aux_fbo[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_aux_texture[i], 0);
		FBO::bind( 0);

	}

    KCL::KCL_Status t;
	m_shader = Shader::CreateShader( "blur.vs", "blur.fs", 0, t);
}


ShadowMap::Blur::~Blur()
{
	glDeleteFramebuffers(2, m_aux_fbo );
	glDeleteTextures( 2, m_aux_texture);
}
