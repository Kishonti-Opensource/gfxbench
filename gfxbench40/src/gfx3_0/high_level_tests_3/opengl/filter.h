/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef _FILTER_H
#define _FILTER_H

struct Filter
{
	Shader *m_shader;
	KCL::uint32 m_fbo;
	KCL::uint32 m_color_texture;
	KCL::uint32 m_width;
	KCL::uint32 m_height;
	int m_dir;
    KCL::uint32 m_max_mipmaps;
	bool m_is_mipmapped;
	KCL::Camera2 *m_active_camera;
	float m_focus_distance;
    bool m_onscreen;

	KCL::uint32 m_input_textures[8];

	Filter()
	{
		for( int i=0; i<8; i++)
		{
			m_input_textures[i] = 0;
		}
	}

	void Clear()
	{
		GLenum e;
		glBindTexture(GL_TEXTURE_2D, 0);

		glDeleteTextures(1, &m_color_texture); 
		e = glGetError();

		GLB::FBO::bind(0);
		glDeleteFramebuffers( 1, &m_fbo);

		e = glGetError();
	}

    //maxmipcount: 0 means complete mipchain
    void Create( KCL::uint32 depth_attachment, KCL::uint32 w, KCL::uint32 h, bool onscreen, KCL::uint32 maxmipcount, int dir)
	{
        m_max_mipmaps = maxmipcount;
		m_is_mipmapped = (maxmipcount == 0) || (maxmipcount > 1);

		m_dir = dir;
		m_shader = 0;

		m_width = w;
		m_height = h;

        m_onscreen = onscreen;

		if( !onscreen)
		{
			m_color_texture = Create2DTexture( maxmipcount, true, m_width, m_height, GL_RGBA8);

			glGenFramebuffers( 1, &m_fbo);

			glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

            if(depth_attachment != 0)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);
            }

			FBO::bind(0);
		}
		else
		{
			m_fbo = 0;
		}
	}

#if defined OCCLUSION_QUERY_BASED_STAT
	void Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction, GLSamplesPassedQuery *glGLSamplesPassedQuery)
#else
	void Render( KCL::uint32 &num_draw_calls, KCL::uint32 &num_triangles , KCL::uint32 &num_vertices , std::set<KCL::uint32> &textureCounter , KCL::int32 &num_samples_passed , KCL::int32 &num_instruction)
#endif
	{
		GLB::OpenGLStateManager::GlUseProgram( m_shader->m_p);


		if(!m_fbo) FBO::bind(NULL); else
			glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);

        if (m_onscreen)
        {
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else
        {
            glClear( GL_COLOR_BUFFER_BIT);
        }

		glViewport( 0, 0, m_width, m_height);

		int i = 8;
		while( i--)
		{
			if( m_shader->m_uniform_locations[100 + i] > -1 && m_input_textures[i])
			{
				GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + i);
				glUniform1i( m_shader->m_uniform_locations[100 + i], i);
				glBindTexture( GL_TEXTURE_2D, m_input_textures[i]);
                glBindSampler(i, 0);
				
#ifdef TEXTURE_COUNTING
				textureCounter.insert( m_input_textures[i] );
#endif
			}
		}

		if( m_shader->m_uniform_locations[22] > -1)
		{
			if( m_dir)
			{
				glUniform2f( m_shader->m_uniform_locations[22], 1.0f / m_width, 0.0f);
			}
			else
			{
				glUniform2f( m_shader->m_uniform_locations[22], 0.0f, 1.0f / m_height);
			}
		}


		if( m_shader->m_uniform_locations[23] > -1)
		{
			glUniform2f( m_shader->m_uniform_locations[23], 1.0f / m_width, 1.0f / m_height);
		}

		if( m_shader->m_uniform_locations[69] > -1 && m_active_camera)
		{
			glUniform4fv( m_shader->m_uniform_locations[69], 1, m_active_camera->m_depth_linearize_factors.v);
		}

		if( m_shader->m_uniform_locations[64] > -1)
		{
			glUniform1f( m_shader->m_uniform_locations[64], m_focus_distance);
		}

		if( m_shader->m_uniform_locations[70] > -1 && m_active_camera)
		{
			glUniform1f( m_shader->m_uniform_locations[70], m_active_camera->GetFov());
		}

		if( !VAO_enabled)
		{
			GLB::OpenGLStateManager::GlEnableVertexAttribArray( m_shader->m_attrib_locations[0]);
			glVertexAttribPointer( m_shader->m_attrib_locations[0], 2, GL_FLOAT, GL_FALSE, sizeof( Vector4D), 0);
		}

		GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
		glGLSamplesPassedQuery->Begin();
#endif
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

		if( !VAO_enabled)
		{
			GLB::OpenGLStateManager::GlDisableVertexAttribArray( m_shader->m_attrib_locations[0]);
		}

		num_draw_calls++;
		num_triangles += 2;
		num_vertices += 4;

#ifdef OCCLUSION_QUERY_BASED_STAT
		glGLSamplesPassedQuery->End();
		num_samples_passed += glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
		num_instruction += m_shader->m_instruction_count_v * 4 + m_shader->m_instruction_count_f * glGLSamplesPassedQuery->Result();
#endif

#endif

#ifdef DEBUG
		if( m_is_mipmapped)
		{
			//printf("!warning: filter m_is_mipmapped, call glGenerateMipmap.\n");
		}
#endif
	}
};

#endif