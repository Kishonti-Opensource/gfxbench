/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_filter.h"

#include "kcl_os.h"
#include "kcl_camera2.h"

#include "platform.h"
#include "opengl/fbo.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_texture.h"
#include "opengl/misc2_opengl.h"
#include "glb_shader2.h"
#include "opengl/glb_discard_functions.h"
#include "glb_scene_.h"

using namespace GLB;

#define VAO_enabled GLB::g_extension->hasFeature(GLB::GLBFEATURE_vertex_array_object)

GLBFilter::GLBFilter()
{
	SetDefaults();
}

GLBFilter::~GLBFilter()
{
	ReleaseResources();
}

void GLBFilter::SetDefaults()
{
	// Clean up the member variables
	for (KCL::uint32 i = 0; i < MAX_INPUT_TEXTURES; i++)
	{
		m_input_textures[i] = 0;
		m_input_samplers[i] = 0;
	}

	for (KCL::uint32 i = 0; i < MAX_INPUT_DEPTH_TEXTURES; i++)
	{
		m_input_depth_textures[i] = 0;
	}

	for (KCL::uint32 i = 0; i < MAX_INPUT_IMAGE_TEXTURES; i++)
	{
		m_input_image_textures[i] = -1 ;
	}
	
	m_shader = 0;
	m_scene = NULL;

	m_color_texture = 0;
	m_dir = 0;
	m_mipmap_count = 0;
	m_is_mipmapped = 0;
	m_active_camera = NULL;
	m_focus_distance = 0;

	m_clear_bitmask = 0;
	m_clear_to_color = false;
		
	m_compute_group_x = 1;
	m_compute_group_y = 1;
	m_compute_group_z = 1;

	m_compute_out_tex = 0;
	m_onscreen = true;

	m_gauss_lod_level = 0;
	m_render_target_level = 0;

	m_shadow_texture = 0;
	m_shadow_matrices = NULL;
	m_shadow_sampler = 0;

    m_ssao_projection_scale = 0.0f;

	m_static_cubemaps_object = 0;

	for(KCL::uint32 i = 0; i < COUNT_OF(m_dynamic_cubemap_objects); i++)
	{
		m_dynamic_cubemap_objects[i] = 0;
	}

	m_reconstructPosInWS = false;
	
	m_fbos.clear();
	m_lod_dims.clear();

#if STATISTICS_LOGGING_ENABLED
	m_scene = NULL;
#endif
}

void GLBFilter::ReleaseResources()
{
	glDeleteTextures( 1, &m_color_texture);
	if(m_fbos.size())
	{
		assert(m_fbos.data());
		glDeleteFramebuffers( m_fbos.size(), m_fbos.data());	
	}

	m_color_texture = 0;
	m_fbos.clear();
}

void GLBFilter::Init(KCL::uint32 vao, KCL::uint32 vbo, KCL::uint32 depth_attachment, KCL::uint32 width, KCL::uint32 height, bool onscreen, KCL::uint32 maxmipcount, int dir, KCL::uint32 internal_format)
{
	ReleaseResources();
	SetDefaults();

	m_vao = vao;
	m_vbo = vbo;
	m_internal_format = internal_format ;

	// Get the actual desired mipmap count
	m_mipmap_count = std::max((KCL::uint32)1, maxmipcount);
	m_mipmap_count = std::min(KCL::uint32(KCL::texture_levels(width, height)), m_mipmap_count);	
	CalcLodDimensions(width, height, m_mipmap_count);

	m_is_mipmapped = m_mipmap_count > 1;
	
	m_dir = dir;

	m_width = width;
	m_height = height;

	m_onscreen = onscreen;

	if(!m_onscreen)
	{
		CreateRenderTarget(width, height, true, m_internal_format, depth_attachment);
		FBO::bind(0);
		m_clear_bitmask = GL_COLOR_BUFFER_BIT;
	}
	else 
	{
		m_clear_bitmask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	}
}

#if defined OCCLUSION_QUERY_BASED_STAT
void GLBFilter::Render(GLSamplesPassedQuery *glGLSamplesPassedQuery)
#else
void GLBFilter::Render()
#endif
{
	KCL::int32 shadow_sampler_binding_point = -1;

	OpenGLStateManager::GlUseProgram(m_shader->m_p);

	KCL::int32 texture_num = 0;
	for (KCL::uint32 i = 0; i < MAX_INPUT_TEXTURES; i++)
	{
		if ((m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1) && m_input_textures[i])
		{
			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i], texture_num);
			glBindTexture(GL_TEXTURE_2D, m_input_textures[i]);
#ifdef TEXTURE_COUNTING
			m_scene->m_textureCounter.insert( m_input_textures[i]);
#endif
			glBindSampler(texture_num, m_input_samplers[i]);
			++texture_num;
		}
	}

	for (KCL::uint32 i = 0; i < MAX_INPUT_DEPTH_TEXTURES; i++)
	{
		if ((m_shader->m_uniform_locations[GLB::uniforms::depth_unit0 + i] > -1) && m_input_depth_textures[i])
		{
			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::depth_unit0 + i], texture_num);
			glBindTexture(GL_TEXTURE_2D, m_input_depth_textures[i]);
#ifdef TEXTURE_COUNTING
			m_scene->m_textureCounter.insert( m_input_depth_textures[i]);
#endif
			glBindSampler(texture_num, 0);
			++texture_num;
		}
	}

	for (KCL::uint32 i = 0; i < MAX_INPUT_IMAGE_TEXTURES; i++)
	{
		if (m_input_image_textures[i] != -1)
		{
			glBindImageTextureProc(i, m_input_image_textures[i], 0, GL_FALSE, 0, GL_READ_ONLY, m_internal_format);
		}
	}

	if (m_shader->IsComputeShader())
	{
#ifdef GL_COMPUTE_SHADER
		glBindImageTextureProc(1, m_compute_out_tex, 0,  GL_FALSE, 0, GL_WRITE_ONLY, m_internal_format);

#ifdef TEXTURE_COUNTING
		m_scene->m_textureCounter.insert(m_compute_out_tex);
#endif

		// GLB::OpenGLStateManager::Commit();
		glDispatchComputeProc(m_compute_group_x, m_compute_group_y, m_compute_group_z);
#endif
	}
	else // Fragment shader filter
	{
		if(m_onscreen)
		{
			FBO::bind(NULL);
			glViewport(0, 0, m_width, m_height);
		}
		else
		{
			glBindFramebuffer( GL_FRAMEBUFFER, m_fbos[m_render_target_level]);           
			glViewport(0, 0, m_lod_dims[m_render_target_level].x, m_lod_dims[m_render_target_level].y);
		}	
	   
		if (m_shader->m_uniform_locations[GLB::uniforms::global_light_dir] > -1 && m_scene)
		{
			glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::global_light_dir], 1, m_scene->m_light_dir.v);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::global_light_color] > -1 && m_scene)
		{
			glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::global_light_color], 1, m_scene->m_light_color.v);
		}

		if ((m_shader->m_uniform_locations[GLB::uniforms::light_pos] > -1) && m_scene) //hack GI for garage by faking reflected light with a point light
		{
			if (m_scene->m_visible_lights.size())
			{
				KCL::Light* l = m_scene->m_visible_lights[0];
				Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

				glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::light_pos], 1, light_pos.v);
			}
		}

		if ((m_shader->m_uniform_locations[GLB::uniforms::light_color] > -1) && m_scene)
		{
			if (m_scene->m_visible_lights.size())
			{
				KCL::Light* l = m_scene->m_visible_lights[0];
				float i = 1.0f;
				if( l->m_intensity_track)
				{
					Vector4D v;

					l->t = m_scene->m_animation_time / 1000.0f;

					_key_node::Get( v, l->m_intensity_track, l->t, l->tb, false);

				   i = v.x;// / l->m_intensity;
				}

				glUniform3f(m_shader->m_uniform_locations[GLB::uniforms::light_color],
					l->m_diffuse_color.x * i,
					l->m_diffuse_color.y * i,
					l->m_diffuse_color.z * i
					);
			}
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::offset_2d] > -1)
		{
			if( m_dir)
			{
				glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 1.0f / m_width, 0.0f);
			}
			else
			{
				glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::offset_2d], 0.0f, 1.0f / m_height);
			}
		}


		if (m_shader->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
		{
			glUniform2f(m_shader->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_width, 1.0f / m_height);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::depth_parameters] > -1 && m_active_camera)
		{           
			glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::depth_parameters], 1, m_active_camera->m_depth_linearize_factors.v);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::camera_focus] > -1)
		{
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::camera_focus], m_focus_distance);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::camera_focus_inv] > -1)
		{
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::camera_focus_inv], 1.0f / m_focus_distance);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::dof_strength] > -1 && m_active_camera)
		{            
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::dof_strength], m_active_camera->GetFov());
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::dof_strength_inv] > -1 && m_active_camera)
		{            
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::dof_strength_inv], 1.0f / (m_active_camera->GetFov() * 0.1f));
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::view_pos] > -1 && m_active_camera)
		{
			glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::view_pos], 1, m_active_camera->GetEye().v);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::corners] > -1 && m_active_camera)
		{            
			KCL::Vector3D v[4];
			if(m_reconstructPosInWS)
			{
				m_active_camera->CalculateRaysToFullscreenBillboard( v);
			}
			else
			{
				m_active_camera->CalculateRaysToFullscreenBillboard( v, true);
			}
			glUniform3fv(m_shader->m_uniform_locations[GLB::uniforms::corners], 4, v->v);
		}

		//dynamic cubemaps for cars
		if (m_shader->m_uniform_locations[GLB::uniforms::envmap1] > -1)
		{
			//Get2Closest( pos, em0, em1, envmaps_interpolator);
			//glUniform1f( s->m_uniform_locations[18], envmaps_interpolator);

			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_dynamic_cubemap_objects[0]); //uses texparameter, not samplers for now

			glBindSampler(texture_num, 0);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::envmap2] > -1)
		{
			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_dynamic_cubemap_objects[1]); //uses texparameter, not samplers for now
			
			glBindSampler(texture_num, 0);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::envmap2], texture_num++);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::envmap1_dp] > -1)
		{
			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_dynamic_cubemap_objects[0]); //uses texparameter, not samplers for now

			glBindSampler(texture_num, 0);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::envmap1_dp], texture_num++);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::envmap2_dp] > -1)
		{
			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_dynamic_cubemap_objects[1]); //uses texparameter, not samplers for now
			
			glBindSampler(texture_num, 0);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::envmap2_dp], texture_num++);
		}
		
		if (m_shader->m_uniform_locations[GLB::uniforms::static_envmaps] > -1)
		{
			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_static_cubemaps_object);

			glBindSampler(texture_num, 0);
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::static_envmaps], texture_num++);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::inv_view] > -1 && m_active_camera)
		{           
			GLB::Matrix4x4 inv_view;
			GLB::Matrix4x4 v = m_active_camera->GetView();
			GLB::Matrix4x4::InvertModelView(v, inv_view);
			glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::inv_view], 1, GL_FALSE, inv_view.v);
		}	
	

		if( m_shader->m_uniform_locations[GLB::uniforms::dpcam_view] > -1 && m_scene)
		{
			glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::dpcam_view], 1, GL_FALSE, m_scene->m_dpcam.GetView().v);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::cascaded_shadow_texture_array] > -1)
		{	
			OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadow_texture);	
			glBindSampler(texture_num, m_shadow_sampler);
			shadow_sampler_binding_point = texture_num;
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::cascaded_shadow_texture_array], texture_num++);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::carindex_translucency_ssaostr_fovscale] > -1)
		{
			KCL::Vector4D carindex_translucency_ssaostr_fovscale = KCL::Vector4D(0.0, 0.0, 0.0, 0.0); //reads SSAO map

			/*
			if( (m_scene->m_animation_time < 20929) && (m_scene->m_animation_time > 18816) )
			{
				carindex_translucency_ssaostr_fovscale.z = 1.0; //turned off
			}
			*/

			glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::carindex_translucency_ssaostr_fovscale], 1, carindex_translucency_ssaostr_fovscale.v);
		}
		 
		// cascaded_shadow_matrices
		if (m_shader->m_uniform_locations[GLB::uniforms::cascaded_shadow_matrices] > -1 && m_shadow_matrices)
		{
			KCL::uint32 cascade_count = 4;
			glUniformMatrix4fv(m_shader->m_uniform_locations[GLB::uniforms::cascaded_shadow_matrices], cascade_count, false, m_shadow_matrices[0].v);
		}

		// cascaded_frustum_distances
		if (m_shader->m_uniform_locations[GLB::uniforms::cascaded_frustum_distances] > -1)
		{
			glUniform4fv(m_shader->m_uniform_locations[GLB::uniforms::cascaded_frustum_distances], 1, m_shadow_distances.v);
		}	

		if (m_shader->m_uniform_locations[GLB::uniforms::gauss_lod_level] > -1)
		{
			glUniform1i(m_shader->m_uniform_locations[GLB::uniforms::gauss_lod_level], m_gauss_lod_level);
		}

		if (m_shader->m_uniform_locations[GLB::uniforms::projection_scale] > -1)
		{
			glUniform1f(m_shader->m_uniform_locations[GLB::uniforms::projection_scale], m_ssao_projection_scale);
		}
		
		if(!VAO_enabled)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			GLB::OpenGLStateManager::GlEnableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_position]);
			GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_bone_weight]);
			glVertexAttribPointer(m_shader->m_attrib_locations[GLB::attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);
			glVertexAttribPointer(m_shader->m_attrib_locations[GLB::attribs::in_bone_weight], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), (GLvoid *)sizeof(KCL::Vector2D));
		}
		else
		{
			glBindVertexArray(m_vao);
		}	

		if (m_clear_bitmask)
		{
			if (m_clear_to_color)
			{
				glClearBufferfv(GL_COLOR, 0, m_clear_color.v);
			}
			else
			{
				glClear(m_clear_bitmask);
			}
		}
			
		GLB::OpenGLStateManager::Commit();	

#ifdef OCCLUSION_QUERY_BASED_STAT
		glGLSamplesPassedQuery->Begin();
#endif
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

#if STATISTICS_LOGGING_ENABLED
		assert(m_scene);
		m_scene->m_num_triangles += 2;
		m_scene->m_num_vertices += 4;
		m_scene->m_num_draw_calls++;
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
		glGLSamplesPassedQuery->End();
		m_scene->m_num_samples_passed += glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
		m_scene->m_num_instruction += m_shader->m_instruction_count_v * 4 + m_shader->m_instruction_count_f * glGLSamplesPassedQuery->Result();
#endif

#endif
				
		if(!VAO_enabled)
		{
			GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_position]);
			GLB::OpenGLStateManager::GlDisableVertexAttribArray(m_shader->m_attrib_locations[GLB::attribs::in_bone_weight]);
			GLB::OpenGLStateManager::Commit();
		}
		else 
		{
			glBindVertexArray(0);
		}

		if (m_onscreen)
		{
			// Uncomment to discard the onscreen filters. 
			//FBO::InvalidateGlobalDepthAttachment() ;
		}
	}

	for (unsigned int i = 0; i < MAX_INPUT_TEXTURES; i++)
	{
		if (m_shader->m_uniform_locations[GLB::uniforms::texture_unit0 + i] > -1)
		{
			glBindSampler(i, 0);
		}
	}
	if (shadow_sampler_binding_point > -1)
	{
		glBindSampler(shadow_sampler_binding_point, 0);
	}
}

void GLBFilter::CreateRenderTarget(KCL::uint32 width, KCL::uint32 height, bool linear, KCL::int32 format, KCL::uint32 depth_attachment)
{
	// Create the color target		
	glGenTextures(1, &m_color_texture);
	glBindTexture( GL_TEXTURE_2D, m_color_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if(linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_is_mipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_is_mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
	}

#if defined HAVE_GLES3 || defined HAVE_GLEW
	glTexStorage2D(GL_TEXTURE_2D, m_mipmap_count, format, width, height);
#else
	// Set the base level
	glTexImage2D (GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		
	if (m_is_mipmapped)
	{
		// Sets the index of the lowest and highest defined mipmap level.                     
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_mipmap_count - 1);
		// Generate the other levels
		glGenerateMipmap(GL_TEXTURE_2D);
	}
#endif
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create the framebuffer objects
	for (KCL::uint32 i = 0; i < m_mipmap_count; i++)
	{
		KCL::uint32 fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, i);

		if(depth_attachment != 0)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, i);
		}
		m_fbos.push_back(fbo);
	}
   
}

KCL::uint32 GLBFilter::Width() const
{
	return m_width;
}

KCL::uint32 GLBFilter::Height() const
{
	return m_height;
}

KCL::uint32 GLBFilter::GetFramebufferObject(KCL::uint32 lod_level) const
{
	return m_fbos[lod_level];
}

void GLBFilter::SetClearColor(const KCL::Vector4D &color)
{
	m_clear_to_color = true;
	m_clear_color = color;
}

void GLBFilter::CalcLodDimensions(KCL::uint32 width, KCL::uint32 height, KCL::uint32 lod_count)
{
	m_lod_dims.clear();
	KCL::uint32 w = 0;
	KCL::uint32 h = 0;
	KCL::uint32 k = 1;
	while (w != 1 && h != 1)
	{
		w = KCL::Max(width / k, 1u);
		h = KCL::Max(height / k, 1u);        
		k *= 2;

		m_lod_dims.push_back(KCL::Vector2D(w, h));
		if (m_lod_dims.size() == lod_count)
		{
			return;
		}
	}
}
