/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_lensflare.h"

#include <iostream>
#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/fbo.h"
#include "kcl_io.h"
#include "ng/json.h"
#include "ubo_bindings.h"
#include "opengl/glb_discard_functions.h"

using namespace KCL ;

#define OFFSET_BUFFER_BIND 1

namespace GLB
{

Lensflare::Lensflare()
{
	m_compute_oc = NULL ;
	m_flares = NULL ;
	m_lens_dirt = NULL ;

	m_offset_count = 128 ;
	m_flare_count = 0 ;

    m_color_texture = -1;
}


Lensflare::~Lensflare()
{
	glDeleteSamplers(1,&m_depth_sampler) ;
	glDeleteBuffers(1,&m_atomic_counter) ;

	glDeleteBuffers(1,&m_lensflare_vbo) ;
	glDeleteBuffers(1,&m_offsets_buffer) ;

	glDeleteVertexArrays(1,&m_lensflare_vao) ;

	glDeleteFramebuffers(1,&m_lensflare_fbo) ;

	delete m_lens_dirt ;
	
	for ( std::map<std::string,GLB::GLBTexture* >::iterator it = m_texture_pool.begin() ; it != m_texture_pool.end() ; it++)
	{
		delete it->second ;
	}

	delete[] m_flares ;
}


void Lensflare::Init(int width, int height) 
{
	m_viewport_width = width ;
	m_viewport_height = height ;

	m_offset_count = 128 ;


	//
	//	Load config file
	//
	GLB::GLBTextureFactory texture_factory;

	std::string base_dir = "common/" ;
	bool has_error = false ;
	
	std::string lensflare_cfg_str ;

	if (KCL::AssetFile::Exists(base_dir+"lensflare.json"))
	{
		KCL::AssetFile lensflare_cfg_file(base_dir+"lensflare.json") ;
		
		lensflare_cfg_str = lensflare_cfg_file.GetBuffer() ;
	}
	else
	{
		has_error = true ;
		INFO("ERROR: lensflare config file not found") ;
	}


	ng::JsonValue lensflare_config ;
	if ( !has_error)
	{
		ng::Result res;
		
		lensflare_config.fromString(lensflare_cfg_str.c_str(),res) ;

		if (!res.ok())
		{
			has_error = true ;
			INFO("ERROR: unable to parse lensflare config file") ;
		}
	}


	if (!has_error) 
	{
		double distance_scale = lensflare_config["distance_scale"].number() ;
		double size_scale     = lensflare_config["size_scale"].number() ;

		ng::JsonValue flares_json = lensflare_config["flares"] ;
		m_flare_count = flares_json.size() ;
		m_flares = new Flare[m_flare_count] ;

		bool sundir_valid = lensflare_config["sun_dir_x"].isNumber() && lensflare_config["sun_dir_y"].isNumber() && lensflare_config["sun_dir_z"].isNumber() ;
		if ( sundir_valid )
		{
			m_light_dir.x = lensflare_config["sun_dir_x"].number() ;
			m_light_dir.y = lensflare_config["sun_dir_y"].number() ;
			m_light_dir.z = lensflare_config["sun_dir_z"].number() ;

			m_light_dir.normalize() ;
		}
		else
		{
			INFO("ERROR: sundir not set in lensflare config, lensflare disabled") ;
			has_error = true ;
		}

		for (int i = 0 ; i < m_flare_count ; i++)
		{
			m_flares[i].distance = distance_scale*flares_json[i]["distance"].number() ;
			m_flares[i].size = size_scale*flares_json[i]["size"].number() ;

			std::string texture_name = base_dir + flares_json[i]["texture"].string() ;

			if ( m_texture_pool.find(texture_name) == m_texture_pool.end() )
			{
				GLBTexture* t = (GLBTexture*)texture_factory.CreateAndSetup(KCL::Texture_2D, texture_name.c_str(), KCL::TC_Clamp | KCL::TC_NoMipmap);
				m_texture_pool[texture_name] = t ;
			}

			m_flares[i].texture = m_texture_pool[texture_name] ;
		}
	
		m_lens_dirt = (GLBTexture*)texture_factory.CreateAndSetup(KCL::Texture_2D, (base_dir + "lensdirt.png").c_str(), KCL::TC_Clamp | KCL::TC_NoMipmap);
	}


	if (has_error)
	{
		m_flare_count = 0 ;
	}
	

	//
	//	occlusion shader build shader 
	//
	GLBShaderBuilder sb ;

	KCL::KCL_Status result ;

	sb.FileCs("lensflare_occlusion.shader") ;

	sb.AddDefine("COLOR_TEX_TYPE rgba16f") ;
	sb.AddDefine("COLOR_TEX_BIND 0") ;

	sb.AddDefineInt("WORK_GROUP_SIZE",m_offset_count) ;

	sb.AddDefineInt("VIEWPORT_WIDTH", width) ;
	sb.AddDefineInt("VIEWPORT_HEIGHT", height) ;
	sb.AddDefineInt("OFFSET_BUFFER_BIND", OFFSET_BUFFER_BIND) ;

	m_compute_oc = sb.Build(result) ;


	//
	//	render shader
	//
	sb.ShaderFile("lensflare.shader") ;

	sb.AddDefineInt("VIEWPORT_WIDTH", width) ;
	sb.AddDefineInt("VIEWPORT_HEIGHT", height) ;
	sb.AddDefineInt("MAX_SAMPLE", m_offset_count) ;

	m_lensflare_shader = sb.Build(result) ;


	//
	//	Generate depth sampler
	//
	glGenSamplers(1,&m_depth_sampler) ;

	glSamplerParameteri(m_depth_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_depth_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(m_depth_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(m_depth_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//
	//	Create atomic buffer
	//
	glGenBuffers(1,&m_atomic_counter) ;
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER,m_atomic_counter) ;
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 1, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);


	//
	//	VBO, VAO
	//
#define LENSFLARE_VBO_DATA_COUNT 16
	float v[LENSFLARE_VBO_DATA_COUNT] =
		{
			-1.0f, -1.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f, 1.0f,
			 1.0f,  1.0f, 1.0f, 1.0f
		};

	glGenBuffers( 1, &m_lensflare_vbo);

	glBindBuffer( GL_ARRAY_BUFFER, m_lensflare_vbo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(float) * LENSFLARE_VBO_DATA_COUNT, v, GL_STATIC_DRAW);
	glBindBuffer( GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_lensflare_vao);

	glBindVertexArray( m_lensflare_vao);

	glBindBuffer( GL_ARRAY_BUFFER, m_lensflare_vbo);

	glVertexAttribPointer( m_lensflare_shader->m_attrib_locations[GLB::attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
	glVertexAttribPointer(m_lensflare_shader->m_attrib_locations[GLB::attribs::in_texcoord0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	glEnableVertexAttribArray(m_lensflare_shader->m_attrib_locations[GLB::attribs::in_position]);
	glEnableVertexAttribArray(m_lensflare_shader->m_attrib_locations[GLB::attribs::in_texcoord0]);

	glBindVertexArray( 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0);


	//
	//	FBO
	//
	glGenFramebuffers(1, &m_lensflare_fbo) ;
	glBindFramebuffer(GL_FRAMEBUFFER,m_lensflare_fbo) ;
    FBO::bind(0) ;


	//
	//	Generate offsets
	//
	int offset_seed = 54643  ;
	std::vector<KCL::Vector2D> offsets ;
	offsets.resize(m_offset_count) ;
	
	for (int i = 0 ; i < m_offset_count ; i++)
	{
		float t = 2.0 * Math::kPi * Math::randomf( &offset_seed ) ; 
		float u = Math::randomf( &offset_seed ) + Math::randomf( &offset_seed ) ;
		float r = ( u > 1.0 ) ? 2.0 - u : u ;

		offsets[i].x = r * cos(t) ;
		offsets[i].y = r * sin(t) * m_viewport_width / m_viewport_height ;

		//printf("%f - %f\n", offsets[i].x, offsets[i].y) ;
	}


	glGenBuffers(1, &m_offsets_buffer) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,m_offsets_buffer) ;
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_offset_count*sizeof(KCL::Vector2D), offsets[0].v, GL_DYNAMIC_DRAW ) ;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,0) ;

}


void Lensflare::Execute()
{	
	//
	//	calculate sun position
	//
	m_screenspace_sun_pos = m_camera->GetViewProjection()*KCL::Vector4D(m_light_dir,0.0f) ;
	float t_w = m_screenspace_sun_pos.w ;
	m_screenspace_sun_pos /= m_screenspace_sun_pos.w ;
	
	KCL::Vector4D normalized_screenspace_sun_pos = m_screenspace_sun_pos ;
	normalized_screenspace_sun_pos.x = (m_screenspace_sun_pos.x + 1.0) / 2.0 ;
	normalized_screenspace_sun_pos.y = (m_screenspace_sun_pos.y + 1.0) / 2.0 ;
	normalized_screenspace_sun_pos.w = t_w ;


	//
	//	clear atomic counter
	//
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomic_counter);
	GLuint a[1] = {0};
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0 , sizeof(GLuint) * 1, a);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	
	//
	//	Dispatch Compute
	// 
	GLB::OpenGLStateManager::GlUseProgram(m_compute_oc->m_p) ;
	glUniform4fv(glGetUniformLocation(m_compute_oc->m_p,"sun_pos"),1,normalized_screenspace_sun_pos.v) ;

	if (m_color_texture != -1)
	{
		glBindImageTextureProc(0,m_color_texture,0,GL_TRUE,0,GL_WRITE_ONLY,GL_RGBA16F) ;
	}

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER,0,m_atomic_counter) ;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_offsets_buffer) ;
	
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0) ;
	glBindSampler(0,m_depth_sampler) ;
	glBindTexture(GL_TEXTURE_2D,m_depth_texture) ;

	glDispatchComputeProc(1,1,1) ;


#if 0
	{
		//
		//	readback atomic counter
		//

		glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT) ;

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomic_counter);
		glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 1, a);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

		printf("visibility: %d\n",a[0]) ;
	}
#endif
	
	glBindSampler(0,0) ;
}


void Lensflare::Render()
{
	if (m_flare_count == 0) return ;

    /*
	if (m_color_texture != -1)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_lensflare_fbo) ;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);
	}
	else
	{
		FBO::bind(0) ;
	}
    */

	OpenGLStateManager::GlEnable(GL_BLEND) ;
	//OpenGLStateManager::GlDisable(GL_BLEND) ;
	OpenGLStateManager::GlBlendFunc( GL_ONE,  GL_ONE);
	OpenGLStateManager::Commit() ;

	
	float aspect_ratio = ((float)m_viewport_width) / m_viewport_height;

	KCL::Matrix4x4 m;
	KCL::Matrix4x4::Ortho( m, -aspect_ratio, aspect_ratio, -1, 1, -1, 1);

	GLB::OpenGLStateManager::GlUseProgram( m_lensflare_shader->m_p);
	glBindBufferBase(GL_UNIFORM_BUFFER,LENSFLARE_BINDING_SLOT,m_atomic_counter) ;

	GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE1);
	glBindTexture( GL_TEXTURE_2D,m_lens_dirt->textureObject());

	glUniform2fv(glGetUniformLocation( m_lensflare_shader->m_p,"sun_dir"),1,m_screenspace_sun_pos.v) ;
	glUniformMatrix4fv(m_lensflare_shader->m_uniform_locations[GLB::uniforms::mvp], 1, 0, m.v);
	glBindVertexArray( m_lensflare_vao);

	for (int i = 0 ; i < m_flare_count ; i++)
	{
		GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0);
		glBindTexture( GL_TEXTURE_2D, m_flares[i].texture->textureObject());

		glUniform1f(glGetUniformLocation( m_lensflare_shader->m_p,"flare_distance"),m_flares[i].distance) ;
		glUniform1f(glGetUniformLocation( m_lensflare_shader->m_p,"flare_size"),m_flares[i].size) ;

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

	}

	glBindVertexArray(0) ;
	glBindBufferBase(GL_UNIFORM_BUFFER,LENSFLARE_BINDING_SLOT,0) ;

	OpenGLStateManager::GlDisable(GL_BLEND);

//	FBO::bind(0) ;
}


}