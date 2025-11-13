/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform.h"

#include "glb_scene_opengl31.h"
#include "opengl/glb_texture.h"

#if defined HAVE_GLES3 || defined HAVE_GLEW

#include <kcl_planarmap.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_animation4.h>
#include "opengl/glb_particlesystem.h"

#include "glb_kcl_adapter.h"
#include "opengl/glb_image.h"
#include "glb_mesh.h"
#include "opengl/glb_material.h"
#include "opengl/glb_light.h"

#include "opengl/shader.h"
#include "platform.h"
#include "opengl/cubemap.h"
#include "opengl/shadowmap.h"
#include "opengl/vbopool.h"

#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"
#include "opengl/glbshader.h"
#include "opengl/texture.h"
#include "opengl/vbopool.h"

#include "misc2.h"
#include "kcl_io.h"

#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_discard_functions.h"
#include "compute_lightning.h"

#include <cmath>
#include <cstddef>
#include <algorithm> //std::sort

#include "instancedlightrenderer.h"
#include "opengl/fragment_blur.h"
#include "opengl/compute_blur.h"
#include "compute_lightning.h"
#include "compute_hdr31.h"

#include "opengl/ubo_cpp_defines.h"
#include "ubo_luminance.h"

#include "gui_interface.h"

#include "dummy_gui.h"
#include "property.h"
#ifdef HAVE_GUI_FOLDER
#include "gfxgui31.h"
#endif

using namespace GLB;

#if defined MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE || defined OCCLUSION_QUERY_BASED_STAT
extern bool do_not_skip_samples_passed;
#endif

const bool occlusion_query_enable = true;
const bool ssao_enabled = false;

GLB_Scene_ES31::GLB_Scene_ES31()
{
    m_last_animation_time = -1;

	m_quad_vao = 0;
	m_quad_vbo = 0;

	m_InstancedLightRenderer = NULL;
    m_dof_blur = NULL;
	m_compute_lightning = NULL;
	m_compute_hdr = NULL;

	m_ubo_manager = NULL;
}

GLB_Scene_ES31::~GLB_Scene_ES31()
{
	delete m_InstancedLightRenderer;
    delete m_dof_blur;
	delete m_compute_lightning;
	delete m_compute_hdr;

	GLBShader2::DeleteShaders();

	glDeleteVertexArrays(1, &m_quad_vao);
	glDeleteBuffers(1, &m_quad_vbo);

	delete m_ubo_manager;
}

bool GLB_Scene_ES31::UseEnvmapMipmaps()
{
	return true;
}

KCL::KCL_Status GLB_Scene_ES31::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{	
	GLBShader2::InitShaders(KCL::SV_31, m_force_highp);

#ifdef UBO31
	m_ubo_manager = new UBOManager();
	m_skip_create_shadow_decal = true;

	float radius = 1000.0f * 1.25f;
	m_shadow_decal_light_matrix.scale(Vector3D(radius, radius, radius));
#endif	

    PropertyLoader prr;
	std::string path = "manhattan31/scene_31.prop";
	std::vector<SerializeEntry> entries = prr.DeSerialize(m_ubo_frame, path);	

#ifdef HAVE_GUI_FOLDER
	m_gui = std::unique_ptr<GFXGui31>(new GFXGui31(*this));
#else
	m_gui = std::unique_ptr<DummyGUI>(new DummyGUI());
#endif

	m_gui->Init();

	// TODO: Create full screen quad class
	// Create a for the filter quad
	Vector4D v[4] = 
	{
		Vector4D( -1, -1, 0, 1),
		Vector4D( 1, -1, 1, 1),
		Vector4D( -1, 1, 0, 0),
		Vector4D( 1, 1, 1, 0)
	};
				
	// Create and upload the vertex buffer
	glGenVertexArrays(1, &m_quad_vao);
	glBindVertexArray(m_quad_vao);

	glGenBuffers(1, &m_quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector4D), v[0].v, GL_STATIC_DRAW);
		
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), (GLvoid *)sizeof(Vector2D));			
		
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLB_Scene_ES3::Process_GL(color_mode, depth_mode, samples);

#ifdef UBO31
	// Precache the scene
	m_ubo_manager->Precache(this);
#endif
	
	return KCL::KCL_TESTERROR_NOERROR;
}

KCL::KCL_Status GLB_Scene_ES31::CreateBuffers()
{
	KCL::KCL_Status ret = GLB_Scene_ES3::CreateBuffers() ;

	if (ret != KCL::KCL_TESTERROR_NOERROR)
	{
		return ret ;
	}

	filters[10]->Clear() ;
	if(m_disabledRenderBits & RenderBits::ERB_Post)
    {
        filters[10]->Create( pp.m_depth_texture, m_viewport_width, m_viewport_height, true, 1, 0, true);
    }
    else //default behaviour
    {
        filters[10]->Create( pp.m_depth_texture, m_viewport_width, m_viewport_height, false, 1, 0, true);
	
		m_pp2_filter.Init( m_quad_vao, m_quad_vbo, 0, m_viewport_width, m_viewport_height, true, 1, 0, GL_RGBA8);

		m_hdr_filter.Init( m_quad_vao, m_quad_vbo, 0, m_viewport_width, m_viewport_height, false, 1, 0, GL_RGBA8);
    }

	return KCL::KCL_TESTERROR_NOERROR ;
}

void GLB_Scene_ES31::DeleteBuffers()
{
	GLB_Scene_ES3::DeleteBuffers();
}

void GLB_Scene_ES31::DoLightingPass()
{
if(!(m_disabledRenderBits & RenderBits::ERB_Lighting))
{
	m_InstancedLightRenderer->m_num_lightning_lights = m_compute_lightning->GetLightCount();
	m_InstancedLightRenderer->m_num_omni_lights = 0;
	m_InstancedLightRenderer->m_num_spot_lights = 0;
	
	for( size_t i=0; i<m_visible_lights.size(); i++)
	{
		KCL::Light *l = m_visible_lights[i];
		float fov = KCL::Math::Rad( l->m_spotAngle);

		InstancedLightRenderer::_instanced_light *cl;
		
		if( l->m_light_type == KCL::Light::OMNI)
		{
			cl = &m_InstancedLightRenderer->m_omni_lights[m_InstancedLightRenderer->m_num_omni_lights];
			m_InstancedLightRenderer->m_num_omni_lights++;

			cl->model.identity();
			cl->model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
			cl->model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );

		}
		else
		{
			cl = &m_InstancedLightRenderer->m_spot_lights[m_InstancedLightRenderer->m_num_spot_lights];
			m_InstancedLightRenderer->m_num_spot_lights++;

			float halfSpotAngle = Math::Rad(0.5f * l->m_spotAngle); 

			float scalingFactorX = KCL::Vector3D(l->m_world_pom.v[0], l->m_world_pom.v[1], l->m_world_pom.v[2]).length();
			float scalingFactorY = KCL::Vector3D(l->m_world_pom.v[4], l->m_world_pom.v[5], l->m_world_pom.v[6]).length();
			float scalingFactorZ = KCL::Vector3D(l->m_world_pom.v[8], l->m_world_pom.v[9], l->m_world_pom.v[10]).length();

			assert(fabs(scalingFactorX - scalingFactorY) < 0.001f);
			assert(fabs(scalingFactorY - scalingFactorZ) < 0.001f);
			assert(fabs(scalingFactorX - scalingFactorZ) < 0.001f);

			cl->model.zero();
			cl->model.v33 = l->m_radius * (1.0f / scalingFactorZ);
			cl->model.v11 = cl->model.v22 = cl->model.v33 * tanf(halfSpotAngle) * 1.2f; //1.2 is: extra opening to counter low tess-factor of the cone
			cl->model.v43 = -cl->model.v33;	// Translate so the top is at the origo
			cl->model.v44 = 1;
			cl->model *= l->m_world_pom;
		}
		

		cl->color.set( l->m_diffuse_color.x, l->m_diffuse_color.y, l->m_diffuse_color.z, 1.0f);
		cl->position.set( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], (l->m_light_type == KCL::Light::OMNI ? 0 : 1));
		cl->atten_parameters.set( -1.0f / (l->m_radius * l->m_radius), 0,0,0);
		cl->dir.set( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10], l->m_world_pom.v[14]);
		cl->spot.set( cosf( fov * 0.5), 1.0f / ( 1.0f - cosf( fov * 0.5f)), 1, 1);
	}

	pp.BindFinalBuffer();
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	glClear( GL_COLOR_BUFFER_BIT);

    m_ubo_manager->BindCamera(UBOManager::Camera::NORMAL);
	m_InstancedLightRenderer->Draw( m_active_camera, pp, m_lbos, m_lighting_shaders);
}
else
{
	pp.BindFinalBuffer();
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	glClear( GL_COLOR_BUFFER_BIT);
}
}

void GLB_Scene_ES31::RunEffect(GLB_Scene_ES3::Effect effect)
{
	switch(effect)
	{
	case BlurEffect:
	{
		if ( !(m_disabledRenderBits & RenderBits::ERB_Compute_DOF) )
        {        
            m_dof_blur->SetInputTexture( m_hdr_filter.m_color_texture);
            m_dof_blur->Execute();
		}
		break;
	}

	case LightningEffect:
	{
		if(!(m_disabledRenderBits & RenderBits::ERB_Compute_Lightning))
		{
			for(unsigned int i=0; i<m_actors.size(); ++i)
			{
				if(m_actors[i]->m_name.find("robot") != std::string::npos)
				{             
					m_compute_lightning->Run(m_animation_time,  m_actors[i]);
					break;
				}
			}
		}
		break;
	}

	default:
		break;
	}
}

void GLB_Scene_ES31::RenderEffect(GLB_Scene_ES3::Effect effect)
{
	switch(effect)
	{
	    case BlurEffect:
	    {
		    //pp with compute dof
		    m_pp2_filter.m_active_camera = m_active_camera;
		    m_pp2_filter.m_focus_distance = m_camera_focus_distance;
		    m_pp2_filter.m_shader = m_pp2;
		    m_pp2_filter.m_input_textures[0] = m_hdr_filter.m_color_texture;
		    //m_pp2_filter.m_input_textures[1] = 0;
		    m_pp2_filter.m_input_textures[2] = pp.m_depth_texture;
            m_pp2_filter.m_input_textures[3] = m_dof_blur->GetOutputTexture();
            m_pp2_filter.m_scene = this;
    #ifdef OCCLUSION_QUERY_BASED_STAT
            m_pp2_filter.Render(m_glGLSamplesPassedQuery);
    #else
            m_pp2_filter.Render();
    #endif
		    break;
	    }

	    case LightningEffect:
	    {
		    GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
		    GLB::OpenGLStateManager::Commit();

		    m_compute_lightning->Draw(m_active_camera);
		    break;
	    }

	    case PostEffect:
	    {
		    m_compute_hdr->SetInputTexture( filters[10]->m_color_texture); //GL_RGB10_A2 is fed to the compute shader
            m_compute_hdr->Execute();

            glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, m_compute_hdr->GetLuminanceBuffer());

		    m_hdr_filter.m_scene = this;
		    m_hdr_filter.m_input_textures[0] = filters[10]->m_color_texture; //GL_RGB10_A2
		    m_hdr_filter.m_input_textures[1] = pp.m_depth_texture;
            m_hdr_filter.m_input_textures[2] = m_compute_hdr->GetBloomTexture(); //GL_RGBA8 bloom

            m_hdr_filter.m_input_samplers[2] = m_compute_hdr->GetBloomSampler();
		    m_hdr_filter.Render(STATISTIC_SAMPLES);

		    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT,0);

		    //pp
		    RunEffect(BlurEffect);
		    RenderEffect(BlurEffect);

		    break;
	    }
	    default:
		    break;
	}
}

KCL::KCL_Status GLB_Scene_ES31::reloadShaders()
{
    GLBShader2::InvalidateShaderCache();

	GLB_Scene_ES3::reloadShaders();
	
	KCL::KCL_Status result;
	GLBShaderBuilder sb;
	sb.FileVs("pp2.vs").FileFs("pp2.fs") ;

	if ( !(m_disabledRenderBits & RenderBits::ERB_Compute_DOF) )
	{
		sb.AddDefine("DOF_ENABLED") ;
	}
	m_pp2 = sb.Build(result);

	sb.FileFs("hdr_luminance.h").FileFs("hdr_common.h").ShaderFile("pp_hdr.shader");
	m_hdr_filter.m_shader = sb.Build( result);
	
	delete m_InstancedLightRenderer;
	m_InstancedLightRenderer = new InstancedLightRenderer();
	m_InstancedLightRenderer->Init();
	
	delete m_compute_lightning;
	m_compute_lightning = new ComputeLightning();
	m_compute_lightning->Init(m_InstancedLightRenderer->GetLightningInstanceDataVBO(), this);   

	// depth of field blur strength for full HD
	int dof_blur_strength = 9;
	dof_blur_strength = (dof_blur_strength*m_viewport_height) / 1080; // normalize for actual resolution

    delete m_dof_blur;
    m_dof_blur = new FragmentBlur();
    m_dof_blur->Init(m_fullscreen_quad_vao, m_fullscreen_quad_vbo, m_viewport_width, m_viewport_height, dof_blur_strength, GL_RGBA8, 1, this);

	delete m_compute_hdr;
	m_compute_hdr = new ComputeHDR31();
    m_compute_hdr->Init(m_viewport_width,m_viewport_height, GL_RGBA8, m_fullscreen_quad_vao, m_fullscreen_quad_vbo, this);    
    
	return KCL::KCL_TESTERROR_NOERROR;
}

void Filter31::SetUniforms()
{
	if (m_shader->m_has_uniform_block[Shader::sUBOnames::FilterConsts])
	{
		m_ubo_manager->BindFilter(this);
	}
}


#ifdef UBO31
Filter * GLB_Scene_ES31::CreateFilter()
{
	Filter31 * filter = new Filter31();
	filter->m_ubo_manager = m_ubo_manager;
	return filter;
}

void GLB_Scene_ES31::Animate()
{
    // Calculate the delta time. It can be negative
    KCL::int32 delta_time;
    if (m_last_animation_time < 0)
    {
        // Ensure first frame delta time is zero even during single frame rendering
        delta_time = 0;
    }
    else
    {
        delta_time = m_animation_time - m_last_animation_time;
        if(delta_time == 0)
        {
            delta_time = 40; //40 ms to simulate 25 fps if animation is stopped
        }
    }   
    m_last_animation_time = m_animation_time;

    m_ubo_frame.time_dt_pad2.x = m_animation_time * 0.001f;
    m_ubo_frame.time_dt_pad2.y = delta_time * 0.001f; // to seconds  
            
    GLB_Scene_ES3::Animate();
}

void GLB_Scene_ES31::Render()
{
	m_ubo_manager->BeginFrame();

	m_ubo_frame.shadow_matrix0 = m_global_shadowmaps[0]->m_matrix;
	m_ubo_manager->SetFrame(&m_ubo_frame);

	float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

	const Camera2 * camera = m_active_camera;
	m_ubo_camera.view_dirXYZ_pad = KCL::Vector4D( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10], 0.0f);
	m_ubo_camera.view_posXYZ_normalized_time = KCL::Vector4D(camera->GetEye().x, camera->GetEye().y, camera->GetEye().z, normalized_time);
	m_ubo_camera.depth_parameters = m_active_camera->m_depth_linearize_factors;
	m_ubo_camera.inv_resolutionXY_pad = KCL::Vector4D(1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f);
	m_ubo_camera.vp = m_active_camera->GetViewProjection();
	m_ubo_manager->SetCamera(UBOManager::Camera::NORMAL, &m_ubo_camera);

	for (unsigned int i = 0; i < 3; i++)
	{
		UploadMeshes(camera, m_visible_meshes[i], NULL);
	}

	UploadMeshes(camera, m_sky_mesh, NULL);

	if (lfm.empty())
		lfm.push_back(m_lens_flare_mesh);
	UploadMeshes(camera, lfm, NULL);


	m_ubo_static_mesh.model = m_shadow_decal_light_matrix;
	m_ubo_static_mesh.inv_model.identity();	
	m_ubo_mesh.mvp = m_shadow_decal_light_matrix * m_active_camera->GetViewProjection();
	m_ubo_mesh.mv.identity();
	m_ubo_mesh.inv_modelview.identity();
	m_ubo_manager->SetLBO(&m_lbos[0], &m_ubo_mesh, &m_ubo_static_mesh);	
	
	KCL::Vector4D emitter_apertureXYZ_focusdist;
	KCL::Matrix4x4 emitter_worldmat;
	KCL::Vector4D emitter_min_freqXYZ_speed;
	KCL::Vector4D emitter_max_freqXYZ_speed;
	KCL::Vector4D emitter_min_ampXYZ_accel;
	KCL::Vector4D emitter_max_ampXYZ_accel;
	KCL::Vector4D emitter_color;
	KCL::Vector4D emitter_externalVel_gravityFactor;
	KCL::Vector3D emitter_maxlife_sizeXY;

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			GLB::TF_emitter *emitter = static_cast<GLB::TF_emitter*>(actor->m_emitters[i]);

		
			emitter->GetEmitterParams(	emitter_apertureXYZ_focusdist, 
										emitter_worldmat, 
										emitter_min_freqXYZ_speed, 
										emitter_max_freqXYZ_speed, 
										emitter_min_ampXYZ_accel,
										emitter_max_ampXYZ_accel,
										emitter_color,
										emitter_externalVel_gravityFactor,
										emitter_maxlife_sizeXY);

			m_ubo_emitter_advect.emitter_apertureXYZ_focusdist = emitter_apertureXYZ_focusdist;
			m_ubo_emitter_advect.emitter_worldmat = emitter_worldmat;
			m_ubo_emitter_advect.emitter_min_freqXYZ_speed = emitter_min_freqXYZ_speed;
			m_ubo_emitter_advect.emitter_max_freqXYZ_speed = emitter_max_freqXYZ_speed;
			m_ubo_emitter_advect.emitter_min_ampXYZ_accel = emitter_min_ampXYZ_accel;
			m_ubo_emitter_advect.emitter_max_ampXYZ_accel = emitter_max_ampXYZ_accel;
			//m_ubo_emitter.emitter_color = emitter_color;
			m_ubo_emitter_advect.emitter_externalVel_gravityFactor = emitter_externalVel_gravityFactor;
			m_ubo_emitter_advect.emitter_maxlifeX_sizeYZ_numSubsteps.x = emitter_maxlife_sizeXY.x;
			m_ubo_emitter_advect.emitter_maxlifeX_sizeYZ_numSubsteps.y = emitter_maxlife_sizeXY.y;
			m_ubo_emitter_advect.emitter_maxlifeX_sizeYZ_numSubsteps.z = emitter_maxlife_sizeXY.z;
			m_ubo_emitter_advect.emitter_maxlifeX_sizeYZ_numSubsteps.w = emitter->GetNumSubsteps();

			m_ubo_emitter_render.emitter_maxlifeX_sizeYZ_pad = KCL::Vector4D(emitter_maxlife_sizeXY);
			m_ubo_emitter_render.color_pad = emitter->GetColor();
			m_ubo_emitter_render.mvp =  m_active_camera->GetViewProjection();
			m_ubo_emitter_render.mv =  m_active_camera->GetView();
						
			m_ubo_manager->SetEmitter(emitter, &m_ubo_emitter_advect, &m_ubo_emitter_render);
		}
	}

	KCL::Matrix4x4 m0;
	KCL::Matrix4x4 m;

	for (unsigned int i = 0; i < m_visible_lights.size(); i++)
	{
		GLB::Light *l = dynamic_cast<GLB::Light *>(m_visible_lights[i]);
		if (!l || !l->m_has_lensflare)
		{
			continue;
		}
		m_ubo_light_lens_flare.light_pos_pad.x = l->m_world_pom.v[12];
		m_ubo_light_lens_flare.light_pos_pad.y = l->m_world_pom.v[13];
		m_ubo_light_lens_flare.light_pos_pad.z = l->m_world_pom.v[14];

		m_ubo_light_lens_flare.light_color_pad.x = l->m_diffuse_color.x;
		m_ubo_light_lens_flare.light_color_pad.y = l->m_diffuse_color.y;
		m_ubo_light_lens_flare.light_color_pad.z = l->m_diffuse_color.z;

		m_ubo_manager->SetLightLensFlare(l, &m_ubo_light_lens_flare);
	}
	

	for (unsigned int i = 0; i < m_lightshafts.size(); i++)
	{
		GLB::Light *l = dynamic_cast<GLB::Light *>(m_lightshafts[i]);
		if (!l)
		{
			continue;
		}

		// shadow_matrix0		
		static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
			0, 0.5f, 0, 0,
			0, 0, 0.5f, 0,
			0.5f, 0.5f, 0.5f, 1);

		KCL::Matrix4x4::Invert4x4( l->m_world_pom, m0);
		m = m0 * l->m_light_projection * shadowM;
		m_ubo_light_shaft.shadow_matrix0 = m;
		
		// light_x		
		Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]);
		dir.normalize();

		m_ubo_light_shaft.light_x.x = dir.x;
		m_ubo_light_shaft.light_x.y = dir.y;
		m_ubo_light_shaft.light_x.z = dir.z;
		m_ubo_light_shaft.light_x.w =l->m_world_pom[14];

		// light_pos
		m_ubo_light_shaft.light_pos_pad.set(l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], 0.0f);
		
		// light_color		
		float intensity = 1.0f;
		if( l->m_intensity_track)
		{
			Vector4D v;
			l->t = m_animation_time / 1000.0f;
			_key_node::Get(v, l->m_intensity_track, l->t, l->tb, true);
			intensity = v.x / l->m_intensity;
		}
		m_ubo_light_shaft.light_color_pad.set(l->m_diffuse_color.x * intensity, l->m_diffuse_color.y * intensity, l->m_diffuse_color.z * intensity, 0.0);		

		// attenuation_parameter		
		m_ubo_light_shaft.spot_cos_attenuation_parameter_pad.z = -1.0f / (l->m_radius * l->m_radius);
		
		// spot cos
		float fov = l->m_spotAngle;
		m_ubo_light_shaft.spot_cos_attenuation_parameter_pad.x = cosf( Math::Rad( fov / 2.0f));
		m_ubo_light_shaft.spot_cos_attenuation_parameter_pad.y = 1.0f / (1.0f - m_ubo_light_shaft.spot_cos_attenuation_parameter_pad.x);

		m_ubo_light_shaft.mvp = m_active_camera->GetViewProjection();
		m_ubo_light_shaft.mv =  m_active_camera->GetView();

		m_ubo_manager->SetLightShaft(l, &m_ubo_light_shaft);
	}

	// Set the filters
	for (unsigned int i = 0; i < FILTER_COUNT; i++)
	{
		if (filters[i]->m_width == 0 && filters[i]->m_height == 0)
		{
			// This filter is not used
			continue;
		}

		if (filters[i]->m_dir)
		{
			m_ubo_filter.offset_2d_pad2.x = 1.0f / filters[i]->m_width;
			m_ubo_filter.offset_2d_pad2.y = 0.0f;
		}
		else
		{
			m_ubo_filter.offset_2d_pad2.x = 0.0f;
			m_ubo_filter.offset_2d_pad2.y = 1.0f / filters[i]->m_height;		
		}
		m_ubo_manager->SetFilter((Filter31*)filters[i], &m_ubo_filter);
	}
	
	camera = &m_global_shadowmaps[0]->m_camera;
	m_ubo_camera.depth_parameters = m_active_camera->m_depth_linearize_factors;
	m_ubo_camera.view_dirXYZ_pad = KCL::Vector4D( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10], 0.0f);
	m_ubo_camera.view_posXYZ_normalized_time = KCL::Vector4D(camera->GetEye().x, camera->GetEye().y, camera->GetEye().z, normalized_time);
	m_ubo_camera.inv_resolutionXY_pad = KCL::Vector4D(1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f);
	m_ubo_camera.vp = camera->GetViewProjection();
	m_ubo_manager->SetCamera(UBOManager::Camera::SHADOW, &m_ubo_camera);
	
	KRL_ShadowMap* sm = m_global_shadowmaps[0];
	std::vector<Mesh*> visible_meshes[2];
	for(uint32 j=0; j<2; j++)
	{
		for(uint32 k=0; k < sm->m_caster_meshes[j].size(); k++)
		{
			visible_meshes[j].push_back( sm->m_caster_meshes[j][k]);
		}
	}

	UploadMeshes(camera, visible_meshes[0], m_shadowCasterMaterial);
	UploadMeshes(camera, visible_meshes[1], m_shadowCasterMaterial);

	m_ubo_manager->Upload();

	GLB_Scene_ES3::Render();	

#ifndef GL_WRAPPER_STATS
    m_gui->RenderGUI();
#endif

	m_ubo_manager->EndFrame();
}

void GLB_Scene_ES31::UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys)
{
	m_gui->UpdateGUI(cursorX, cursorY, mouseLPressed, mouseLClicked, mouseRPressed, mouseRClicked, downKeys);
}

void GLB_Scene_ES31::CollectInstances( std::vector<KCL::Mesh*> &visible_meshes)
{
#if defined ENABLE_FRAME_CAPTURE
#pragma message("Framecapture: CollectInstances disabled")
	return;
#endif

	// Clear the instance collections
    for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		Mesh* sm = (Mesh*)visible_meshes[j];
		KRL::Mesh3 *km = (KRL::Mesh3 *)sm->m_mesh;

		km->m_is_instance_data_updated = false;
		km->m_instances.clear();
		km->m_is_rendered.clear();
	}

	// Collect the same instances by material
	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		KCL::Mesh* m = visible_meshes[j];
		KRL::Mesh3 *km = (KRL::Mesh3*)m->m_mesh;
		KRL::Mesh3::InstanceData id;

		id.mv = m->m_world_pom;
		KCL::Matrix4x4::InvertModelView( id.mv, id.inv_mv);

		km->m_instances[m->m_material].push_back( id);
	}

	// Upload the instance data
	// Note: Instancing only supports one type of material
    for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		KCL::Mesh* m = visible_meshes[j];
        GLB::Mesh3 *gm = static_cast<GLB::Mesh3*>(m->m_mesh);
		if (gm->m_is_instance_data_updated)
		{
			// The instance data is already uploaded for this geometry mesh
			continue;
		}

		std::vector<Mesh3::InstanceData> & instances = gm->m_instances[m->m_material];
		if (instances.size() > 1)
		{
			size_t dataCount = sizeof(instances[0]) * instances.size();	
			gm->UpdateInstanceVBO(instances[0].mv.v, static_cast<int>(dataCount));
			gm->m_is_instance_data_updated = true;
		}
	}
}

void GLB_Scene_ES31::UploadMeshes(const KCL::Camera2* camera, std::vector<KCL::Mesh*> & meshes, KCL::Material *_override_material)
{
	unsigned int mesh_count = meshes.size();
	if (!mesh_count)
	{
		return;
	}

	if (&meshes == &m_visible_meshes[0])
	{
		CollectInstances(meshes);
	}	

	GLB::Material *override_material = dynamic_cast<GLB::Material*>(_override_material);

	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 model;
	KCL::Matrix4x4 inv_model;
	KCL::Matrix4x4 inv_modelview;
	Vector3D pos;
	
	const KCL::Matrix4x4 & camera_view = camera->GetView();		
	const KCL::Matrix4x4 & camera_view_projection = camera->GetViewProjection();
	const KCL::Matrix4x4 & camera_view_projection_origo = camera->GetViewProjectionOrigo();

	for( uint32 j=0; j<mesh_count; j++)
	{
		Mesh* sm = (Mesh*)meshes[j];
		KRL::Mesh3 *krl_mesh  = (KRL::Mesh3 *)sm->m_mesh;

		if( !sm->m_mesh)
		{
			continue;
		}
		int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

		if( krl_mesh->m_instances[sm->m_material].size() > 1)
		{			
			mesh_type = 2;
		}

		GLB::Material *material = dynamic_cast<GLB::Material*>(sm->m_material);

		if( override_material)
		{
			material = override_material;
		}
		
		// Decode the video here. "Translate UV coords" part overwrites m_frame_when_animated!
		if( material->m_ogg_decoder)
		{
			if( material->m_frame_when_animated != m_frame)
			{
				material->DecodeVideo();
				material->m_frame_when_animated = m_frame;
			}
		}
		
		pos.x = sm->m_world_pom.v[12];
		pos.y = sm->m_world_pom.v[13];
		pos.z = sm->m_world_pom.v[14];		 
								
		switch( mesh_type)
		{
		case 0:
			{
				if( material->m_material_type == KCL::Material::SKY)
				{
					mvp = sm->m_world_pom * camera_view_projection_origo;
				}
				else
				{
					mvp = sm->m_world_pom * camera_view_projection;
				}
				
				mv = sm->m_world_pom * camera_view;
				GLB::Matrix4x4::InvertModelView(mv, inv_modelview);
				
				model = sm->m_world_pom;
				inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
				break;
			}
		case 2:
		case 1:
			{
				mvp = camera_view_projection;
				mv = camera_view;
				GLB::Matrix4x4::InvertModelView(mv, inv_modelview);
				model.identity();
				inv_model.identity();
				break;
			}
		}
		
		m_ubo_static_mesh.model = model;
		m_ubo_static_mesh.inv_model = inv_model;
		m_ubo_static_mesh.color_pad.x = sm->m_color.x;
		m_ubo_static_mesh.color_pad.y = sm->m_color.y;
		m_ubo_static_mesh.color_pad.z = sm->m_color.z;

		m_ubo_mesh.mvp = mvp;
		m_ubo_mesh.mv = mv;
		m_ubo_mesh.inv_modelview = inv_modelview;
		m_ubo_manager->SetMesh(sm, &m_ubo_mesh, &m_ubo_static_mesh);

		for (unsigned int i = 0; i < 2; i++)
		{
			Shader * s = material->m_shaders[i][mesh_type];
			if (!s)
			{
				continue;
			}
			if (s->m_has_uniform_block[Shader::sUBOnames::TranslateUVConsts])
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

						_key_node::Get( r, sm->m_material->m_translate_u_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

						sm->m_material->m_uv_offset.x = -r.x;
					}
					if( sm->m_material->m_translate_v_track)
					{
						Vector4D r;

						_key_node::Get( r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

						sm->m_material->m_uv_offset.y = -r.x;
					}
					sm->m_material->m_frame_when_animated = m_frame;
				}		

				m_ubo_translate_uv.translate_uv_pad2.x = sm->m_material->m_uv_offset.x;
				m_ubo_translate_uv.translate_uv_pad2.y = sm->m_material->m_uv_offset.y;
				m_ubo_manager->SetTranslateUV(sm, &m_ubo_translate_uv);
				break;
			}
		}
		
		if (!m_cubemaps.empty())
		{
			KRL_CubeEnvMap *envmaps[2];
			float envmaps_interpolator = 0.0f;
			for (unsigned int i = 0; i < 2; i++)
			{
				Shader * s = material->m_shaders[i][mesh_type];
				if (!s)
				{
					continue;
				}
				if (s->m_has_uniform_block[Shader::sUBOnames::EnvmapsInterpolatorConsts])
				{				
					Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);

					m_ubo_envmaps.envmaps_interpolator_pad3.x = envmaps_interpolator;
					m_ubo_manager->SetEnvmapsInterpolator(sm, &m_ubo_envmaps);					

					break;				
				}
			}
		}		
	}
}

static bool material_compare(Mesh* A, Mesh* B)
{
	return (A->m_material->m_name < B->m_material->m_name);
}

struct mesh_depth_sort_info
{
	KCL::Mesh *mesh;
	Vector2D extents;
    float vertexCenterDist;
};

static bool depth_compare(const mesh_depth_sort_info &A, const mesh_depth_sort_info &B)
{
    return A.vertexCenterDist < B.vertexCenterDist;
}

static void sort_vis_list(std::vector<KCL::Mesh*> &visible_meshes, KCL::Camera2 *camera)
{
	std::vector<mesh_depth_sort_info> sorted_visible_meshes;

	for (uint32 i = 0; i < visible_meshes.size(); i++)
	{
		mesh_depth_sort_info mesh_info;

		mesh_info.mesh = visible_meshes[i];
		mesh_info.extents = visible_meshes[i]->m_aabb.DistanceFromPlane(camera->GetCullPlane(KCL::CULLPLANE_NEAR));
        mesh_info.vertexCenterDist = Vector4D::dot( KCL::Vector4D(visible_meshes[i]->m_vertexCenter), camera->GetCullPlane(KCL::CULLPLANE_NEAR));

		// force objects with infinite bounds (actors) to draw first 
		if (mesh_info.extents.x < -FLT_MAX || mesh_info.extents.y > FLT_MAX)
		{
			mesh_info.extents.x = 0.0f;
			mesh_info.extents.y = 0.0f;
            mesh_info.vertexCenterDist = 0.0f;
		}
		sorted_visible_meshes.push_back(mesh_info);
	}

	// depth sort
	std::sort (sorted_visible_meshes.begin(), sorted_visible_meshes.end(), &depth_compare);

	// remap original visible meshes
	for (uint32 i = 0; i < visible_meshes.size(); i++)
		visible_meshes[i] = sorted_visible_meshes[i].mesh;
}

#define DEPTH_SORT_DRAWCALLS
void GLB_Scene_ES31::Render( KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
{
	if (visible_meshes.empty())
	{
		return;
	}

	GLB::Light *glb_light = dynamic_cast<GLB::Light *>(light);

	m_ubo_manager->BindCamera(camera == m_active_camera ? UBOManager::Camera::NORMAL : UBOManager::Camera::SHADOW);

#ifdef PER_FRAME_ALU_INFO
	uint32 samplesPassed = 0;
#endif


#ifdef DEPTH_SORT_DRAWCALLS
		// depth sort 
		sort_vis_list(visible_meshes, camera);
#elif defined MATERIAL_SORT_DRAWCALLS
		// material sort 
		std::sort (visible_meshes.begin(), visible_meshes.end(), &material_compare);
		//qsort( &visible_meshes[0], visible_meshes.size(), sizeof( Mesh*), &material_compare);
#endif	
			
	//if( &visible_meshes == &m_visible_meshes[0])
	//{
	//	CollectInstances( visible_meshes);
	//}	

	GLB::Material *override_material = dynamic_cast<GLB::Material*>(_override_material);
	GLB::Material *last_material = NULL;
	int last_mesh_type = -1;
	KCL::uint32 texture_num_from_material = 0;

	Vector3D pos;
	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
#ifdef PER_FRAME_ALU_INFO
		samplesPassed = 0;
		glBeginQuery(GL_SAMPLES_PASSED, Shader::QueryId());
#endif

		Mesh* sm = (Mesh*)visible_meshes[j];
		KRL::Mesh3 *krl_mesh  = (KRL::Mesh3 *)sm->m_mesh;

		if( !sm->m_mesh)
		{
			continue;
		}

		int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

		if( krl_mesh->m_instances[sm->m_material].size() > 1)
		{
			std::set<KCL::Material*>::iterator iter = krl_mesh->m_is_rendered.find( sm->m_material);

			if( iter != krl_mesh->m_is_rendered.end())
			{
				continue;
			}
			
			mesh_type = 2;
		}

		GLB::Material *material = dynamic_cast<GLB::Material*>(sm->m_material);

		if( override_material)
		{
			material = override_material;
		}

		pos.x = sm->m_world_pom.v[12];
		pos.y = sm->m_world_pom.v[13];
		pos.z = sm->m_world_pom.v[14];	

		// force use of shader[1][?] for depth prepass
		int shader_bank = (pass_type == -1) ? 1 : 0;

		Shader *s = material->m_shaders[shader_bank][mesh_type];
		KRL_CubeEnvMap *envmaps[2];
		float envmaps_interpolator = 0.0f;
		if( last_material != material || last_mesh_type != mesh_type)
		{
			if( last_material)
			{
				last_material->postInit();
			}

			texture_num_from_material = 0;
			material->preInit( texture_num_from_material, mesh_type, pass_type);
		}

		KCL::uint32 texture_num = texture_num_from_material;

		last_material = material;

		last_mesh_type = mesh_type;

		// envmaps_interpolator
		if (!m_cubemaps.empty() && s->m_has_uniform_block[Shader::sUBOnames::EnvmapsInterpolatorConsts])
		{			
			Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);

			m_ubo_manager->BindEnvmapsInterpolator(sm);

			// envmap0
			envmaps[0]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap0], texture_num++);

			// envmap1
			envmaps[1]->GetTexture()->bind( texture_num);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);

			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

#ifdef TEXTURE_COUNTING
            m_textureCounter.insert( ((GLBTexture*)envmaps[0]->GetTexture())->textureObject() );
			m_textureCounter.insert( ((GLBTexture*)envmaps[1]->GetTexture())->textureObject() );
#endif
		}

		if (s->m_has_uniform_block[Shader::sUBOnames::TranslateUVConsts])
		{
			m_ubo_manager->BindTranslateUV(sm);
		}
				
		if (glb_light)
		{
			m_ubo_manager->BindLightLensFlare(glb_light);
		}		
		
#ifdef OCCLUSION_QUERY_BASED_STAT
        if(do_not_skip_samples_passed || m_measurePixelCoverage)
			m_glGLSamplesPassedQuery->Begin();
#endif

        KCL::uint32 instanceCount = 0;

#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
        if(m_measurePixelCoverage)
	    {
            //turn off depth testing - otherwise covered samples are not taken into account but covered polys are
            GLB::OpenGLStateManager::GlDisable(GL_DEPTH_TEST);
            GLB::OpenGLStateManager::GlDepthFunc(GL_ALWAYS);
        }
#endif

		if( krl_mesh->m_instances[sm->m_material].size() < 2)
		{
			GLB::OpenGLStateManager::Commit();
						
			glBindVertexArray(m_vao_storage->get(sm->m_mesh,s));

			m_ubo_manager->BindMesh(sm, material);		
			//dump_ubos();
			glDrawElements(GL_TRIANGLES, sm->m_primitive_count ? sm->m_primitive_count : KCL::uint32(sm->m_mesh->getIndexCount(lod)), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset);

			glBindVertexArray( 0);

            instanceCount = 1;
		}
		else
		{
			krl_mesh->m_is_rendered.insert( sm->m_material);

			glBindVertexArray(m_vao_storage->get(sm->m_mesh,s));

			for( int i=0; i<4; i++)
			{				
				glVertexAttribDivisor(s->m_attrib_locations[GLB::attribs::in_instance_mv0 + i], 1);
				glVertexAttribDivisor(s->m_attrib_locations[GLB::attribs::in_instance_inv_mv] + i, 1);
			}

			GLB::OpenGLStateManager::Commit();

			m_ubo_manager->BindMesh(sm, material);
			//dump_ubos();
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(sm->m_mesh->getIndexCount(lod)), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset, static_cast<GLsizei>(krl_mesh->m_instances[sm->m_material].size()));

			for( int i=0; i<4; i++)
			{			
				glVertexAttribDivisor(s->m_attrib_locations[GLB::attribs::in_instance_mv0 + i], 0);
				glVertexAttribDivisor(s->m_attrib_locations[GLB::attribs::in_instance_inv_mv] + i, 0);
			}

			glBindVertexArray( 0);

            instanceCount = (uint32)krl_mesh->m_instances[sm->m_material].size();
		}

		m_num_draw_calls++;
		m_num_triangles += KCL::uint32(sm->m_mesh->getIndexCount(lod)) / 3 * instanceCount;
		m_num_vertices += (uint32)sm->m_mesh->getIndexCount(lod) * instanceCount;

#ifdef OCCLUSION_QUERY_BASED_STAT
		if(do_not_skip_samples_passed || m_measurePixelCoverage)
		{
			m_glGLSamplesPassedQuery->End();
            size_t sampleCount = m_glGLSamplesPassedQuery->Result();
            if(do_not_skip_samples_passed)
			{
                m_num_samples_passed += sampleCount;

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
			    m_num_instruction += s->m_instruction_count_v * sm->m_mesh->getIndexCount(lod) * instanceCount + s->m_instruction_count_f * sampleCount;
#endif
            }
            
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
            if(m_measurePixelCoverage && sampleCount)
		    {
                KCL::uint32 primCount = sm->m_mesh->getIndexCount(lod) / 3 / 2 * instanceCount;

		        float vis = float(sampleCount) / float(primCount);
                fprintf( m_coverageFile, "%.0f;%s;%d;%d;%.1f\n", m_animation_time * m_animation_multiplier, sm->m_name.c_str(), primCount, sampleCount, vis);

                m_pixel_coverage_sampleCount += sampleCount;
                m_pixel_coverage_primCount += primCount;
		    }
#endif
		}


#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
        if(m_measurePixelCoverage)
	    {
            //turn depth testing back on
            OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
            OpenGLStateManager::GlDepthFunc(GL_LESS);
        }
#endif

#endif

#ifdef PER_FRAME_ALU_INFO
		glEndQuery(GL_SAMPLES_PASSED);
				
		glGetQueryObjectuiv( Shader::QueryId(), GL_QUERY_RESULT, &samplesPassed);

		Shader::Add_FrameAluInfo(s, samplesPassed, sm->m_mesh->m_vertex_indices[lod].size() / 3);
#endif

	} // for visible_meshes

	if(last_material)
	{
		last_material->postInit();
	}

	VboPool::Instance()->BindBuffer(0);
	IndexBufferPool::Instance()->BindBuffer(0);
}

void GLB_Scene_ES31::RenderLight( KCL::Light *l)
{    
    // Do not render shadows's light if there is no visible shadow caster
    if( m_global_shadowmaps[0]->m_caster_meshes[0].empty() && m_global_shadowmaps[0]->m_caster_meshes[1].empty())
    {       
        return;
    }

	Shader *s = m_lighting_shaders[15];

	glUseProgram( s->m_p);

	GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + 3);
	glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit3], 3);
	glBindTexture( GL_TEXTURE_2D, pp.m_depth_texture);
    glBindSampler(3, 0);

#ifdef TEXTURE_COUNTING
	m_textureCounter.insert( pp.m_depth_texture );
#endif

	m_ubo_manager->BindCamera(UBOManager::Camera::NORMAL);
	m_ubo_manager->BindLBO(&m_lbos[0]);
	
	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId() );
    glBindSampler(7, 0);
	glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], 7);
#ifdef TEXTURE_COUNTING
	m_textureCounter.insert( m_global_shadowmaps[0]->GetTextureId() );
#endif

	GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

	GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery->Begin();
#endif

	glBindVertexArray( m_lbos[0].m_vao);
	
	glDrawElements( GL_TRIANGLES, m_lbos[0].m_num_indices, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray( 0);	

	m_num_draw_calls++;
	m_num_triangles += m_lbos[0].m_num_indices / 3;
	m_num_vertices += m_lbos[0].m_num_indices;

#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery->End();
	m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
	m_num_instruction += s->m_instruction_count_v * m_lbos[light_buffer_index].m_num_indices + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif	
}

void GLB_Scene_ES31::MoveParticles()
{
	GLB::OpenGLStateManager::Reset();

	KCL::uint32 advectVAO, renderBuffer, renderVAO;

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			GLB::TF_emitter *emitter = static_cast<GLB::TF_emitter*>(actor->m_emitters[i]);

			m_ubo_manager->BindEmitterAdvect(emitter);
			
			//Advect + birth particles in VS
			glEnable(GL_RASTERIZER_DISCARD);
			{
				Shader* s = m_particleAdvect_shader;

				// Set up the advection shader:
				GLB::OpenGLStateManager::GlUseProgram(s->m_p);
				Shader::m_last_shader = s;
				//glUseProgram(s->m_p);

				//noise map for per instance rnd emission values
                m_particle_noise->bind(0);
				#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( ((GLBTexture*)m_particle_noise)->textureObject() );
				#endif

					glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
				
				//get buffers - simulatesubstep might decide to skip advection, but we still need to render
				emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);

				const unsigned int numSubsteps = emitter->GetNumSubsteps();

                KCL::int32 subStepData[ TF_emitter::max_num_substeps * 4 ];

				for( unsigned int substep = 0; substep < numSubsteps; substep++ )
				{
					emitter->SimulateSubStep();

					float dummy;
					emitter->GetBufferData( subStepData[ substep * 4 ], subStepData[ substep * 4 + 1 ], subStepData[ substep * 4 + 2 ], dummy );
				}

				//advect all substeps in the shader in a single draw
				{
					emitter->SwitchBuffers();
					emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);

					// particleBufferParamsXYZ_pad
					if (s->m_uniform_locations[GLB::uniforms::particleBufferParamsXYZ_pad] > -1)
					{
						glUniform4iv(s->m_uniform_locations[GLB::uniforms::particleBufferParamsXYZ_pad], numSubsteps, subStepData);
					}
					
					// Specify the source buffer:
					glBindVertexArray(advectVAO);

					// Specify the target buffer:
                    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, renderBuffer);
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, renderBuffer);

					// Perform GPU advection:
					glBeginTransformFeedback(GL_POINTS);
					GLB::OpenGLStateManager::Commit();
					glDrawArrays(GL_POINTS, 0, emitter->VisibleParticleCount());

                    m_num_draw_calls++;
	                m_num_vertices += numSubsteps * emitter->VisibleParticleCount();
					#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
						m_num_instruction += s->m_instruction_count_v * numSubsteps * emitter->GetMaxParticleCount();
					#endif

					glEndTransformFeedback();

					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
					glBindVertexArray(0);
				}

			}
		}
	}

	glDisable(GL_RASTERIZER_DISCARD);
}

void GLB_Scene_ES31::RenderTFParticles()
{
	GLB::OpenGLStateManager::Reset();

	KCL::uint32 advectVAO, renderBuffer, renderVAO;

	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			GLB::TF_emitter *emitter = static_cast<GLB::TF_emitter*>(actor->m_emitters[i]);
		
			glDisable(GL_RASTERIZER_DISCARD);
			glDisable( GL_CULL_FACE);

            //get buffers - simulatesubstep might decide to skip advection, but we still need to render
            emitter->GetBuffers(advectVAO, renderBuffer, renderVAO);
				
			//Render particles
			{
				KCL::uint32 texture_num = 0;
				GLB::Material *material = NULL;
				if( emitter->m_name.find( "smoke") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_smoke_material);
				}
				if( emitter->m_name.find( "fire") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
				}
				if( emitter->m_name.find( "spark") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
				}
				if( emitter->m_name.find( "soot") != std::string::npos)
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_spark_material);
				}
				else
				{
					material = dynamic_cast<GLB::Material*>(m_instanced_fire_material);
				}

				Shader* s = material->m_shaders[0][0];
								
				GLB::OpenGLStateManager::GlUseProgram(s->m_p);
				Shader::m_last_shader = s;
				
				material->preInit( texture_num, 0, 0);

				// texture_3D_unit0
				if (s->m_uniform_locations[GLB::uniforms::texture_3D_unit0] > -1)
				{
					//if( emitter->m_emitter_type == 1)
                    m_fire_texid->bind(texture_num);
#ifdef TEXTURE_COUNTING
						m_textureCounter.insert( ((GLBTexture*)m_fire_texid)->textureObject());
#endif
						glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_3D_unit0], texture_num++);
				}

				// texture_unit0
				if (s->m_uniform_locations[GLB::uniforms::texture_unit0] > -1)
				{
					glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
				}
				
				m_ubo_manager->BindEmitterRender(emitter);

				glBindVertexArray(renderVAO);

#ifdef OCCLUSION_QUERY_BASED_STAT
				m_glGLSamplesPassedQuery->Begin();
#endif
				glVertexAttribDivisor(0, 1);
				glVertexAttribDivisor(1, 1);
				glVertexAttribDivisor(2, 1);
				glVertexAttribDivisor(3, 1);
				glVertexAttribDivisor(4, 1);
				glVertexAttribDivisor(5, 1);
				glVertexAttribDivisor(6, 1);
				glVertexAttribDivisor(7, 1);
				glVertexAttribDivisor(8, 1);

				GLB::OpenGLStateManager::Commit(); 
				glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, emitter->VisibleParticleCount() );
				m_num_draw_calls++;
				m_num_triangles += 2 * (emitter->VisibleParticleCount());
				m_num_vertices += 6 * (emitter->VisibleParticleCount());

				glVertexAttribDivisor(8, 0);
				glVertexAttribDivisor(7, 0);
				glVertexAttribDivisor(6, 0);
				glVertexAttribDivisor(5, 0);
				glVertexAttribDivisor(4, 0);
				glVertexAttribDivisor(3, 0);
				glVertexAttribDivisor(2, 0);
				glVertexAttribDivisor(1, 0);
				glVertexAttribDivisor(0, 0);
					
#ifdef OCCLUSION_QUERY_BASED_STAT
				m_glGLSamplesPassedQuery->End();
				m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
				m_num_instruction += s->m_instruction_count_v * (6 * emitter->VisibleParticleCount()) + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif
				glBindVertexArray(0);
				material->postInit();
			}
		}
	}
}

void GLB_Scene_ES31::RenderLightshafts()
{	
	LightShaft ls;

	for( uint32 j=0; j<m_lightshafts.size(); j++)
	{		
		KCL::Light *l = m_lightshafts[j];

		for( uint32 i=0; i<8; i++)
		{
			ls.m_corners[i].set( l->m_frustum_vertices[i].v);
		}

		KCL::Matrix4x4 m2 = l->m_inv_light_projection * l->m_world_pom;
		ls.CreateCone( m2, m_active_camera->GetCullPlane( KCL::CULLPLANE_NEAR), false);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_lightshaft_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lightshaft_ebo);

	if( ls.m_vertices.size() && ls.m_indices.size())
	{
        glBufferData( GL_ARRAY_BUFFER, ls.m_vertices.size() * sizeof(KCL::Vector3D), ls.m_vertices[0].v, GL_DYNAMIC_DRAW);
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, ls.m_indices.size() * sizeof(uint16), &ls.m_indices[0], GL_DYNAMIC_DRAW);
	}

	GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
	GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlDepthMask( 0);
	GLB::OpenGLStateManager::GlEnable( GL_BLEND);
	GLB::OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Shader *s = m_fog_shader;
	glUseProgram( s->m_p);

	for( uint32 j=0; j<m_lightshafts.size(); j++)
	{
		GLB::Light *l = dynamic_cast<GLB::Light *>(m_lightshafts[j]);
		if (!l)
		{
			continue;
		}
						
		if( !ls.m_num_indices[j])
		{
			continue;
		}

		KCL::uint32 textureNum = 1;
		if (s->m_uniform_locations[GLB::uniforms::texture_unit0 + 1] > -1)
		{
			GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + textureNum);
			glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0 + 1], textureNum);
			glBindTexture( GL_TEXTURE_2D, pp.m_depth_texture);
            glBindSampler(textureNum, 0);
			#ifdef TEXTURE_COUNTING
				m_textureCounter.insert( pp.m_depth_texture );
			#endif
			++textureNum;
		}
			
        m_light_noise->bind(0);
		glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
#ifdef TEXTURE_COUNTING
		m_textureCounter.insert( ((GLBTexture*)m_light_noise)->textureObject() );
#endif
		GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
		
#ifdef TEXTURE_COUNTING
		m_textureCounter.insert( ((GLBTexture*)m_fog_texid)->textureObject() );
#endif

		GLB::OpenGLStateManager::Commit();
#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->Begin();
#endif
		m_ubo_manager->BindCamera(UBOManager::Camera::NORMAL);
		m_ubo_manager->BindLightShaft(l);

		glBindVertexArray( m_lightshaft_vao);
		glDrawElements( GL_TRIANGLES, ls.m_num_indices[j], GL_UNSIGNED_SHORT, (void*) (ls.m_index_offsets[j] * sizeof( uint16)));
		glBindVertexArray( 0);

		m_num_draw_calls++;
		m_num_triangles += ls.m_num_indices[j] / 3;
		m_num_vertices += ls.m_num_indices[j];

#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->End();
		m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
		m_num_instruction += s->m_instruction_count_v * ls.m_num_indices[j] + s->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif

#endif
	}
}

static GLenum b[] = 
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3
};
void GLB_Scene_ES31::QueryLensFlare()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DRIVER WORKAROUND STARTS HERE: Avoids crash related to transform feedback on some devices
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    glFlush();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DRIVER WORKAROUND ENDS HERE
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GLenum c[] = 
	{
		GL_NONE,
		GL_NONE,
		GL_NONE,
		GL_NONE
	};
	glDrawBuffers(4, c);

#ifdef HAVE_GLEW
    if (!g_extension->isES())
    {
	    glEnable( GL_PROGRAM_POINT_SIZE);
    }
#endif

	glColorMask( 0,0,0,0);
	GLB::OpenGLStateManager::GlDepthMask( 0);
	GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);

	Shader::m_last_shader = 0;
	GLB::OpenGLStateManager::GlUseProgram( m_occlusion_query_shader->m_p);

	m_ubo_manager->BindCamera(UBOManager::Camera::NORMAL);

	for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
	{
		GLB::Light *l = static_cast<GLB::Light*>(m_visible_lights[i]);

		if( !l->m_has_lensflare)
		{
			continue;
		}

		if( l->m_light_type == KCL::Light::AMBIENT || l->m_light_type == KCL::Light::DIRECTIONAL || l->m_diffuse_color == Vector3D() )
		{
			continue;
		}


		l->NextQueryObject();

		glBeginQuery( GL_ANY_SAMPLES_PASSED, l->GetCurrentQueryObject());

		glBindBuffer( GL_ARRAY_BUFFER, m_occlusion_query_vbo);
		glBufferData( GL_ARRAY_BUFFER, 1 * sizeof(KCL::Vector3D), &l->m_world_pom.v[12], GL_DYNAMIC_DRAW);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
		
		GLB::OpenGLStateManager::Commit();
		
		glBindVertexArray( m_occlusion_query_vao);		
		glDrawArrays( GL_POINTS, 0, 1);
		glBindVertexArray( 0);
	
        m_num_draw_calls++;
	    m_num_vertices += 1;
		#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
			m_num_instruction += m_occlusion_query_shader->m_instruction_count_v * 1;
		#endif
			
		glEndQuery( GL_ANY_SAMPLES_PASSED);
	}

	GLB::OpenGLStateManager::GlDepthMask( 1);
	glColorMask( 1,1,1,1);
	GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
    glDrawBuffers(1, b);
}

#endif // UBO31

#else


GLB_Scene_ES31::GLB_Scene_ES31()
{
}

GLB_Scene_ES31::~GLB_Scene_ES31()
{	
}

#endif
