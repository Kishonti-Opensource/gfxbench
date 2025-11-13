/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_scene5_ovr.h"
#include "gfxb_ovr_mesh.h"

#include "gfxb_ovr_mesh_filters.h"

#include "common/gfxb_factories.h"
#include "common/gfxb_light.h"
#include "common/gfxb_texture.h"
#include "common/gfxb_material.h"
#include "common/gfxb_shader.h"
#include "common/gfxb_mesh.h"
#include "common/gfxb_shapes.h"
#include "common/gfxb_cubemap.h"
#include "common/gfxb_environment.h"
#include "common/gfxb_debug_renderer.h"
#include "common/gfxb_hdr.h"
#include "common/gfxb_cascaded_shadow_map.h"
#include "common/gfxb_frustum_cull.h"
#include "common/gfxb_tools.h"
#include "common/gfxb_hdr.h"
#include "common/gfxb_fragment_blur.h"
#include "gfxb_barrier.h"
#include <ngl.h>

#include <kcl_actor.h>

#include "common/gfxb_mesh_shape.h"

//#define DEFERRED_MODE

using namespace GFXB;

//KCL::Vector3D EYEPOS = KCL::Vector3D(51.6269608f, 1.09762466f, 0.660734951f);
KCL::Vector3D EYEPOS = KCL::Vector3D(50.0f, 1.8f, 0.0f); //altar pos
//KCL::Vector3D EYEPOS = KCL::Vector3D( 48.85f, 0.5f, 2.0f ); //altar pos


Scene5Ovr::Scene5Ovr() : SceneBase(KCL::SV_50_OVR)
{
	KCL::Camera2::enable_orrientation_rotation = false; //rotation fix

	// Create the factories
	m_texture_factory = new GFXB::TextureFactory();
	m_texture_factory->SetMipLevelLimit( 11 );
	m_mesh3_factory = new GFXB::Mesh3Factory();
	m_material_factory = new GFXB::MaterialFactory();
	m_light_factory = new GFXB::LightFactory(this);
	//m_particlesystem_factory = new GFXB::ParticleSystemFactory(this);
	m_mesh_factory = new OvrMeshFactory();


	// Register KCL::Object factories
	GetFactoryInstance().RegisterFactory(m_material_factory, KCL::MATERIAL);
	GetFactoryInstance().RegisterFactory(m_light_factory, KCL::LIGHT);
	//GetFactoryInstance().RegisterFactory(m_particlesystem_factory, KCL::EMITTER5);
	GetFactoryInstance().RegisterFactory(m_mesh_factory, KCL::MESH);

	m_texture_factory->SetAnisotropyValue(nglGetInteger(NGL_TEXTURE_MAX_ANISOTROPY) > 0 ? 4 : 0);

	//init!!!
	m_shapes = nullptr;
	m_lighting_render = 0;
	m_deferred_render = 0;
	m_shadow_job = 0;
	m_ssao_render = 0;

	// Shaders
	m_cubemap_shader = 0;
	m_fullscreen_shader = 0;
	m_lighting_shader = 0;
	m_ssao_shader = 0;
	m_gbuffer_shader = 0;
	m_gbuffer_tessellated_shader = 0;
	m_gbuffer_alpha_shader = 0;
	m_skeletal_gbuffer_shader = 0;

	m_sunlight_dir = KCL::Vector3D(0.0f, 0.0f, 0.0f);

	m_integrate_brdf_lut_texture = 0;
	m_prefiltered_cubemap_texture = 0;

	m_gbuffer_color_texture = 0;
	m_gbuffer_normal_texture = 0;
	m_gbuffer_specular_texture = 0;
	m_gbuffer_depth_texture = 0;

	m_ssao_texture = 0;
	m_ssao_blur = 0;
	m_ssao_blur_strength = 0;

	m_bloom = 0;
	m_hdr_computer = 0;

	m_main_mesh_filter = nullptr;
	m_main_frustum_cull = nullptr;

	m_cmd_buffer = 0;
}


Scene5Ovr::~Scene5Ovr()
{
	ShaderFactory::Release();

	delete m_texture_factory;
	delete m_mesh3_factory;
	delete m_material_factory;
	delete m_light_factory;
	//delete m_particlesystem_factory;
	delete m_mesh_factory;

	delete m_shapes;

	delete m_bloom;
	delete m_hdr_computer;

	delete m_main_mesh_filter;
	delete m_main_frustum_cull;

	delete m_ssao_blur;
}

//taken from occulus sdk...
const float eye_sep = 0.0640f * 0.5f;
//convergence distance
const float conv_dist = 10.0f;
//old content hack, now not needed
const float cm_to_m = 1.0f;

void Scene5Ovr::Animate()
{
	SceneBase::Animate();

	GFXB::HDRValues hdrval = HDRValues();
	if( m_osm.getIntOption( "EXPOSURE_MANUAL" ).m_option == 1 )
	{
		hdrval.m_exposure_mode = EXPOSURE_MANUAL;
	}
	else
	{
		hdrval.m_exposure_mode = EXPOSURE_ADAPTIVE;
		//hdrval.m_exposure_mode = EXPOSURE_AUTO;
	}
	hdrval.m_bloom_intensity = 10.0f;
	hdrval.m_bloom_threshold = 0.6f;
	hdrval.m_exposure = 1.0f;
	//hdrval.m_adaptation_speed = 10.1f;
	//hdrval.m_auto_max_exposure = 10.0f;
	if( !m_eye )
	{
		m_hdr_computer->Animate( hdrval, m_animation_time );
	}

	KCL::Camera2* cam = m_fps_camera;
	KCL::Vector3D eyevec = cam->GetEye();
	KCL::Vector3D goal = cam->GetCenter();
	KCL::Vector3D view = ( goal - eyevec ).normalize();
	KCL::Vector3D up = this->m_up.normalize();
	//KCL::Vector3D up = cam.GetUp();
	KCL::Vector3D right = KCL::Vector3D::cross( view, up ).normalize();
	eyevec = ( EYEPOS + ( m_eye ? 1.0f * right * eye_sep : -1.0f * right * eye_sep ) ) * cm_to_m;
	cam->LookAt( eyevec, eyevec + view * conv_dist, up );
	cam->Perspective( 90.0f, m_w, m_h, 0.1f, 2500.0f );
	cam->Update();
	m_main_frustum_cull->Cull( cam );

	//m_fps_camera->SetNearFar( m_main_frustum_cull->GetNear(), m_main_frustum_cull->GetFar() );
	m_fps_camera->Update();

	m_active_camera->Perspective( 90.0f, m_w, m_h, cam->GetNear(), cam->GetFar() );
}

void Scene5Ovr::Render()
{
	if( !m_eye )
	{
		nglBeginCommandBuffer( m_cmd_buffer );
	}

	if( m_osm.GetHasChanged() )
	{
		ReloadShaders();
	}

	//test for one eye
	//if( m_eye )
	//	return;

	KCL::uint32 renderer =
#ifdef WITH_OVR_SDK
		m_final_render[m_active_backbuffer_id];
#else
		!m_eye ? m_final_render_left[m_active_backbuffer_id] : m_final_render_right[m_active_backbuffer_id];
#endif

	//debug
	//nglBegin( renderer );
	//nglCustomAction( 0, 200 );
	//nglEnd( renderer );

	KCL::Camera2 cam = *m_fps_camera;
	KCL::Camera2 cubemap_cam = *m_fps_camera;

	KCL::Vector4D light_dir;
	KCL::Vector4D view_dir;

	{
		KCL::Vector3D eyevec = cam.GetEye();
		KCL::Vector3D goal = cam.GetCenter();
		KCL::Vector3D view = (goal - eyevec).normalize();
		KCL::Vector3D up = this->m_up.normalize();
		//KCL::Vector3D up = cam.GetUp();
		KCL::Vector3D right = KCL::Vector3D::cross(view, up).normalize();
		eyevec = (EYEPOS + (m_eye ? 1.0f * right * eye_sep : -1.0f * right * eye_sep)) * cm_to_m;
		cam.LookAt(eyevec, eyevec + view * conv_dist, up);
		cam.Perspective(90.0f, m_w, m_h, cam.GetNear(), cam.GetFar());
		cam.Update(); 
	}

	{
		KCL::Vector3D eyevec = cubemap_cam.GetEye();
		KCL::Vector3D goal = cubemap_cam.GetCenter();
		KCL::Vector3D view = (goal - eyevec).normalize();
		view_dir = KCL::Vector4D(view, 1);
		KCL::Vector3D up = this->m_up.normalize();
		//KCL::Vector3D up = cubemap_cam.GetUp();
		//eyevec = EYEPOS * cm_to_m;
		eyevec = KCL::Vector3D( 0.0f, 0.0f, 0.0f );
		cubemap_cam.LookAt(eyevec, eyevec + view * conv_dist, up);
		cubemap_cam.Perspective(90.0f, m_w, m_h, cam.GetNear(), cam.GetFar());
		cubemap_cam.Update();

		//KCL::Matrix4x4 proj = cubemap_cam.GetProjection();
		//for (int c = 0; c < 16; ++c)
		//	INFO("PROJ MX: %.5f", proj.v[c]);
		//INFO( "\n" );
	}

	m_active_camera->Update();
		 
	KCL::Matrix4x4 model;
	KCL::Matrix4x4 mvp = cam.GetViewProjection();
	KCL::Matrix4x4 vp = cam.GetViewProjection();

	void *p[UNIFORM_MAX];
	memset( p, 0, sizeof( p ) );
	p[UNIFORM_MVP] = mvp.v;
	p[UNIFORM_MODEL] = model.v;
	light_dir = KCL::Vector4D(m_sunlight_dir, 0.0f);
	p[UNIFORM_LIGHT_DIR] = light_dir.v;
	p[UNIFORM_SHADOW_MATRIX] = m_shadow_matrix.v;
	p[UNIFORM_VIEW_DIR] = view_dir.v;
	KCL::Vector4D view_pos = KCL::Vector4D( cam.GetEye().v );
	p[UNIFORM_VIEW_POS] = view_pos.v;
	p[UNIFORM_LIGHT_COLOR] = m_light_color.v;
	p[UNIFORM_DEPTH_PARAMETERS] = cam.m_depth_linearize_factors.v;

	RenderShadow( m_shadow_job, cam, p );

	//shadow overwrites this!
	p[UNIFORM_VP] = vp.v;

	RenderGBuffer( m_deferred_render, cubemap_cam, cam, p );
	//RenderGBuffer( renderer, cubemap_cam, cam, p );
	
#ifdef DEFERRED_MODE
	RenderSSAO( m_ssao_render, cam, p );

	RenderLighting( m_lighting_render, cam, p );
#endif

	//fix for left eye not being rendered properly on galaxy s6
	//+dirty optimization...
	if( !m_eye )
	{
		RenderHDR( -1, cam, p );
	}

	RenderBloom( -1, cam, p );

	RenderFinal( renderer, cam, p );

	if( m_eye )
	{
		nglEndCommandBuffer( m_cmd_buffer );
		nglSubmitCommandBuffer( m_cmd_buffer );
	}
}

void Scene5Ovr::RenderShadow( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	if( !m_osm.getIntOption( "ENABLE_SHADOW_MAPPING" ).m_option )
	{
		return;
	}

	if( m_eye ) //ONLY REQUIRED ONCE PER FRAME
	{
		return;
	}

	//INFO( "shadow render" );
	//INFO( "m_sunlight_dir: %.3f %.3f %.3f", m_sunlight_dir.x, m_sunlight_dir.y, m_sunlight_dir.z );

	p[UNIFORM_VP] = (void*)m_shadow_camera.GetViewProjection().v;

	/*INFO("shadow cam viewproj:");
	for( int c = 0; c < 4; ++c )
	{
		INFO( "%.3f %.3f %.3f %.3f", 
			m_shadow_camera.GetViewProjection()[c*4+0],
			m_shadow_camera.GetViewProjection()[c*4+1], 
			m_shadow_camera.GetViewProjection()[c*4+2],
			m_shadow_camera.GetViewProjection()[c*4+3] );
	}*/

	nglBegin( job, m_cmd_buffer );

	nglDepthState( job, NGL_DEPTH_LESS, true );

	//cubemap + oltar render
	auto e = this->m_rooms[0]; //only one eye
	{
		for( unsigned i = 0; i < e->m_meshes.size(); ++i )
		{
			auto c = e->m_meshes[i];

			if( c->m_name.find( "sky" ) != std::string::npos )
			{
				continue;
			}

			Mesh3 *gfxb_mesh3 = (Mesh3*)c->m_mesh;

			//p[UNIFORM_MODEL] = c->m_world_pom.v;
			KCL::Matrix4x4 m = c->m_world_pom * m_shadow_camera.GetViewProjection();
			p[UNIFORM_MVP] = m.v;

			p[UNIFORM_COLOR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::COLOR )->m_id;
			p[UNIFORM_ALPHA_TEST_THRESHOLD] = &c->m_material->m_alpha_test_threshold;
			uint32_t shader_code;

			if( c->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST )
			{
				shader_code = m_shadow_shaders[1];
			}
			else
			{
				if( c->m_mesh->m_vertex_matrix_indices.size() )
				{
					shader_code = m_shadow_shaders[2];
				}
				else
				{
					shader_code = m_shadow_shaders[0];
				}
			}

			nglDraw( job, NGL_TRIANGLES, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, c->m_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p );
		}
	}

	for( size_t i = 0; i < m_actors.size(); i++ )
	{
		for( size_t j = 0; j < m_actors[i]->m_meshes.size(); j++ )
		{
			if( m_actors[i]->m_meshes[j]->m_mesh->m_nodes.size() > 0 )
			{ //is animated?
				auto c = m_actors[i]->m_meshes[j];
				Mesh3 *gfxb_mesh3 = (Mesh3*)c->m_mesh;

				//p[UNIFORM_MODEL] = c->m_world_pom.v;
				KCL::Matrix4x4 m = c->m_world_pom * m_shadow_camera.GetViewProjection();
				p[UNIFORM_MVP] = m.v;

				p[UNIFORM_COLOR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::COLOR )->m_id;
				p[UNIFORM_ALPHA_TEST_THRESHOLD] = &c->m_material->m_alpha_test_threshold;
				p[UNIFORM_BONES] = &gfxb_mesh3->m_node_matrices[0];

				uint32_t shader_code;

				if( c->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST )
				{
					shader_code = m_shadow_shaders[1];
				}
				else
				{
					if( c->m_mesh->m_vertex_matrix_indices.size() )
					{
						shader_code = m_shadow_shaders[2];
					}
					else
					{
						shader_code = m_shadow_shaders[0];
					}
				}

				nglDraw( job, NGL_TRIANGLES, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, NGL_FRONT_SIDED, p );
			}
		}
	}

	nglEnd( job );
}

void Scene5Ovr::RenderGBuffer( KCL::uint32 job, const KCL::Camera2& cubemap_cam, const KCL::Camera2& cam, void** p )
{
	nglBegin( job, m_cmd_buffer );

	p[UNIFORM_SHADOW_MAP] = &m_shadow_texture;
	p[UNIFORM_ENVMAP0] = &m_prefiltered_cubemap_texture;
	p[UNIFORM_BRDF] = &m_integrate_brdf_lut_texture;

	nglDepthState( job, NGL_DEPTH_LESS, true );

	for( unsigned j = 0; j < m_main_frustum_cull->m_visible_meshes[OvrMainMeshFilter::MESH_OPAQUE].size(); ++j )
	{
		KCL::Mesh* c = m_main_frustum_cull->m_visible_meshes[OvrMainMeshFilter::MESH_OPAQUE][j];

		Mesh3 *gfxb_mesh3 = (Mesh3*)c->m_mesh;

		p[UNIFORM_MODEL] = c->m_world_pom.v;
		KCL::Matrix4x4 m = c->m_world_pom * cam.GetViewProjection();
		p[UNIFORM_MVP] = m.v;

		p[UNIFORM_COLOR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::COLOR )->m_id;
		p[UNIFORM_NORMAL_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::BUMP )->m_id;
		p[UNIFORM_SPECULAR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::MASK )->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &( (Material*)c->m_material )->m_alpha_test_threshold;

		uint32_t shader_code;
		if( c->m_material->m_opacity_mode == KCL::Material::OpacityMode::NO_OPACITY )
		{
			shader_code = m_gbuffer_shader;
		}
		else
		{
#ifndef DEFERRED_MODE
			continue;
#endif
			shader_code = m_gbuffer_alpha_shader;
		}

		if( c->m_mesh->m_nodes.size() > 0 )
		{ //is animated?
			p[UNIFORM_BONES] = &gfxb_mesh3->m_node_matrices[0];

			shader_code = m_skeletal_gbuffer_shader;
		}

		if( c->m_material->m_name.find( "altarB_mat3" ) != std::string::npos && m_osm.getIntOption( "ENABLE_TESSELLATION" ).m_option )
		{
			KCL::uint32 texid = ( (Material*)c->m_material )->GetTexture( KCL::Material::AUX0 )->m_id;
			p[UNIFORM_DISPLACEMENT_TEX] = &texid;
			//tess_factor: limit, scale, bias, max dist
			KCL::Vector4D tess_factor = KCL::Vector4D( 5.0f, 2.0f, 0.0f, 1.0f );
			p[UNIFORM_TESSELLATION_FACTOR] = tess_factor.v;// c->m_material->m_tessellation_factor.v;
			float cam_near = cam.GetNear();
			p[UNIFORM_CAM_NEAR] = &cam_near;
			KCL::Vector4D frustum_planes[6];
			for( int d = 0; d < 6; ++d )
			{
				frustum_planes[d] = cam.GetCullPlane( d );
			}
			p[UNIFORM_FRUSTUM_PLANES] = frustum_planes;

			//TODO is this needed at all?
			// Calculate FOV scale for adaptive tessellation
			float tessellation_fov_scale;
			{
				float aperture = (float)tanf( KCL::Math::Rad( cam.GetFov() / 2.0f ) ) * cam.GetNear();
				float left0 = -aperture * cam.GetAspectRatio();
				float right0 = aperture * cam.GetAspectRatio();
				float top0 = aperture;
				float bottom0 = -aperture;

				float w = right0 - left0;
				float h = top0 - bottom0;

				//values for 60 deg FOV (freecam) with 0.1m near plane dist
				float fov60_w = 0.205280095f;
				float fov60_h = 0.115470052f;

				tessellation_fov_scale = sqrtf( fov60_w / w  *  fov60_h / h );
			}
			float tessellation_viewport_scale = sqrtf( float( m_viewport_width ) / 1920.0f * float( m_viewport_height ) / 1080.0f );
			float tessellation_multiplier = 800.0f * tessellation_viewport_scale * tessellation_fov_scale;
			p[UNIFORM_TESSELLATION_MULTIPLIER] = &tessellation_multiplier;

			KCL::Matrix4x4 mv = c->m_world_pom * cam.GetView();
			KCL::Matrix4x4 inv_model;
			KCL::Matrix4x4::Invert4x4( c->m_world_pom, inv_model );
			p[UNIFORM_MV] = mv.v;
			p[UNIFORM_INV_MODEL] = inv_model.v;

			shader_code = m_gbuffer_tessellated_shader;

			//nglCustomAction( 0, 300 );
			nglDraw( job, NGL_PATCH3, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, c->m_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p );
			//nglCustomAction( 0, 301 );
		}
		else
		{
			nglDraw( job, NGL_TRIANGLES, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, c->m_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p );
		}
	}

#ifndef DEFERRED_MODE
	//separate job not to write depth and read it at the same time...
	//nglBegin( m_lighting_render, m_cmd_buffer );
	//RenderAlpha( m_lighting_render, cam, p );
	//nglEnd( m_lighting_render );

	RenderAlpha( job, cam, p );
#endif

#ifndef DEFERRED_MODE
	RenderSkybox( job, cam, p );
#endif

	nglEnd( job );
}

void Scene5Ovr::RenderAlpha( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	nglDepthState( job, NGL_DEPTH_LESS, false );

	for( unsigned j = 0; j < m_main_frustum_cull->m_visible_meshes[OvrMainMeshFilter::MESH_OPAQUE].size(); ++j )
	{
		KCL::Mesh* c = m_main_frustum_cull->m_visible_meshes[OvrMainMeshFilter::MESH_OPAQUE][j];

		Mesh3 *gfxb_mesh3 = (Mesh3*)c->m_mesh;

		p[UNIFORM_MODEL] = c->m_world_pom.v;
		KCL::Matrix4x4 m = c->m_world_pom * cam.GetViewProjection();
		p[UNIFORM_MVP] = m.v;

		p[UNIFORM_COLOR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::COLOR )->m_id;
		p[UNIFORM_NORMAL_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::BUMP )->m_id;
		p[UNIFORM_SPECULAR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::MASK )->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &( (Material*)c->m_material )->m_alpha_test_threshold;

		uint32_t shader_code;
		if( c->m_material->m_opacity_mode == KCL::Material::OpacityMode::NO_OPACITY )
		{
			continue;
		}
		else
		{
			shader_code = m_gbuffer_alpha_shader;
		}

		if( c->m_mesh->m_nodes.size() > 0 )
		{ //is animated?
			p[UNIFORM_BONES] = &gfxb_mesh3->m_node_matrices[0];

			shader_code = m_skeletal_gbuffer_shader;
		}

		if( c->m_material->m_name.find( "altarB_mat3" ) != std::string::npos && m_osm.getIntOption( "ENABLE_TESSELLATION" ).m_option )
		{
			KCL::uint32 texid = ( (Material*)c->m_material )->GetTexture( KCL::Material::AUX0 )->m_id;
			p[UNIFORM_DISPLACEMENT_TEX] = &texid;
			//tess_factor: limit, scale, bias, max dist
			KCL::Vector4D tess_factor = KCL::Vector4D( 5.0f, 2.0f, 0.0f, 1.0f );
			p[UNIFORM_TESSELLATION_FACTOR] = tess_factor.v;// c->m_material->m_tessellation_factor.v;
			float cam_near = cam.GetNear();
			p[UNIFORM_CAM_NEAR] = &cam_near;
			KCL::Vector4D frustum_planes[6];
			for( int d = 0; d < 6; ++d )
			{
				frustum_planes[d] = cam.GetCullPlane( d );
			}
			p[UNIFORM_FRUSTUM_PLANES] = frustum_planes;

			//TODO is this needed at all?
			// Calculate FOV scale for adaptive tessellation
			float tessellation_fov_scale;
			{
				float aperture = (float)tanf( KCL::Math::Rad( cam.GetFov() / 2.0f ) ) * cam.GetNear();
				float left0 = -aperture * cam.GetAspectRatio();
				float right0 = aperture * cam.GetAspectRatio();
				float top0 = aperture;
				float bottom0 = -aperture;

				float w = right0 - left0;
				float h = top0 - bottom0;

				//values for 60 deg FOV (freecam) with 0.1m near plane dist
				float fov60_w = 0.205280095f;
				float fov60_h = 0.115470052f;

				tessellation_fov_scale = sqrtf( fov60_w / w  *  fov60_h / h );
			}
			float tessellation_viewport_scale = sqrtf( float( m_viewport_width ) / 1920.0f * float( m_viewport_height ) / 1080.0f );
			float tessellation_multiplier = 800.0f * tessellation_viewport_scale * tessellation_fov_scale;
			p[UNIFORM_TESSELLATION_MULTIPLIER] = &tessellation_multiplier;

			KCL::Matrix4x4 mv = c->m_world_pom * cam.GetView();
			KCL::Matrix4x4 inv_model;
			KCL::Matrix4x4::Invert4x4( c->m_world_pom, inv_model );
			p[UNIFORM_MV] = mv.v;
			p[UNIFORM_INV_MODEL] = inv_model.v;

			shader_code = m_gbuffer_tessellated_shader;

			//nglCustomAction( 0, 300 );
			nglDraw( job, NGL_PATCH3, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, c->m_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p );
			//nglCustomAction( 0, 301 );
		}
		else
		{
			nglDraw( job, NGL_TRIANGLES, shader_code, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, c->m_material->m_is_two_sided ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p );
		}
	}
}

void Scene5Ovr::RenderSSAO( KCL::uint32 job, KCL::Camera2& cam, void** p )
{
	//ssao
	if( !m_osm.getIntOption( "SSAO_ENABLED" ).m_option )
	{
		return;
	}

	KCL::Vector4D inv_resolution( float( SSAO_DOWNSCALE ) / float( m_viewport_width ), float( SSAO_DOWNSCALE ) / ( float( m_viewport_height ) ), 0.0f, 0.0f );
	float SAO_projection_scale = (float)( ( m_viewport_width * 0.5f ) / ( 2.0f * tan( KCL::Math::Rad( cam.GetFov() * 0.5f ) ) ) );

	KCL::Vector4D m_vs_corners[4];
	cam.CalculateVSRaysToFullscreenBillboard( m_vs_corners );

	p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_gbuffer_depth_texture;
	p[UNIFORM_GBUFFER_NORMAL_TEX] = &m_gbuffer_normal_texture;
	p[UNIFORM_CORNERS] = m_vs_corners;
	p[UNIFORM_INV_RESOLUTION] = inv_resolution.v;
	p[UNIFORM_VIEW] = (void*)cam.GetView().v;
	p[UNIFORM_SAO_PROJECTION_SCALE] = &SAO_projection_scale;

	nglBegin( job, m_cmd_buffer );

	nglDraw( job, NGL_TRIANGLES, m_ssao_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p );

	nglEnd( job );

	// blur final texture
	if( m_ssao_blur )
	{
		NormalizeEffectParameters();

		m_ssao_blur->SetKernelSize( m_ssao_blur_strength );
		m_ssao_blur->SetInputTexture( m_ssao_texture );
		m_ssao_blur->RenderVerticalPass(m_cmd_buffer);
		m_ssao_blur->RenderHorizontalPass(m_cmd_buffer);
	}
}

void Scene5Ovr::RenderSkybox( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	nglDepthState( job, NGL_DEPTH_LESS, false );

	//skybox render here!
	{
		OvrMainMeshFilter::MeshType type = !m_eye ? OvrMainMeshFilter::MESH_SKY_LEFT : OvrMainMeshFilter::MESH_SKY_RIGHT;

		/*INFO( "%s eye", !m_eye ? "left" : "right" );
		for( unsigned c = 0; c < m_main_frustum_cull->m_visible_meshes.size(); ++c )
		{
		INFO( "%i: %i meshes", c, m_main_frustum_cull->m_visible_meshes[c].size() );
		}*/

		for( unsigned j = 0; j < m_main_frustum_cull->m_visible_meshes[type].size(); ++j )
		{
			KCL::Mesh* c = m_main_frustum_cull->m_visible_meshes[type][j];
			Mesh3 *gfxb_mesh3 = (Mesh3*)c->m_mesh;

			KCL::Matrix4x4 trans;
			trans.translate( -EYEPOS * cm_to_m );
			KCL::Matrix4x4 m = c->m_world_pom * trans;
			p[UNIFORM_MODEL] = m.v;
			KCL::Matrix4x4 m2 = c->m_world_pom * cam.GetViewProjection();
			p[UNIFORM_MVP] = m2.v;

			p[UNIFORM_COLOR_TEX] = &( (Material*)c->m_material )->GetTexture( KCL::Material::COLOR )->m_id;
			//p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_gbuffer_depth_texture;
			KCL::Vector4D texsize = KCL::Vector4D( 1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f );
			p[UNIFORM_TEXSIZE_SIZE] = texsize.v;

			nglDraw( job, NGL_TRIANGLES, m_cubemap_shader, 1, &gfxb_mesh3->m_vbid, gfxb_mesh3->m_ibid, NGL_FRONT_SIDED, p );
		}
	}
}

void Scene5Ovr::RenderLighting( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	nglBegin( job, m_cmd_buffer );

	RenderSkybox( job, cam, p );

	//fullscreen quad directional light render here
	{
		p[UNIFORM_GBUFFER_COLOR_TEX] = &m_gbuffer_color_texture;
		p[UNIFORM_GBUFFER_NORMAL_TEX] = &m_gbuffer_normal_texture;
		p[UNIFORM_GBUFFER_SPECULAR_TEX] = &m_gbuffer_specular_texture;
		p[UNIFORM_GBUFFER_DEPTH_TEX] = &m_gbuffer_depth_texture;

		p[UNIFORM_SHADOW_MAP] = &m_shadow_texture;
		p[UNIFORM_ENVMAP0] = &m_prefiltered_cubemap_texture;
		p[UNIFORM_BRDF] = &m_integrate_brdf_lut_texture;
		p[UNIFORM_SSAO_TEXTURE] = &m_ssao_texture;
		p[UNIFORM_BLURRED_SSAO_TEX] = m_ssao_blur->GetUniformOutputTexture();

		KCL::Matrix4x4 vp = cam.GetViewProjection();
		KCL::Matrix4x4 inv_vp;
		KCL::Matrix4x4::Invert4x4( vp, inv_vp );
		p[UNIFORM_INV_VP] = inv_vp.v;

		nglDraw( job, NGL_TRIANGLES, m_lighting_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p );
	}

	nglEnd( job );
}

void Scene5Ovr::RenderHDR( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	m_hdr_computer->SetInputTexture( m_lighting_tex );
	m_hdr_computer->Render(m_cmd_buffer);
}

void Scene5Ovr::RenderBloom( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	//compute bloom
	if( !m_osm.getIntOption( "ENABLE_BLOOM" ).m_option )
	{
		return;
	}

	m_bloom->SetColorCorrection( m_color_correction );
	//initial downsampling pass 1/1 --> 1/4
	m_bloom->ExecuteBrightPass( m_cmd_buffer, m_lighting_tex );

	///////////////////////////////////
	//mobile bloom
	///////////////////////////////////

	//1/4  --> 1/8
	//1/8  --> 1/16
	//1/16 --> 1/32
	//1/32 --> 1/64
	for( KCL::uint32 layer = 1; layer < m_bloom->GetLayerCount(); layer++ )
	{
		m_bloom->ExecuteDownsample( m_cmd_buffer, layer );
	}

	//1/64 + 1/32 --> 1/32
	//1/32 + 1/16 --> 1/16
	//1/16 + 1/8  --> 1/8
	//1/8         --> 1/4
	for( KCL::uint32 layer = 1; layer < m_bloom->GetLayerCount(); layer++ )
	{
		m_bloom->ExecuteUpsample( m_cmd_buffer, layer );
	}


	///////////////////////////////////
	///////////////////////////////////


	/*for( KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount() - 1; layer++ )
	{
		nglSubmit( m_bloom->ExecuteDownsample( layer ) );
	}

	for( KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount(); layer++ )
	{
		nglSubmit( m_bloom->ExectureVerticalBlur( layer ) );
	}
	for( KCL::uint32 layer = 0; layer < m_bloom->GetLayerCount(); layer++ )
	{
		nglSubmit( m_bloom->ExectureHorizontalBlur( layer ) );
	}*/
}

void Scene5Ovr::RenderFinal( KCL::uint32 job, const KCL::Camera2& cam, void** p )
{
	nglBegin( job, m_cmd_buffer );

	KCL::Matrix4x4 model;
	KCL::Matrix4x4 mvp;

	//void *p[UNIFORM_MAX];
	//memset( p, 0, sizeof( p ) );
	p[UNIFORM_MVP] = mvp.v;
	p[UNIFORM_MODEL] = model.v;

	p[UNIFORM_HDR_ABCD] = m_hdr_computer->GetUniformTonemapperConstants1();
	p[UNIFORM_HDR_EFW_TAU] = m_hdr_computer->GetUniformTonemapperConstants2();
	p[UNIFORM_HDR_TONEMAP_WHITE] = m_hdr_computer->GetUniformTonemapWhite();
	p[UNIFORM_HDR_EXPOSURE] = m_hdr_computer->GetUniformExposure();
	p[UNIFORM_BLOOM_PARAMETERS] = m_hdr_computer->GetUniformBloomParameters();
	p[UNIFORM_BLOOM_TEXTURE] = m_bloom->GetUniformBloomTexture( m_bloom->GetLayerCount() - 1 ); //layer=1?
	p[UNIFORM_COLOR_CORRECTION] = m_color_correction.v;

	//p[UNIFORM_COLOR_TEX] = &m_lighting_tex;
	p[UNIFORM_COLOR_TEX] = &m_lighting_tex;
	nglDraw( job, NGL_TRIANGLES, m_fullscreen_shader, 1, &m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, NGL_FRONT_SIDED, p );

	nglEnd( job );
}

void Scene5Ovr::Resize( KCL::uint32 x, KCL::uint32 y, KCL::uint32 w, KCL::uint32 h)
{
	SceneBase::Resize(x, y, w, h);

	KCL::int32 vp[4] = { 0, 0, (KCL::int32)w, (KCL::int32)h };
#ifndef WITH_OVR_SDK
	KCL::int32 vpr[4] = { ( KCL::int32 )w, 0, ( KCL::int32 )w, ( KCL::int32 )h };
#endif

	KCL::uint32 size[3] = { w, h, 0 };
	uint32_t textures[] =
	{
		m_gbuffer_color_texture, m_gbuffer_normal_texture, m_gbuffer_specular_texture, m_gbuffer_depth_texture, m_lighting_tex, m_ssao_texture, 
    };
	nglResizeTextures( COUNT_OF( textures ), textures, size );

	nglViewportScissor(m_lighting_render, vp, vp);
#ifdef WITH_OVR_SDK
	for( unsigned c = 0; c < m_final_render.size(); ++c )
	{
		nglViewportScissor( m_final_render[c], vp, vp );
	}
#else
	for( unsigned c = 0; c < m_final_render_left.size(); ++c )
	{
		nglViewportScissor( m_final_render_left[c], vp, vp );
		nglViewportScissor( m_final_render_right[c], vpr, vpr );
	}
#endif

	KCL::int32 ssao_vp[4] = { 0, 0, KCL::int32( w / SSAO_DOWNSCALE ), KCL::int32( h / SSAO_DOWNSCALE ) };
	nglViewportScissor( m_ssao_render, ssao_vp, ssao_vp );

	if( m_ssao_blur )
	{
		m_ssao_blur->Resize( 1, w / SSAO_DOWNSCALE, h / SSAO_DOWNSCALE );
	}
}

KCL::KCL_Status Scene5Ovr::ReloadShaders()
{
	InitShaders();

	m_hdr_computer->DeleteRenderers();
	m_bloom->DeletePipelines();

	nglDeletePipelines( m_lighting_render );
	nglDeletePipelines( m_deferred_render );
	nglDeletePipelines( m_shadow_job );
#ifdef WITH_OVR_SDK
	for( unsigned c = 0; c < m_final_render.size(); ++c )
	{
		nglDeletePipelines( m_final_render[c] );
	}
#else
	for( unsigned c = 0; c < m_final_render_left.size(); ++c )
	{
		nglDeletePipelines( m_final_render_left[c] );
		nglDeletePipelines( m_final_render_right[c] );
	}
#endif
	nglDeletePipelines( m_ssao_render );

	if( m_ssao_blur )
	{
		m_ssao_blur->DeletePipelines();
	}

	return KCL::KCL_TESTERROR_NOERROR;
}

void Scene5Ovr::ResizeShadowMap()
{
	KCL::uint32 size[3] = { (unsigned)m_osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option, (unsigned)m_osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option, 0 };
	uint32_t textures[] =
	{
		m_shadow_texture
	};
	nglResizeTextures( COUNT_OF( textures ), textures, size );
}

KCL::KCL_Status Scene5Ovr::Init()
{
	INFO( "Scene5Ovr::Init" );
	KCL::KCL_Status status = SceneBase::Init();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	InitOSMDefaults();

	/////////////////////////

	/*for( unsigned c = 0; c < this->m_rooms.size(); ++c )
	{
		for( unsigned d = 0; d < this->m_rooms[c]->m_meshes.size(); ++d )
		{
			if( this->m_rooms[c]->m_meshes[d]->m_name.find( "sky" ) != std::string::npos )
			{
				KCL::Vector3D diff = this->m_rooms[c]->m_meshes[d]->m_aabb.GetMaxVertex() - 
					this->m_rooms[c]->m_meshes[d]->m_aabb.GetMinVertex();

				for( unsigned e = 0; e < 3; ++e )
				{
					if( diff[e] < 0.1f )
					{
						KCL::Vector3D expand = this->m_rooms[c]->m_meshes[d]->m_aabb.GetMaxVertex();
						*((float*)&expand[e]) += 0.1f;
						this->m_rooms[c]->m_meshes[d]->m_aabb.Merge( expand );
					}
				}
			}
		}
	}*/

	int num_lefts = 0;
	int num_rights = 0;
	for( unsigned c = 0; c < this->m_rooms.size(); ++c )
	{
		for( unsigned d = 0; d < this->m_rooms[c]->m_meshes.size(); ++d )
		{
			if( this->m_rooms[c]->m_meshes[d]->m_name.find( "sky" ) != std::string::npos )
			{
				num_lefts += ( ( OvrMesh* )this->m_rooms[c]->m_meshes[d] )->m_is_left_eye_sky;
				num_rights += ( ( OvrMesh* )this->m_rooms[c]->m_meshes[d] )->m_is_right_eye_sky;
			}
		}
	}

	INFO( "lefts: %i \n rights: %i", num_lefts, num_rights );

	/////////////////////////

	m_shapes = new Shapes();
	m_shapes->Init();

	INFO("Init shaders...");
	InitShaders();

	int32_t scene_viewport[4] =
	{
		0, 0, (int32_t)m_viewport_width, (int32_t)m_viewport_height
	};
#ifndef WITH_OVR_SDK
	int32_t scene_viewport_right[4] =
	{
		(int32_t)m_viewport_width, 0, (int32_t)m_viewport_width, (int32_t)m_viewport_height
	};
#endif
	int32_t ssao_viewport[4] =
	{
		0, 0, (int32_t)m_viewport_width / (int32_t)SSAO_DOWNSCALE, (int32_t)m_viewport_height / (int32_t)SSAO_DOWNSCALE
	};

	//gbuffer
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue( 0.0f );

		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;

		texture_layout.m_name = "gbuffer_color";
		nglGenTexture( m_gbuffer_color_texture, texture_layout, nullptr );

		texture_layout.m_name = "gbuffer_specular";
		nglGenTexture( m_gbuffer_specular_texture, texture_layout, nullptr );

		texture_layout.m_name = "gbuffer_normal";
		nglGenTexture( m_gbuffer_normal_texture, texture_layout, nullptr );

		/*texture_layout.m_name = "gbuffer_velocity";
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_format = NGL_R16_G16_FLOAT;
		texture_layout.SetAllClearValue( 0.0f );
		nglGenTexture( m_gbuffer_velocity_texture, texture_layout, nullptr );*/

		texture_layout.m_name = "gbuffer_depth";
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_format = NGL_D24_UNORM;
		texture_layout.SetAllClearValue( 1.0f );
		nglGenTexture( m_gbuffer_depth_texture, texture_layout, nullptr );
	}

	{//gen tex
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "lighting";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM; //rgbm encoded
		//texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_is_renderable = true;
		texture_layout.m_num_levels = 1;
		texture_layout.m_clear_value[0] = 1.0f;
		texture_layout.m_clear_value[1] = .0f;
		texture_layout.m_clear_value[2] = 1.0f;
		texture_layout.m_clear_value[3] = 1.0f;

		m_lighting_tex = 0;
		nglGenTexture( m_lighting_tex, texture_layout, 0);

		Transitions::Get().Register( m_lighting_tex, texture_layout );
	}

	//SSAO texture
	{
		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "ssao";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_LINEAR;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = (uint32_t)ssao_viewport[2];
		texture_layout.m_size[1] = (uint32_t)ssao_viewport[3];
		texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue( 0.0f );

		nglGenTexture( m_ssao_texture, texture_layout, nullptr );
	}

	/*{//gen tex
		NGL_texture_descriptor texture_layout;

		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;
		texture_layout.m_format = NGL_D24_UNORM;
		//texture_layout.m_format = NGL_R16_G16_B16_A16_FLOAT;
		texture_layout.m_size[0] = m_viewport_width;
		texture_layout.m_size[1] = m_viewport_height;
		texture_layout.m_is_renderable = true;
		texture_layout.m_clear_value[0] = 1;
		texture_layout.m_num_levels = 1;

		main_depth_tex = 0;
		nglGenTexture(main_depth_tex, texture_layout, 0);
	}*/

	//SSAO job
	{
		NGL_attachment_descriptor ad;
		NGL_job_descriptor rrd;

		ad.m_attachment.m_idx = m_ssao_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		rrd.m_attachments.push_back( ad );

		rrd.m_load_shader_callback = LoadShader;

		NGL_subpass sp;
		sp.m_name = "ssao";
		sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );
		rrd.m_subpasses.push_back( sp );

		m_ssao_render = nglGenJob( rrd );
		nglViewportScissor( m_ssao_render, ssao_viewport, ssao_viewport );
	}
	
	//deferred job
	{
		NGL_job_descriptor rrd;

		NGL_subpass sp;
		sp.m_name = "deferred";
		//sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );

		rrd.m_load_shader_callback = LoadShader;

		//sp.m_usages.resize( 3 );

#ifndef DEFERRED_MODE
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_lighting_tex;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );
			//sp.m_usages[1] = NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE;
			//sp.m_usages[2] = NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE;
			rrd.m_attachments.push_back( ad );
		}
#endif

#ifdef DEFERRED_MODE
		ad.m_attachment = m_gbuffer_color_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
		ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
		ad.m_attachment_usages[0] = NGL_RENDERTARGET_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_SHADER_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_SHADER_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad );

		ad.m_attachment = m_gbuffer_normal_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		ad.m_attachment_usages[0] = NGL_RENDERTARGET_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_SHADER_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_SHADER_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad );

		ad.m_attachment = m_gbuffer_specular_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
		ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
		ad.m_attachment_usages[0] = NGL_RENDERTARGET_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_SHADER_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_SHADER_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad );
#endif

		/*ad.m_attachment = m_gbuffer_velocity_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		ad.m_attachment_usages[0] = NGL_RENDERTARGET_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_NO_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_NO_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad );*/
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_gbuffer_depth_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			sp.m_usages.push_back( NGL_DEPTH_ATTACHMENT );
			//sp.m_usages[1] = NGL_DEPTH_ATTACHMENT;
			//sp.m_usages[2] = NGL_DEPTH_ATTACHMENT;
			rrd.m_attachments.push_back( ad );
		}
		/*ad.m_attachment = m_direct_lighting_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		ad.m_attachment_usages[0] = NGL_NO_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_RENDERTARGET_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_NO_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad ); // 3

		ad.m_attachment = m_indirect_lighting_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		ad.m_attachment_usages[0] = NGL_NO_ATTACHMENT;
		ad.m_attachment_usages[1] = NGL_NO_ATTACHMENT;
		ad.m_attachment_usages[2] = NGL_RENDERTARGET_ATTACHMENT;
		rrd.m_color_attachments.push_back( ad ); // 4

		if( nglGetApi() == NGL_METAL_IOS )
		{
			ad.m_attachment = m_gbuffer_color_depth_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
			ad.m_attachment_usages[0] = NGL_RENDERTARGET_ATTACHMENT;
			ad.m_attachment_usages[1] = NGL_SHADER_ATTACHMENT;
			ad.m_attachment_usages[2] = NGL_SHADER_ATTACHMENT;
			rrd.m_color_attachments.push_back( ad ); // 5
		}*/

		rrd.m_subpasses.push_back( sp );

		m_deferred_render = nglGenJob( rrd );

		nglDepthState( m_deferred_render, NGL_DEPTH_LESS, true );
		nglViewportScissor( m_deferred_render, scene_viewport, scene_viewport );
	}

	//lighting job
	{
		NGL_attachment_descriptor ad;
		NGL_job_descriptor rrd;

		rrd.m_load_shader_callback = LoadShader;

		ad.m_attachment.m_idx = m_lighting_tex;
		//ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		rrd.m_attachments.push_back(ad);

		/*ad.m_attachment = main_depth_tex;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		rrd.m_depth_attachment.push_back(ad);*/

		NGL_subpass sp;
		sp.m_name = "lighting";
		sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );
		rrd.m_subpasses.push_back( sp );

		//nglCustomAction(0, 200);
		m_lighting_render = nglGenJob(rrd);
		nglDepthState(m_lighting_render, NGL_DEPTH_DISABLED, false);
		nglViewportScissor(m_lighting_render, scene_viewport, scene_viewport);
	}

	//final job
	{
#ifdef WITH_OVR_SDK
		m_final_render.reserve( m_backbuffers.size() );
		for( unsigned c = 0; c < m_backbuffers.size(); ++c )
		{
			KCL::uint32 backbuffer = 0;

			NGL_attachment_descriptor ad;
			NGL_job_descriptor rrd;

			rrd.m_load_shader_callback = LoadShader;

			ad.m_attachment.m_idx = m_backbuffers[c];
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			//ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR; //for debug
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back( ad );

			NGL_subpass sp;
			sp.m_name = "final";
			sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );
			rrd.m_subpasses.push_back( sp );

			//nglCustomAction(0, 200); //for debug
			backbuffer = nglGenJob( rrd );
			nglViewportScissor( backbuffer, scene_viewport, scene_viewport );
			m_final_render.push_back( backbuffer );
		}
#else
		m_final_render_left.reserve( m_backbuffers.size() );
		m_final_render_right.reserve( m_backbuffers.size() );
		for( unsigned c = 0; c < m_backbuffers.size(); ++c )
		{
			KCL::uint32 backbuffer_left = 0;
			KCL::uint32 backbuffer_right = 0;

			NGL_attachment_descriptor ad;
			NGL_job_descriptor rrd;

			rrd.m_load_shader_callback = LoadShader;

			ad.m_attachment.m_idx = m_backbuffers[c];
			ad.m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
			//ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR; //for debug
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back( ad );

			NGL_subpass sp;
			sp.m_name = "final";
			sp.m_usages.push_back( NGL_COLOR_ATTACHMENT );
			rrd.m_subpasses.push_back( sp );

			//nglCustomAction(0, 200); //for debug
			backbuffer_left = nglGenJob( rrd );
			nglViewportScissor( backbuffer_left, scene_viewport, scene_viewport );

			//nglCustomAction(0, 200); //for debug
			backbuffer_right = nglGenJob( rrd );
			nglViewportScissor( backbuffer_right, scene_viewport_right, scene_viewport_right );

			m_final_render_left.push_back( backbuffer_left );
			m_final_render_right.push_back( backbuffer_right );
		}
#endif
	}

	INFO("Init meshes...");
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		Mesh3 *gfxb_mesh = (Mesh3*)m_meshes[i];
		if (gfxb_mesh->UploadMesh() == false)
		{
			INFO("Can not upload mesh to NGL: %s", gfxb_mesh->m_name.c_str());
			return KCL::KCL_TESTERROR_UNKNOWNERROR;
		}
	}

	INFO( "Init materials..." );
	for( size_t i = 0; i < m_materials.size(); i++ )
	{
		Material *m = (Material*)m_materials[i];

		KCL::KCL_Status status = m->Init();
		if( status != KCL::KCL_TESTERROR_NOERROR )
		{
			INFO( "Can not init material: %s", m->m_name.c_str() );
			return status;
		}
	}

	//get sunlight properties
	KCL::Actor *actor;
	KCL::Light *light;
	for (size_t i = 0; i < this->m_actors.size(); i++)
	{
		bool found = false;

		actor = this->m_actors[i];
		for (size_t j = 0; j < actor->m_lights.size(); j++)
		{
			light = actor->m_lights[j];
			if (light->m_light_shape->m_light_type == KCL::LightShape::DIRECTIONAL)
			{
				m_sunlight_dir = KCL::Vector3D(light->m_world_pom.v[8], light->m_world_pom.v[9], light->m_world_pom.v[10]).normalize();
				//m_sunlight_dir = KCL::Vector3D( 0.764171f, 0.645013f, -0.000000f ).normalize();
				//m_sunlight_dir = KCL::Vector3D( 0.0f, 0.9f, -0.000000f ).normalize();
				m_light_color = KCL::Vector4D( light->m_light_shape->m_color * light->m_light_shape->m_intensity );
				found = true;
				break;
			}
		}

		if (found)
			break;
	}

	//init ibl
	if (m_integrate_brdf_lut_texture == 0)
	{
		INFO("Integrate BRDF look up texture...");
		m_integrate_brdf_lut_texture = CreateIntegrateBRDF_LUT(m_cmd_buffer, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, LUT_TEXTURE_SIZE);
	}

	if (m_prefiltered_cubemap_texture == 0)
	{
		INFO("Loading IBL cubemap: ibl.pvr");
		uint32_t size;
		KCL::KCL_Status status = LoadCubemapRGB9E5("ibl.pvr", m_prefiltered_cubemap_texture, size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			INFO("Can not load IBL texture!");
		}
	}

	//init hdr compute
	m_hdr_computer = new ComputeHDR();
	m_hdr_computer->Init(this, scene_viewport[2], scene_viewport[3], 4);
	
	const KCL::uint32 BLOOM_DOWNSCALE = 4;
	const KCL::uint32 BLOOM_STRENGTH = 10;
	float bloom_strength = BLOOM_STRENGTH;
	bloom_strength = (bloom_strength*scene_viewport[3]) / 1080; // normalize for actual resolution
	//m_bloom = new Bloom();
	//bloom not done in float...
	//m_bloom->Init(m_shapes, m_hdr_computer, uint32_t(scene_viewport[2] / float(BLOOM_DOWNSCALE)), uint32_t(scene_viewport[3] / float(BLOOM_DOWNSCALE)), 4, uint32_t(bloom_strength), NGL_R8_G8_B8_A8_UNORM);
	m_bloom = new BloomMobile();
	m_bloom->Init( m_shapes, m_hdr_computer, uint32_t( scene_viewport[2] / float( BLOOM_DOWNSCALE ) ), uint32_t( scene_viewport[3] / float( BLOOM_DOWNSCALE ) ), 5, 15, 8, NGL_R8_G8_B8_A8_UNORM);
	
	const Environment::Values &env_values = GetEnvironment()->GetValues();
	const ColorCorrectionValues &cc_values = env_values.m_color_correction_values;
	// Saturation
	m_color_correction.x = cc_values.m_saturation;

	// Color scale
	m_color_correction.y = cc_values.m_contrast;

	// Color bias
	float complementer = 1.0f - cc_values.m_contrast;
	m_color_correction.z = KCL::Math::interpolate(0.0f, complementer, cc_values.m_contrast_center);

	//shadow values
	{//gen shadow tex
		m_shadow_texture_viewport[0] = 0;
		m_shadow_texture_viewport[1] = 0;
		m_shadow_texture_viewport[2] = m_osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option;
		m_shadow_texture_viewport[3] = m_osm.getIntOption( "SHADOW_MAP_SIZE" ).m_option;

		NGL_texture_descriptor texture_layout;

		texture_layout.m_name = "Sun shadow";
		texture_layout.m_type = NGL_TEXTURE_2D;
		texture_layout.m_filter = NGL_NEAREST;
		texture_layout.m_wrap_mode = NGL_CLAMP_ALL;

		texture_layout.m_size[0] = (uint32_t)m_shadow_texture_viewport[2];
		texture_layout.m_size[1] = (uint32_t)m_shadow_texture_viewport[3];
		texture_layout.m_format = NGL_D24_UNORM;
		texture_layout.m_is_renderable = true;
		texture_layout.SetAllClearValue( 1.0f );

		m_shadow_texture = 0;
		nglGenTexture( m_shadow_texture, texture_layout, nullptr );
	}

	//shadow job
	{
		NGL_attachment_descriptor ad;
		NGL_job_descriptor rrd;

		rrd.m_load_shader_callback = LoadShader;

		ad.m_attachment.m_idx = m_shadow_texture;
		ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
		ad.m_attachment_store_op = NGL_STORE_OP_STORE;
		rrd.m_attachments.push_back( ad );

		NGL_subpass sp;
		sp.m_name = "final";
		sp.m_usages.push_back( NGL_DEPTH_ATTACHMENT );
		rrd.m_subpasses.push_back( sp );

		m_shadow_job = 0;
		m_shadow_job = nglGenJob( rrd );

		nglDepthState( m_shadow_job, NGL_DEPTH_LESS_WITH_OFFSET, true );
		nglViewportScissor( m_shadow_job, m_shadow_texture_viewport, m_shadow_texture_viewport );
	}

	//set up bias mx
	m_bias_matrix.translate( KCL::Vector3D( 0.5f, 0.5f, 0.5f ) );
	m_bias_matrix.scale( KCL::Vector3D( 0.5f, 0.5f, 0.5f ) );

	//set up shadow cam
	m_shadow_camera.Ortho( -15.0f, 15.0f, -15.0f, 15.0f, -10.0f, 20.0f );
	KCL::Vector3D campos = EYEPOS * cm_to_m;
	m_shadow_camera.LookAt( campos, campos - m_sunlight_dir, KCL::Vector3D( 0.0f, 1.0f, 0.0f ) );
	m_shadow_camera.Update();

	m_shadow_matrix = m_shadow_camera.GetViewProjection() * m_bias_matrix;

	//set up frustum culling
	m_main_mesh_filter = new OvrMainMeshFilter();
	m_main_frustum_cull = new FrustumCull( this, m_main_mesh_filter );

	//set up ssao blur
	m_ssao_blur = new FragmentBlur();
	m_ssao_blur->SetPrecision( "half3" );
	m_ssao_blur->SetComponentCount( 3 );
	m_ssao_blur->Init( "ssao blur", m_shapes, m_ssao_texture, m_viewport_width / SSAO_DOWNSCALE, m_viewport_height / SSAO_DOWNSCALE, m_ssao_blur_strength, NGL_R8_G8_B8_A8_UNORM, 1 );

	return KCL::KCL_TESTERROR_NOERROR;
}

void Scene5Ovr::NormalizeEffectParameters()
{
	m_ssao_blur_strength = SSAO_BLUR_STRENGTH;
	m_ssao_blur_strength = ( m_ssao_blur_strength * m_viewport_height ) / 1080; // normalize for actual resolution
}

void Scene5Ovr::InitOSMDefaults()
{
	switch( nglGetApi() )
	{
		case NGL_OPENGL:
		{
			m_osm.AddStaticIntOption( "SHADER_GLSL", true );
			break;
		}

		case NGL_OPENGL_ES:
		{
			m_osm.AddStaticIntOption( "SHADER_GLSL", true );
			break;
		}

		default:
			break;
	}

	switch( nglGetInteger( NGL_RASTERIZATION_CONTROL_MODE ) )
	{
		case NGL_ORIGIN_LOWER_LEFT:
		{
			m_osm.AddStaticIntOption( "NGL_ORIGIN_LOWER_LEFT", true );
			break;
		}
		case NGL_ORIGIN_UPPER_LEFT:
		{
			m_osm.AddStaticIntOption( "NGL_ORIGIN_UPPER_LEFT", true );
			break;
		}
		case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP:
		{
			m_osm.AddStaticIntOption( "NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP", true );
			break;
		}
		default: assert( 0 ); break;
	}

	switch( nglGetInteger( NGL_DEPTH_MODE ) )
	{
		case NGL_ZERO_TO_ONE:
		{
			m_osm.AddStaticIntOption( "NGL_ZERO_TO_ONE", true );
			break;
		}
		case NGL_NEGATIVE_ONE_TO_ONE:
		{
			m_osm.AddStaticIntOption( "NGL_NEGATIVE_ONE_TO_ONE", true );
			break;
		}
		default: assert( 0 ); break;
	}

	m_osm.AddStaticIntOption( "NORMAL_MAPPING_ENABLED", true );

	m_osm.AddStaticIntOption( "MAX_BONES", KCL::Mesh3::MAX_BONES * 3 );

	m_osm.AddStaticIntOption( "GAMMA_CORRECTION_FAST", true );

	/////////////////////////////////////////////////////////////
	m_osm.AddIntOption( "SSAO_ENABLED", false, 3.15f, 4 );
	m_osm.AddIntOption( "TONEMAPPER_FILMIC", true, 0.34f, 10 );
	m_osm.AddIntOption( "EXPOSURE_MANUAL", false, 0.88f, 7 );
	m_osm.AddIntOption( "ENABLE_DIRECT_LIGHTING", true, 0.34f, 9 );
	m_osm.AddIntOption( "ENABLE_SHADOW_MAPPING", true, 0.87f, 8 );
	//m_osm.AddIntOption( "SHADOW_MAP_SIZE", 2048, 0.03f, 6 );
	m_osm.AddIntOption( "SHADOW_MAP_SIZE", 1024, 0.03f, 6 );
	//osm.AddIntOption( "SHADOW_MAP_SIZE", 512 );
	m_osm.AddIntOption( "ENABLE_CONSTANT_AMBIENT", true, 0.001f, 1 );
	m_osm.AddIntOption( "ENABLE_HEMISPHERIC_AMBIENT", true, 0.0025f, 1 );
	m_osm.AddIntOption( "ENABLE_IBL_AMBIENT", true, 0.83f, 4 );
	m_osm.AddIntOption( "ENABLE_BLOOM", true, 1.58f, 8 );
	m_osm.AddIntOption( "ENABLE_TESSELLATION", true, 2.8f, 4 );
	//osm.AddIntOption( "ENABLE_ANISOTROPY", true );
	//osm.AddIntOption( "ANISOTROPY_SIZE", 4 );
	//osm.AddIntOption( "ANISOTROPY_SIZE", 8 );
	//osm.AddIntOption( "ANISOTROPY_SIZE", 16 );
}

void Scene5Ovr::InitShaders()
{
	ShaderFactory *shader_factory = ShaderFactory::GetInstance();

	//shader_factory->SetForceHighp( true );

	shader_factory->ClearGlobals();

	shader_factory->AddShaderSubdirectory( "shaders/scene5/shaders_ovr" );

	shader_factory->AddGlobalHeaderFile( "common.h" );

	m_osm.ApplyOptions();

	//shader_factory->AddGlobalDefine( "NORMAL_MAPPING_ENABLED" );

	switch( nglGetApi() )
	{
	case NGL_OPENGL:
	{
		shader_factory->SetGlobalHeader( "#version 430 core\n" );
		//shader_factory->AddGlobalDefine( "SHADER_GLSL" );
		break;
	}

	case NGL_OPENGL_ES:
	{
		std::stringstream sstream;
		sstream << "#version 310 es" << std::endl;

		//highp
		sstream << "precision highp float;" << std::endl;
		sstream << "precision highp int;" << std::endl;
		sstream << "precision highp sampler2D;" << std::endl;
		sstream << "precision highp sampler2DArray;" << std::endl;
		sstream << "precision highp sampler2DArrayShadow;" << std::endl;
		sstream << "precision highp samplerCube;" << std::endl;
		sstream << "precision highp samplerCubeShadow;" << std::endl;
		sstream << "precision highp image2D;" << std::endl;

		shader_factory->SetGlobalHeader( sstream.str().c_str() );

		//shader_factory->AddGlobalDefine( "SHADER_GLSL" );
		break;
	}

	case NGL_VULKAN:
	{
		shader_factory->SetGlobalHeader( "#version 430 core\n" );
		shader_factory->AddGlobalDefine( "SHADER_VULKAN" );
		break;
	}

	case NGL_DIRECT3D_12:
		shader_factory->AddGlobalDefine( "SHADER_HLSL" );
		break;

	case NGL_METAL_IOS:
		shader_factory->AddGlobalDefine( "SHADER_METAL_IOS" );
		shader_factory->AddGlobalDefine( "SHADER_METAL" );
		break;
	case NGL_METAL_MACOS:
		shader_factory->AddGlobalDefine( "SHADER_METAL_OSX" );
		shader_factory->AddGlobalDefine( "SHADER_METAL" );
		break;

	default:
		break;
	}

	switch( nglGetInteger( NGL_RASTERIZATION_CONTROL_MODE ) )
	{
	case NGL_ORIGIN_LOWER_LEFT: 
	{
		//shader_factory->AddGlobalDefine( "NGL_ORIGIN_LOWER_LEFT" ); break;
		break;
	}
	case NGL_ORIGIN_UPPER_LEFT:
	{
		//shader_factory->AddGlobalDefine( "NGL_ORIGIN_UPPER_LEFT" ); break;
		break;
	}
	case NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP: 
	{
		//shader_factory->AddGlobalDefine( "NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP" ); break;
		break;
	}
	default: assert( 0 ); break;
	}

	switch( nglGetInteger( NGL_DEPTH_MODE ) )
	{
	case NGL_ZERO_TO_ONE:
	{
		//shader_factory->AddGlobalDefine( "NGL_ZERO_TO_ONE" ); break;
		break;
	}
	case NGL_NEGATIVE_ONE_TO_ONE: 
	{
		//shader_factory->AddGlobalDefine( "NGL_NEGATIVE_ONE_TO_ONE" ); break;
		break;
	}
	default: assert( 0 ); break;
	}

#ifdef DEFERRED_MODE
	m_skeletal_gbuffer_shader = shader_factory->AddDescriptor( ShaderDescriptor( "skeletal.vert", "gbuffer.frag" ) );
	m_gbuffer_shader = shader_factory->AddDescriptor( ShaderDescriptor( "static.vert", "gbuffer.frag" ).SetName( "main" ) );
	m_gbuffer_alpha_shader = shader_factory->AddDescriptor( ShaderDescriptor( "static.vert", "gbuffer.frag" ).AddDefine("ALPHA_TEST") );
#else
	m_skeletal_gbuffer_shader = shader_factory->AddDescriptor( ShaderDescriptor( "skeletal.vert", "gbuffer_forward.frag" ) );
	m_gbuffer_shader = shader_factory->AddDescriptor( ShaderDescriptor( "static.vert", "gbuffer_forward.frag" ).SetName( "main" ) );
	m_gbuffer_alpha_shader = shader_factory->AddDescriptor( ShaderDescriptor( "static.vert", "gbuffer_forward.frag" ).AddDefine( "ALPHA_TEST" ) );
#endif

#ifdef DEFERRED_MODE
	m_gbuffer_tessellated_shader = shader_factory->AddDescriptor( ShaderDescriptor() 
		.SetVSFile("pre_tess_vertex.shader")
		.SetTCSFile("pre_tess_control.shader")
		.SetTESFile("pre_tess_eval.shader")
		.SetFSFile("gbuffer.frag")		
		);
#else
	m_gbuffer_tessellated_shader = shader_factory->AddDescriptor( ShaderDescriptor()
		.SetVSFile( "pre_tess_vertex.shader" )
		.SetTCSFile( "pre_tess_control.shader" )
		.SetTESFile( "pre_tess_eval.shader" )
		.SetFSFile( "gbuffer_forward.frag" )
		);
#endif

	m_lighting_shader = shader_factory->AddDescriptor(ShaderDescriptor("lighting.vert", "lighting.frag")
		//.AddDefineInt("SSAO_ENABLED", 1 ) 
		);

	m_cubemap_shader = shader_factory->AddDescriptor(ShaderDescriptor("cubemap.vert", "cubemap.frag"));
	m_fullscreen_shader = shader_factory->AddDescriptor(ShaderDescriptor("fullscreen.vert", "fullscreen.frag").AddHeaderFile("tonemapper.h")
		//.AddDefineInt("TONEMAPPER_FILMIC", 1)
		//.AddDefineInt("EXPOSURE_MANUAL", 1)
		);

	//shader
	{
		m_shadow_shaders[0] = ShaderFactory::GetInstance()->AddDescriptor( ShaderDescriptor( "shadow_caster.vert", "shadow_caster.frag" ) );
		m_shadow_shaders[1] = ShaderFactory::GetInstance()->AddDescriptor( ShaderDescriptor( "shadow_caster.vert", "shadow_caster.frag" ).AddDefine( "ALPHA_TEST" ) );

		// Skeletal mesh
		ShaderDescriptor skeletal_desc( "shadow_caster.vert", "shadow_caster.frag" );
		skeletal_desc.AddDefineInt( "SKELETAL", 1 );
		//skeletal_desc.AddDefineInt( "MAX_BONES", 3 * KCL::Mesh3::MAX_BONES ); // One bone needs 3 vec4
		m_shadow_shaders[2] = ShaderFactory::GetInstance()->AddDescriptor( skeletal_desc );
	}

	m_ssao_shader = shader_factory->AddDescriptor( ShaderDescriptor( "sao.vert", "sao.frag" ) );
}


Scheduler *Scene5Ovr::CreateScheduler()
{
	switch (nglGetApi())
	{
	case NGL_METAL_MACOS:
	case NGL_METAL_IOS:
	case NGL_DIRECT3D_12:
		//INFO("Create Multi Threaded Sceduler...");
		//return new MultiThreadedOnDemandScheduler(4);

	default:
		INFO("Create Single Threaded Sceduler...");
		return new SingleThreadedScheduler();
	}
}


KCL::TextureFactory &Scene5Ovr::TextureFactory()
{
	return *m_texture_factory;
}


GFXB::Mesh3Factory &Scene5Ovr::Mesh3Factory()
{
	return *m_mesh3_factory;
}