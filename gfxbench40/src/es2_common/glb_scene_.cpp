/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_scene_.h"
#include "opengl/shader.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/vbopool.h"
#include "platform.h"

//#define NOT_USE_GFXB

#ifdef NOT_USE_GFXB
#pragma message("GLB_Scene_ES2_::Render using KRL")
#include "krl_material.h"
#define RENDER_MATERIAL KRL::Material
#else
#pragma message("GLB_Scene_ES2_::Render using GLB")
#include "opengl/glb_material.h"
#define RENDER_MATERIAL GLB::Material
#endif
#include "opengl/misc2_opengl.h"

using namespace KCL;

bool do_not_skip_samples_passed = true;

void GLB_Scene_ES2_::InitFactories()
{
#ifndef NOT_USE_GFXB
	m_animatedFactory = new KCL::AnimatedEmitterFactory();
	m_tf_emitterFactory = new GLB::TF_emitterFactory();
	m_lightFactory = new GLB::GLBLightFactory();
	m_materialFactory = new GLB::MaterialFactory(m_scene_version);

	m_factory.RegisterFactory(m_animatedFactory, KCL::EMITTER1);
	m_factory.RegisterFactory(m_tf_emitterFactory, KCL::EMITTER2);
	m_factory.RegisterFactory(m_lightFactory, KCL::LIGHT);
	m_factory.RegisterFactory(m_materialFactory, KCL::MATERIAL);
#endif
}


GLB_Scene_ES2_::GLB_Scene_ES2_()
{
	m_animatedFactory = 0;
	m_tf_emitterFactory = 0;
	m_lightFactory = 0;
	m_materialFactory = 0;
#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery = 0;
#endif

	m_measurePixelCoverage = false;
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE

	m_coverageFile = fopen("gfx3_stat.csv", "at");
#endif

}


GLB_Scene_ES2_::~GLB_Scene_ES2_()
{
	delete m_animatedFactory;
	delete m_tf_emitterFactory;
	delete m_lightFactory;
	delete m_materialFactory;
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	fclose(m_coverageFile);
#endif
}


void GLB_Scene_ES2_::Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
{
	RENDER_MATERIAL *override_material = dynamic_cast<RENDER_MATERIAL*>(_override_material);
	RENDER_MATERIAL *last_material = NULL;
	int last_mesh_type = -1;
	KCL::uint32 texture_num_from_material = 0;

	float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

#ifdef PER_FRAME_ALU_INFO
	uint32 samplesPassed = 0;
#endif

	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
#ifdef PER_FRAME_ALU_INFO
		samplesPassed = 0;
		glBeginQuery(GL_SAMPLES_PASSED, Shader::QueryId());
#endif

		KCL::Mesh* sm = (KCL::Mesh*)visible_meshes[j];

		if( !sm->m_mesh)
		{
			continue;
		}

		int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

		RENDER_MATERIAL *material = dynamic_cast<RENDER_MATERIAL*>(sm->m_material);

		if( override_material)
		{
			material = override_material;
		}

		/* force use of shader[1][?] for depth prepass */
		int shader_bank = (pass_type == -1) ? 1 : 0;

		Shader *s = material->m_shaders[shader_bank][mesh_type];
		KRL_CubeEnvMap *envmaps[2];
		float envmaps_interpolator = 0.0f;
		KCL::Matrix4x4 mvp;
		KCL::Matrix4x4 mv;
		KCL::Matrix4x4 model;
		KCL::Matrix4x4 inv_model;
		Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);


		if( last_material != material || last_mesh_type != mesh_type)
		{
			if( last_material)
			{
				last_material->postInit();
			}

			texture_num_from_material = 0;


#ifndef NOT_USE_GFXB
			if( material->m_ogg_decoder)
			{
				if( material->m_frame_when_animated != m_frame)
				{
					material->PlayVideo( m_animation_time * m_animation_multiplier / 1000.0f);
					material->m_frame_when_animated = m_frame;
				}
			}
#endif

			material->preInit( texture_num_from_material, mesh_type, pass_type);
		}

		KCL::uint32 texture_num = texture_num_from_material;

		last_material = material;

		last_mesh_type = mesh_type;

		switch( mesh_type)
		{
		case 0:
			{
				if( material->m_material_type == KCL::Material::SKY)
				{
					mvp = sm->m_world_pom * camera->GetViewProjectionOrigo();
				}
				else
				{
					mvp = sm->m_world_pom * camera->GetViewProjection();
				}
				mv = sm->m_world_pom * camera->GetView();
				model = sm->m_world_pom;
				inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
				break;
			}
		case 1:
			{
				mvp = camera->GetViewProjection();
				mv = camera->GetView();
				model.identity();
				inv_model.identity();
				break;
			}
		}

		glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, mvp.v);


		if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
		{
			glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, mv.v);
		}


		if (s->m_uniform_locations[GLB::uniforms::bones] > -1 && sm->m_mesh->m_node_matrices.size())
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::bones], 3 * sm->m_mesh->m_nodes.size(), sm->m_mesh->m_node_matrices.data());
		}

		if (s->m_uniform_locations[GLB::uniforms::model] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::model], 1, GL_FALSE, model.v);
		}

		if( s->m_uniform_locations[GLB::uniforms::inv_model] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_model], 1, GL_FALSE, inv_model.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::global_light_dir] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::global_light_dir], 1, m_light_dir.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::global_light_color] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::global_light_color], 1, m_light_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
		{
			Vector3D view_dir( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]);
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, camera->GetEye().v);
		}

		if (s->m_uniform_locations[GLB::uniforms::time] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::time], normalized_time);
		}

		if (s->m_uniform_locations[GLB::uniforms::background_color] > -1)
		{
			glUniform3fv( s->m_uniform_locations[GLB::uniforms::background_color], 1, m_background_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::fog_density] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::fog_density], m_fog_density);
		}

		if (s->m_uniform_locations[GLB::uniforms::diffuse_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::diffuse_intensity], material->m_diffuse_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::specular_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::specular_intensity], material->m_specular_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::reflect_intensity] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::reflect_intensity], material->m_reflect_intensity);
		}

		if (s->m_uniform_locations[GLB::uniforms::specular_exponent] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::specular_exponent], material->m_specular_exponent);
		}

		if (s->m_uniform_locations[GLB::uniforms::transparency] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::transparency], material->m_transparency);
		}

		if (s->m_uniform_locations[GLB::uniforms::envmaps_interpolator] > -1 && m_cubemaps.size())
		{
			Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);

			glUniform1f( s->m_uniform_locations[GLB::uniforms::envmaps_interpolator], envmaps_interpolator);

			envmaps[0]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap0], texture_num++);

			envmaps[1]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( ((GLB::GLBTexture*)envmaps[0]->GetTexture())->textureObject() );
			m_textureCounter.insert( ((GLB::GLBTexture*)envmaps[1]->GetTexture())->textureObject() );
#endif
		}

		if (s->m_uniform_locations[GLB::uniforms::light_pos] > -1 && light)
		{
			glUniform3fv( s->m_uniform_locations[GLB::uniforms::light_pos], 1, &light->m_world_pom.v[12]);
		}

		if (s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1 && m_global_shadowmaps[0])
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m_global_shadowmaps[0]->m_matrix.v);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId() );
			glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], texture_num++);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( m_global_shadowmaps[0]->GetTextureId() );
#endif
		}
		if (s->m_uniform_locations[GLB::uniforms::shadow_matrix1] > -1 && m_global_shadowmaps[1])
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix1], 1, GL_FALSE, m_global_shadowmaps[1]->m_matrix.v);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[1]->GetTextureId() );
			glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit1], texture_num++);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( m_global_shadowmaps[1]->GetTextureId() );
#endif
		}

		if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
		{
			glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
		}

		if (s->m_uniform_locations[GLB::uniforms::color] > -1)
		{
			glUniform3fv(s->m_uniform_locations[GLB::uniforms::color], 1, sm->m_color.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::translate_uv] > -1)
		{
			if( sm->m_material->m_frame_when_animated != m_frame)
			{
                //if the test has looped, reset animation offsets
                if(sm->m_material->m_animation_time > m_animation_time / 1000.0f)
                {
                    sm->m_material->m_animation_time_base = 0.0f;
                }

				sm->m_material->m_animation_time = m_animation_time / 1000.0f;

				if( sm->m_material->m_translate_u_track)
				{
					Vector4D r;

					KCL::_key_node::Get( r, sm->m_material->m_translate_u_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

					sm->m_material->m_uv_offset.x = -r.x;
				}
				if( sm->m_material->m_translate_v_track)
				{
					Vector4D r;

					KCL::_key_node::Get( r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

					sm->m_material->m_uv_offset.y = -r.x;
				}
				sm->m_material->m_frame_when_animated = m_frame;
			}

			glUniform2fv(s->m_uniform_locations[GLB::uniforms::translate_uv], 1, sm->m_material->m_uv_offset.v);
		}

		if (s->m_uniform_locations[GLB::uniforms::particle_data] > -1)
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_data], MAX_PARTICLE_PER_MESH, m_particle_data[sm->m_offset1].v);
		}
		if (s->m_uniform_locations[GLB::uniforms::particle_color] > -1)
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_color], MAX_PARTICLE_PER_MESH, m_particle_color[sm->m_offset1].v);
		}
		if (s->m_uniform_locations[GLB::uniforms::alpha_threshold] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::alpha_threshold], sm->m_alpha);
		}

		if (s->m_uniform_locations[GLB::uniforms::light_color] > -1 && light)
		{
			glUniform3fv( s->m_uniform_locations[GLB::uniforms::light_color], 1, light->m_diffuse_color.v);
		}
		if (s->m_uniform_locations[GLB::uniforms::world_fit_matrix] > -1)
		{
			glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::world_fit_matrix], 1, GL_FALSE, m_world_fit_matrix.v);
		}

#ifndef NOT_USE_GFXB
		if (override_material != 0 && override_material->m_planar_map != 0 && s->m_uniform_locations[GLB::uniforms::planar_reflection] > -1)
		{
			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
			glBindTexture( GL_TEXTURE_2D, override_material->m_planar_map->GetTextureId() );
			glUniform1i(s->m_uniform_locations[GLB::uniforms::planar_reflection], texture_num++);

			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
			m_textureCounter.insert( override_material->m_planar_map->GetTextureId() );
#endif
		}
#endif

		if (s->m_uniform_locations[GLB::uniforms::mvp2] > -1)
		{
			switch( mesh_type)
			{
			case 0:
				{
					KCL::Matrix4x4 mvp2;

					mvp2 = sm->m_prev_world_pom * m_prev_vp;

					glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, mvp2.v);

					break;
				}
			case 1:
				{
					glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, m_prev_vp.v);
					break;
				}
			}
		}

		if (s->m_uniform_locations[GLB::uniforms::prev_bones] > -1 && sm->m_mesh->m_prev_node_matrices.size())
		{
			glUniform4fv(s->m_uniform_locations[GLB::uniforms::prev_bones], 3 * m_max_joint_num_per_mesh, sm->m_mesh->m_prev_node_matrices.data());
		}

		if (s->m_uniform_locations[GLB::uniforms::mblur_mask] > -1)
		{
			glUniform1f(s->m_uniform_locations[GLB::uniforms::mblur_mask], sm->m_is_motion_blurred);
		}


#ifdef USE_VBO
		VboPool::Instance()->BindBuffer(sm->m_mesh->m_vbo);
		IndexBufferPool::Instance()->BindBuffer(sm->m_mesh->m_ebo[lod].m_buffer);
#endif

		for( uint32 l=0; l<16; l++)
		{
			if( s->m_attrib_locations[l] > -1 && sm->m_mesh->m_vertex_attribs[l].m_size != 0)
			{
				GLB::OpenGLStateManager::GlEnableVertexAttribArrayInstantCommit( s->m_attrib_locations[l]);

				glVertexAttribPointer(
					s->m_attrib_locations[l],
					sm->m_mesh->m_vertex_attribs[l].m_size,
					sm->m_mesh->m_vertex_attribs[l].m_type,
					sm->m_mesh->m_vertex_attribs[l].m_normalized,
					sm->m_mesh->m_vertex_attribs[l].m_stride,
					sm->m_mesh->m_vertex_attribs[l].m_data
					);
			}
		}

		GLB::OpenGLStateManager::Commit();

#ifdef OCCLUSION_QUERY_BASED_STAT
		if(do_not_skip_samples_passed || m_measurePixelCoverage)
		{
            m_glGLSamplesPassedQuery->Begin();
        }
#endif

#ifdef USE_VBO
		glDrawElements(GL_TRIANGLES, sm->m_primitive_count ? sm->m_primitive_count : sm->m_mesh->getIndexCount(lod), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset);
#else
		glDrawElements(GL_TRIANGLES, sm->getIndexCount(lod), GL_UNSIGNED_SHORT, &sm->m_vertex_indices[0]);
#endif

		m_num_draw_calls++;
		m_num_triangles += sm->m_mesh->getIndexCount(lod) / 3;
		m_num_vertices += sm->m_mesh->getIndexCount(lod);

#ifdef OCCLUSION_QUERY_BASED_STAT
		if(do_not_skip_samples_passed || m_measurePixelCoverage)
		{
			m_glGLSamplesPassedQuery->End();
            size_t sampleCount = m_glGLSamplesPassedQuery->Result();
			m_num_samples_passed += sampleCount;

    #ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
			m_num_instruction += s->m_instruction_count_v * sm->m_mesh->getIndexCount(lod) + s->m_instruction_count_f * sampleCount;
    #endif
    #ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
            if(m_measurePixelCoverage && sampleCount)
		    {
                KCL::uint32 primCount = sm->m_mesh->getIndexCount(lod) / 3 / 2 * 1; //1 is instancecount

		        float vis = float(sampleCount) / float(primCount);
                fprintf( m_coverageFile, "%.0f;%s;%d;%d;%.1f\n", m_animation_time * m_animation_multiplier, sm->m_name.c_str(), primCount, sampleCount, vis);

                m_pixel_coverage_sampleCount += sampleCount;
                m_pixel_coverage_primCount += primCount;
		    }
     #endif
		}
#endif

		for( uint32 l=0; l<16; l++)
		{
			if( s->m_attrib_locations[l] > -1)
			{
				GLB::OpenGLStateManager::GlDisableVertexAttribArray( s->m_attrib_locations[l]);
			}
		}

#ifdef PER_FRAME_ALU_INFO
		glEndQuery(GL_SAMPLES_PASSED);

		uint32 ready=0;
		while( !ready)
		{
			glGetQueryObjectuiv( Shader::QueryId(), GL_QUERY_RESULT_AVAILABLE, &ready);
		}

		glGetQueryObjectuiv( Shader::QueryId(), GL_QUERY_RESULT, &samplesPassed);

		Shader::Add_FrameAluInfo(s, samplesPassed, sm->m_mesh->m_vertex_indices[lod].size() / 3);
#endif

	}

	if(last_material)
	{
		last_material->postInit();
	}

#ifdef USE_VBO
	VboPool::Instance()->BindBuffer(0);
	IndexBufferPool::Instance()->BindBuffer(0);
#endif

}


void GLB_Scene_ES2_::CreateVBOPool()
{
#ifdef ENABLE_FRAME_CAPTURE
	VboPool::Instance()->SetCapacity(1);
	IndexBufferPool::Instance()->SetCapacity(1);
#else
	VboPool::Instance()->SetCapacity(1024 * 1024);
	IndexBufferPool::Instance()->SetCapacity(1024 * 1024);
#endif
}


void GLB_Scene_ES2_::DeleteVBOPool()
{
	VboPool::DeleteInstance();
	IndexBufferPool::DeleteInstance();
}


void GLB_Scene_ES2_::DeleteShaders()
{
	Shader::DeleteShaders();
}


KCL::KCL_Status GLB_Scene_ES2_::reloadShaders()
{
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;

	char max_joint_num_per_mesh_str[64];

	sprintf( max_joint_num_per_mesh_str, " %d ", m_max_joint_num_per_mesh);

	std::set< std::string> def;
	def.insert("DEP_TEXTURING");	/* force mediump */

	if( m_mblur_enabled  && (m_scene_version == KCL::SV_27) )
	{
		m_blur_shader = Shader::CreateShader( "pp.vs", "mblur_final.fs", &def, result);
	}

	for(size_t i=0; i<m_materials.size(); ++i)
	{
		std::string s;

		if( m_shadow_method_str == "depth map(depth)")
		{
			s += "shadow_depth_map_depth ";
		}
		else if( m_shadow_method_str == "depth map(color)")
		{
			s += "shadow_depth_map_color ";
		}
		else if( m_shadow_method_str == "simple projective")
		{
			s += "shadow_simple_projective ";
		}
		if( m_soft_shadow_enabled)
		{
			s += "soft_shadow ";
		}
		s += GetVersionStr() + " ";

		result = dynamic_cast<KRL::Material*>(m_materials[i])->InitShaders( s.c_str(), max_joint_num_per_mesh_str);

		if(result != KCL_TESTERROR_NOERROR)
		{
			INFO("Can not init material: %s", m_materials[i]->m_name.c_str());
			return result;
		}
	}

	if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)
	{
		std::string lighting_fs_shader;
		if (m_test_id == "gl_manhattan311_wqhd_off" || 
			m_test_id == "gl_manhattan311_fixed_wqhd_off")
		{
			lighting_fs_shader = "lighting_wqhd.fs";
		}
		else
		{
			lighting_fs_shader = "lighting.fs";
		}

		std::set< std::string> default_defines;
		default_defines.insert( m_scene_version == KCL::SV_30 ? "SV_30" : "SV_31");

		std::set< std::string> defines = default_defines;

		defines.insert( "SPECIAL_DIFFUSE_CLAMP");
		defines.insert( "NEED_HIGHP");
		m_lighting_shaders[0] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[10] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);

		defines = default_defines;

		defines.insert( "POINT_LIGHT");
		defines.insert( "NEED_HIGHP");
		if( m_scene_version == KCL::SV_31)
		{
			defines.insert( "INSTANCING");
		}
		m_lighting_shaders[1] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[11] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);

		defines = default_defines;

		defines.insert( "SPOT_LIGHT");
		defines.insert( "NEED_HIGHP");
		if( m_scene_version == KCL::SV_31)
		{
			defines.insert( "INSTANCING");
		}
		m_lighting_shaders[2] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[12] = Shader::CreateShader("lighting.vs", lighting_fs_shader.c_str(), &defines, result);

		m_lighting_shaders[3] = Shader::CreateShader( "lighting.vs", "small_light.fs", &defines, result);
		m_lighting_shaders[13] = Shader::CreateShader( "lighting.vs", "small_light.fs", &defines, result);

		defines = default_defines;

		//m_lighting_shaders[4] = Shader::CreateShader( "lighting.vs", "ssao.fs", 0, result);

		m_lighting_shaders[15] = Shader::CreateShader( "lighting.vs", "shadow_decal.fs", 0, result);

		defines = default_defines;
		defines.insert( "NEED_HIGHP");

		// Not used in 3.x
		//m_lighting_shader = Shader::CreateShader( "lighting.vs", "lighting2.fs", &defines, result);
		m_lighting_shader = 0;

		defines = default_defines;

		m_pp_shaders[2] = Shader::CreateShader( "pp.vs", "sub.fs", &defines, result);
		m_pp_shaders[1] = Shader::CreateShader( "color_blur.vs", "color_blur.fs", &defines, result);
		m_pp_shaders[0] = Shader::CreateShader( "pp.vs", "pp.fs", &defines, result);

		m_reflection_emission_shader = Shader::CreateShader( "pp.vs", "re.fs", &defines, result);

		m_hud_shader = Shader::CreateShader( "hud.vs", "hud.fs", &defines, result);

		m_occlusion_query_shader = Shader::CreateShader( "oc.vs", "oc.fs", &defines, result);

		m_fog_shader = Shader::CreateShader( "fog.vs", "fog.fs", &defines, result);

		m_camera_fog_shader = Shader::CreateShader( "fog.vs", "fog_camera.fs", &defines, result);

#if !defined ENABLE_FRAME_CAPTURE
		m_particleAdvect_shader = Shader::CreateShader( "particleAdvect.vs", "particleAdvect.fs", &defines, result, Shader::ShaderTypes::TransformFeedback);
#else
#pragma message("No particleAdvect shader when ENABLE_FRAME_CAPTURE is set!")
#endif
	}
	if( m_scene_version == KCL::SV_VDB)
	{
		std::set< std::string> defines;

		m_pp_shaders[0] = Shader::CreateShader( "pp.vs", "scene+glow.fs", &defines, result);
		m_pp_shaders[1] = Shader::CreateShader( "pp.vs", "glow_filter.fs", &defines, result);
		m_pp_shaders[2] = Shader::CreateShader( "color_blur.vs", "color_blur.fs", &defines, result);
	}

	return result;
}
