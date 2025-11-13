/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "instancedlightrenderer.h"
#include "platform.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include "kcl_camera2.h"
#include "opengl/gbuffer.h"
#include "glb_scene_.h"
#include "render_statistics_defines.h"
#include "opengl/shader.h"

using namespace GLB;

InstancedLightRenderer::InstancedLightRenderer()
{
    m_num_omni_lights = 0;
    m_num_spot_lights = 0;
	m_num_lightning_lights = 0;
    m_lightning_lights_instance_data_vbo = 0;

    m_instance_manager = NULL;
}


InstancedLightRenderer::~InstancedLightRenderer()
{
	glDeleteBuffers( 1, &m_lightning_lights_instance_data_vbo);

    delete m_instance_manager;
}



void InstancedLightRenderer::Init()
{
    m_instance_manager = new InstanceManager<_instanced_light>(MAX_LIGHTS * sizeof(_instanced_light));
    m_instance_manager->PreallocateBuffers(64);
		
	glGenBuffers( 1, &m_lightning_lights_instance_data_vbo);
	glBindBuffer( GL_UNIFORM_BUFFER, m_lightning_lights_instance_data_vbo);
	glBufferData( GL_UNIFORM_BUFFER, MAX_LIGHTS * sizeof(_instanced_light), 0, GL_DYNAMIC_COPY);
	glBindBuffer( GL_UNIFORM_BUFFER, 0);
    
    m_num_omni_lights = 0;
	m_num_spot_lights = 0;
	m_num_lightning_lights = 0;
}


KCL::uint32 InstancedLightRenderer::GetLightningInstanceDataVBO() const
{
	return m_lightning_lights_instance_data_vbo;
}


//TODO: LightBufferObject duplicated
struct LightBufferObject
{
	KCL::uint32 m_vbo;
	KCL::uint32 m_ebo;
	KCL::uint32 m_num_indices;
	KCL::uint32 m_vao;

	KCL::int32 m_ubo_handle;
	
	LightBufferObject()
	{
		m_vbo = 0 ;
		m_ebo = 0 ;
		m_num_indices = 0 ;
		m_vao = 0 ;
		m_ubo_handle = -1;
	}
};

void InstancedLightRenderer::Draw( KCL::Camera2 *camera, const PP &pp, LightBufferObject *m_lbos, Shader *m_lighting_shaders[16])
{
	int light_shader_index = 3;
	int light_buffer_index = -1;
	int num_lights;

	GLB::OpenGLStateManager::GlBlendFunc( 1, 1);
	GLB::OpenGLStateManager::GlEnable( GL_BLEND);

	
	#define DEPTH_TEST_LIGHTS

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_GEQUAL);
	    GLB::OpenGLStateManager::GlDepthMask(0);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#else
    {
        GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#endif

	KCL::uint32 input_textures[] = 
	{
		pp.m_color_map,
		pp.m_normal_map,
		pp.m_reflection_map,
		pp.m_depth_texture,
		pp.m_param_map
	};
    

    bool lightning_lights;
	for( int light_type=0; light_type<3; light_type++)
	{
        lightning_lights = false;
		if( light_type == 0)
		{
            //omni
            if (!m_num_omni_lights)
            {
                continue;
            }
            		
			light_shader_index = 1;
			light_buffer_index = 0;
			num_lights = m_num_omni_lights;
            			
            m_instance_manager->UploadInstanceData(m_omni_lights, m_num_omni_lights);
		}
		else if( light_type == 1)
		{
            //spot
            if (!m_num_spot_lights)
            {
                continue;
            }
            			
			light_shader_index = 2;
			light_buffer_index = 1;
			num_lights = m_num_spot_lights;

			m_instance_manager->UploadInstanceData(m_spot_lights, m_num_spot_lights);
		}
		else
		{
            //lightning omni
            if (!m_num_lightning_lights)
            {
                continue;
            }
            			
			light_shader_index = 1;
			light_buffer_index = 0;
			num_lights = m_num_lightning_lights;
            lightning_lights = true;
			glBindBufferBase( GL_UNIFORM_BUFFER, Shader::sUBOnames::LightInstancingConsts, m_lightning_lights_instance_data_vbo);
		}
		Shader *s = m_lighting_shaders[light_shader_index];

		glUseProgram( s->m_p);

		int j = 5;
		while( j--)
		{
			if (s->m_uniform_locations[GLB::uniforms::texture_unit0 + j] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + j);
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + j], j);
				glBindTexture( GL_TEXTURE_2D, input_textures[j]);
				glBindSampler(j, 0);
			}
		}

		GLB::OpenGLStateManager::Commit();

		glBindVertexArray( m_lbos[light_buffer_index].m_vao);

        if (lightning_lights)
        {
            glDrawElementsInstanced( GL_TRIANGLES, m_lbos[light_buffer_index].m_num_indices, GL_UNSIGNED_SHORT, 0, num_lights);
        }
        else
        {                
            KCL::uint32 instance_batch_size = 0;
            KCL::uint32 instance_offset = 0;
            while (num_lights > 0)
            {
                m_instance_manager->BindInstanceBuffer(Shader::sUBOnames::LightInstancingConsts, num_lights, instance_offset, instance_batch_size);

                if (s->m_uniform_locations[GLB::uniforms::instance_offset] > -1)
                {
                    glUniform1i(s->m_uniform_locations[GLB::uniforms::instance_offset], instance_offset);
                    glDrawElementsInstanced( GL_TRIANGLES, m_lbos[light_buffer_index].m_num_indices, GL_UNSIGNED_SHORT, 0, instance_batch_size);
                }
                num_lights -= instance_batch_size;
           }
       }
	}

	GLB::OpenGLStateManager::GlDisable( GL_BLEND);

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
        GLB::OpenGLStateManager::GlCullFace( GL_BACK);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_LESS);
	    GLB::OpenGLStateManager::GlDepthMask(1);
    }
#else
    {
        GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
    }
#endif

	glBindVertexArray( 0);
}

/*
void InstancedLightRenderer::Draw( KCL::Camera2 *camera, const PP &pp, LightBufferObject *m_lbos, Shader *m_lighting_shaders[16])
{
	assert(m_num_omni_lights <= MAX_LIGHTS);
	assert(m_num_spot_lights <= MAX_LIGHTS);
	assert(m_num_lightning_lights <= MAX_LIGHTNING_LIGHTS);

	glBindBuffer( GL_UNIFORM_BUFFER, m_omni_lights_instance_data_vbo);
	glBufferSubData( GL_UNIFORM_BUFFER, 0, m_num_omni_lights * sizeof(_instanced_light), m_omni_lights);
	glBindBuffer( GL_UNIFORM_BUFFER, 0);

	glBindBuffer( GL_UNIFORM_BUFFER, m_spot_lights_instance_data_vbo);
	glBufferSubData( GL_UNIFORM_BUFFER, 0, m_num_spot_lights * sizeof(_instanced_light), m_spot_lights);
	glBindBuffer( GL_UNIFORM_BUFFER, 0);

	int light_shader_index = 3;
	int light_buffer_index = -1;
	int num_lights;

	GLB::OpenGLStateManager::GlBlendFunc( 1, 1);
	GLB::OpenGLStateManager::GlEnable( GL_BLEND);

	
	#define DEPTH_TEST_LIGHTS

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_GEQUAL);
	    GLB::OpenGLStateManager::GlDepthMask(0);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#else
    {
        GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    }
#endif

	KCL::uint32 input_textures[] = 
	{
		pp.m_color_map,
		pp.m_normal_map,
		pp.m_reflection_map,
		pp.m_depth_texture,
		pp.m_param_map
	};
    
	for( int light_type=0; light_type<3; light_type++)
	{
		if( light_type == 0)
		{
			//omni
			light_shader_index = 1;
			light_buffer_index = 0;
			num_lights = m_num_omni_lights;

			glBindBufferBase( GL_UNIFORM_BUFFER, Shader::sUBOnames::LightInstancingConsts, m_omni_lights_instance_data_vbo);
		}
		else if( light_type == 1)
		{
			//spot
			light_shader_index = 2;
			light_buffer_index = 1;
			num_lights = m_num_spot_lights;

			glBindBufferBase( GL_UNIFORM_BUFFER, Shader::sUBOnames::LightInstancingConsts, m_spot_lights_instance_data_vbo);
		}
		else
		{
			//lightning omni
			light_shader_index = 1;
			light_buffer_index = 0;
			num_lights = m_num_lightning_lights;

			glBindBufferBase( GL_UNIFORM_BUFFER, Shader::sUBOnames::LightInstancingConsts, m_lightning_lights_instance_data_vbo);
		}
		Shader *s = m_lighting_shaders[light_shader_index];

		glUseProgram( s->m_p);

		int j = 5;
		while( j--)
		{
			if (s->m_uniform_locations[GLB::uniforms::texture_unit0 + j] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + j);
				glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + j], j);
				glBindTexture( GL_TEXTURE_2D, input_textures[j]);
				glBindSampler(j, 0);
			}
		}

		GLB::OpenGLStateManager::Commit();

		glBindVertexArray( m_lbos[light_buffer_index].m_vao);

		//s->Validateprogram();

		glDrawElementsInstanced( GL_TRIANGLES, m_lbos[light_buffer_index].m_num_indices, GL_UNSIGNED_SHORT, 0, num_lights);
	}

	GLB::OpenGLStateManager::GlDisable( GL_BLEND);

#ifdef DEPTH_TEST_LIGHTS
    {
	    GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
        GLB::OpenGLStateManager::GlCullFace( GL_BACK);
	    GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	    GLB::OpenGLStateManager::GlDepthFunc( GL_LESS);
	    GLB::OpenGLStateManager::GlDepthMask(1);
    }
#else
    {
        GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
    }
#endif

	glBindVertexArray( 0);
}
*/