/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vdashboard.h"
#include "opengl/glb_opengl_state_manager.h"


using namespace KCL;

struct Filter2
{
	Shader *m_shader;
	KCL::uint32 m_fbo;
	KCL::uint32 m_color_texture;
	KCL::uint32 m_depth_renderbuffer;
	KCL::uint32 m_width;
	KCL::uint32 m_height;
	int m_dir;
    KCL::uint32 m_max_mipmaps;
	bool m_is_mipmapped;
	KCL::Camera2 *m_active_camera;
	float m_focus_distance;
    bool m_onscreen;

	KCL::uint32 m_input_textures[7];

	~Filter2()
	{
		Clear();
	}

	void Clear()
	{
		GLenum e;
		glBindTexture(GL_TEXTURE_2D, 0);

		glDeleteTextures(1, &m_color_texture); 
		e = glGetError();

		glBindFramebuffer( GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers( 1, &m_fbo);

		e = glGetError();
	}

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
			glGenTextures( 1, &m_color_texture);

			glBindTexture( GL_TEXTURE_2D, m_color_texture);

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			glBindTexture( GL_TEXTURE_2D, 0);

			glGenFramebuffers( 1, &m_fbo);

			glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

            if(depth_attachment != 0)
            {
				glGenRenderbuffers( 1, &m_depth_renderbuffer);
				glBindRenderbuffer( GL_RENDERBUFFER, m_depth_renderbuffer);
				glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
				glBindRenderbuffer( GL_RENDERBUFFER, 0);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_renderbuffer);
            }

			glBindFramebuffer( GL_FRAMEBUFFER, 0);
		}
		else
		{
			m_fbo = 0;
		}
	}

	void Render()
	{
		GLB::OpenGLStateManager::GlUseProgram( m_shader->m_p);


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
			if (m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1)
			{
				GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + i);
				glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i], i);
				glBindTexture( GL_TEXTURE_2D, m_input_textures[i]);
			}
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::offset_2d] > -1)
		{
			if( m_dir)
			{
				glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 1.2f / m_width, 0.0f);
			}
			else
			{
				glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 0.0f, 1.2f / m_height);
			}
		}


		if (m_shader->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
		{
			glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_width, 1.0f / m_height);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::depth_parameters] > -1)
		{
			glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::camera_focus] > -1)
		{
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::camera_focus], m_focus_distance);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::dof_strength] > -1)
		{
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::dof_strength], m_active_camera->GetFov());
		}

		GLB::OpenGLStateManager::GlEnableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_position]);
		glVertexAttribPointer(m_shader->m_attrib_locations[GLB::attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);

		GLB::OpenGLStateManager::Commit();
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

		GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_position]);

#ifdef DEBUG
		if( m_is_mipmapped)
		{
			//printf("!warning: filter m_is_mipmapped, call glGenerateMipmap.\n");
		}
#endif
	}

	void Bind()
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_fbo);
		glViewport( 0, 0, m_width, m_height);
	}
};


KCL::KCL_Status VirtualDashboardScene::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
	IncrementProgressTo(0.5f);

	std::string required_render_api;

	required_render_api = "es2";

	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;

    Shader::SetForceHIGHP(m_force_highp);
    IncrementProgressTo(0.51f);

	Shader::InitShaders( required_render_api, true);
    IncrementProgressTo(0.52f);

    IncrementProgressTo(0.53f);

    IncrementProgressTo(0.54f);

    IncrementProgressTo(0.55f);

	{
		IncrementProgressTo(0.59f);
		for( uint32 i=0; i<m_rooms.size(); i++)
		{
			XRoom *room = m_rooms[i];
			for( uint32 j=0; j<room->m_meshes.size(); j++)
			{
				//NOTE: do not delete
				//room->m_meshes[j]->DeleteUnusedAttribs();
			}
		}
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.6f);

	for( uint32 i = 0; i < m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( uint32 i = 0; i < actor->m_meshes.size(); i++)
		{
			Mesh* m = actor->m_meshes[i];
			//NOTE: do not delete
			//m->DeleteUnusedAttribs();

			if( actor->m_name.find( "decal") == std::string::npos)
			{
				m->m_is_motion_blurred = true;
			}
		}

		for( KCL::uint32 j = 0; j < actor->m_emitters.size(); j++)
		{
			KCL::AnimatedEmitter *emitter = dynamic_cast<KCL::AnimatedEmitter*>(actor->m_emitters[j]);

			for( KCL::uint32 k = 0; k < MAX_MESH_PER_EMITTER; k++)
			{
				emitter->m_meshes[k].m_mesh = m_particle_geometry;

				//TODO: az emitter typehoz tartozo materialokat lekezelni
				switch( emitter->m_emitter_type)
				{
				case 3:
					{
						emitter->m_meshes[k].m_color.set( .1f, .105f, .12f);
						emitter->m_meshes[k].m_material = m_steamMaterial;
						break;
					}
				case 0:
				case 1:
				case 2:
				default:
					{
						emitter->m_meshes[k].m_color.set( 0.72f, 0.55f, 0.33f);
						emitter->m_meshes[k].m_material = m_smokeMaterial;
						break;
					}
				}
			}
		}

	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	for( uint32 i=0; i<m_sky_mesh.size(); i++)
	{
		m_sky_mesh[i]->DeleteUnusedAttribs();
	}

	for(size_t i = 0; i < m_meshes.size(); ++i)
	{
		m_meshes[i]->InitVertexAttribs();
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.7f);
	result = reloadShaders();
	if(result != KCL_TESTERROR_NOERROR)
	{
		return result;
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	for(size_t i = 0; i < m_materials.size(); ++i)
	{
		m_materials[i]->InitImages();
		if(result != KCL_TESTERROR_NOERROR)
		{
			return result;
		}

		switch (KCL_Status status = g_os->LoadingCallback(0))
		{
		case KCL_TESTERROR_NOERROR:
			break;
		default:
			return status;
		}

	}
    IncrementProgressTo(0.8f);
	
	m_filters = new Filter2[12];

	m_filters[0].Create( 1, m_viewport_width*2, m_viewport_height*2, false, 1, 0);
	m_filters[1].Create( 0, m_viewport_width, m_viewport_height, false, 1, 0);
	m_filters[10].Create( 0, m_viewport_width, m_viewport_height, true, 1, 0);

	m_filters[2].Create( 0, m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, 1, 0);
	m_filters[3].Create( 0, m_viewport_width / 2.0f, m_viewport_height / 2.0f, false, 1, 1);
	m_filters[4].Create( 0, m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, 1, 0);
	m_filters[5].Create( 0, m_viewport_width / 4.0f, m_viewport_height / 4.0f, false, 1, 1);
	m_filters[6].Create( 0, m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, 1, 0);
	m_filters[7].Create( 0, m_viewport_width / 8.0f, m_viewport_height / 8.0f, false, 1, 1);
	m_filters[8].Create( 0, m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, 1, 0);
	m_filters[9].Create( 0, m_viewport_width / 16.0f, m_viewport_height / 16.0f, false, 1, 1);

	if( !m_fullscreen_quad_vbo)
	{
		Vector4D v[4] = 
		{
			Vector4D( -1, -1, 0, 0),
			Vector4D( 1, -1, 1, 0),
			Vector4D( -1, 1, 0, 1),
			Vector4D( 1, 1, 1, 1)
		};

		glGenBuffers( 1, &m_fullscreen_quad_vbo);
		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
		glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), v[0].v, GL_STATIC_DRAW);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	{
		m_logo_texture = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/kishonti_logo_512.png", KCL::TC_Clamp | KCL::TC_NoMipmap);
		if (!m_logo_texture)
		{
			INFO("Error: missing texture %s", "common/kishonti_logo_512.png");
		}
		m_logo_shader = Shader::CreateShader( "logo.vs", "logo.fs", 0, result);

		float v[] =
		{
			0, 0, 0, 0,
			(float)m_logo_texture->getWidth(), 0, 1, 0,
			0, (float)m_logo_texture->getHeight(), 0, 1,
			(float)m_logo_texture->getWidth(), (float)m_logo_texture->getHeight(), 1, 1
		};

		glGenBuffers( 1, &m_logo_vbo);

		glBindBuffer( GL_ARRAY_BUFFER, m_logo_vbo);
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 16, v, GL_STATIC_DRAW);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
	}

	IncrementProgressTo(0.9f);

	return result;

}


VirtualDashboardScene::VirtualDashboardScene() : m_fullscreen_quad_vbo( 0), m_filters( 0), m_logo_texture( 0), m_logo_vbo( 0), m_logo_shader( 0)
{
	
}


VirtualDashboardScene::~VirtualDashboardScene()
{
	delete [] m_filters;

	if( m_fullscreen_quad_vbo)
	{
		glDeleteBuffers( 1, &m_fullscreen_quad_vbo);
	}
	if( m_logo_vbo)
	{
		glDeleteBuffers( 1, &m_logo_vbo);
	}

	delete m_logo_texture;
}


void VirtualDashboardScene::Render()
{
#ifdef LOG_SHADERS

#ifdef PER_FRAME_ALU_INFO
	Shader::Push_New_FrameAluInfo();
#endif

#endif

	//printf("%f, %f, %f\n", m_active_camera->GetEye().x, m_active_camera->GetEye().y, m_active_camera->GetEye().z);

	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;

    GLB::OpenGLStateManager::Reset();

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
    m_pixel_coverage_sampleCount = 0;
    m_pixel_coverage_primCount = 0;
#endif	

#ifdef TEXTURE_COUNTING_ERR // Error: KCL::Material has no method named TextureCounter
	m_textureCounter.clear();
	Material::TextureCounter(m_textureCounter);
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	m_num_samples_passed = 0;
#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	m_num_instruction = 0;
#endif
#endif

	m_filters[0].Bind();

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef OCCLUSION_QUERY_BASED_STAT
	//do_not_skip_samples_passed = true;
#endif

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = true;
#endif


	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

	GLB_Scene_ES2_::Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0, 0);
	GLB_Scene_ES2_::Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0, 0);
    
	glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);

	m_filters[1].m_shader = m_pp_shaders[1];
	m_filters[1].m_input_textures[0] = m_filters[0].m_color_texture;
	m_filters[1].Render();

	for( int i=2; i<=9; i++)
	{
		m_filters[i].m_shader = m_pp_shaders[2];
		m_filters[i].m_input_textures[0] = m_filters[i-1].m_color_texture;
		m_filters[i].Render();
	}

	m_filters[10].m_shader = m_pp_shaders[0];
	m_filters[10].m_input_textures[0] = m_filters[0].m_color_texture;
	m_filters[10].m_input_textures[1] = m_filters[9].m_color_texture;
	m_filters[10].Render();
	
	{
		KCL::Matrix4x4 m;

		KCL::Matrix4x4::Ortho( m, 0, m_viewport_width, m_viewport_height, 0, -1, 1);

		m.translate( KCL::Vector3D( m_viewport_width - m_logo_texture->getWidth() - 16, m_viewport_height - m_logo_texture->getHeight() - 16, 0));

		Shader *s = m_logo_shader;

		glEnable( GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram( s->m_p);

		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, 0, m.v);

		glBindBuffer( GL_ARRAY_BUFFER, m_logo_vbo);

		glEnableVertexAttribArray(m_logo_shader->m_attrib_locations[GLB::attribs::in_position]);
		glEnableVertexAttribArray(m_logo_shader->m_attrib_locations[GLB::attribs::in_texcoord0]);

		glVertexAttribPointer(m_logo_shader->m_attrib_locations[GLB::attribs::in_position], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
		glVertexAttribPointer(m_logo_shader->m_attrib_locations[GLB::attribs::in_texcoord0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

		glActiveTexture( GL_TEXTURE0);
		glBindTexture( GL_TEXTURE_2D, m_logo_texture->textureObject());
		glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

		glDisable( GL_BLEND);
	}



	glBindBuffer( GL_ARRAY_BUFFER, 0);

	Shader::m_last_shader = 0;

#ifdef TEXTURE_COUNTING_ERR // Error: KCL::Material has no method named NullTextureCounter
	Material::NullTextureCounter();
	m_num_used_texture = m_textureCounter.size();
#endif

	//printf(" %d, %d\n", m_num_draw_calls, m_num_triangles);
	//log_file = fopen( "log2.txt", "at");
	//fprintf( log_file, "%d, %d\n", m_animation_time, num_triangles);
	//fclose( log_file);
}

