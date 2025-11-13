/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_scene_handler.h>
#include <kcl_io.h>
#include <kcl_mesh.h>
#include <kcl_material.h>
#include <kcl_light2.h>
#include <kcl_image.h>
#include <kcl_planarmap.h>
#include <kcl_animation4.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_particlesystem2.h>
#include <kcl_envprobe.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "ng/stringutil.h"

using namespace KCL;
using namespace std;

#include "progressbar.h"

void KCL::SceneHandler::Animate() //base!!!
{
	Vector3D eye, ref, up;
	AnimateParameter ap( Matrix4x4(), m_animation_time * m_animation_multiplier);
	AnimateParameter prev_ap( Matrix4x4(), m_animation_time  * m_animation_multiplier - 5);

	m_frame++;

	if( m_free_camera)
	{
		m_fps_camera->LookAt( m_camera_position, m_camera_ref, KCL::Vector3D(0, 1, 0));
		m_active_camera = m_fps_camera;

		m_camera_focus_distance = 5.0f;
		m_prev_vp = m_fps_camera->GetViewProjection() ;
	}
	else
	{
		{
			float fov;
			Matrix4x4 m;
			Vector3D fp;

			AnimateCamera( ap.time, m, fov, fp);
			eye.set(m.v[12], m.v[13], m.v[14]);
			ref.set(-m.v[8], -m.v[9], -m.v[10]);
			ref += eye;
			up.set(m.v[4], m.v[5], m.v[6]);

			m_animated_camera->Perspective( fov, m_viewport_width, m_viewport_height, m_CullNear, m_CullFar);
			m_animated_camera->LookAt(eye, ref, up);
			m_active_camera = m_animated_camera;

			Vector3D r = mult4x3( m_active_camera->GetView(), fp);
			m_camera_focus_distance = -r.z;
		}

		{
			float fov;
			Matrix4x4 m;
			Vector3D fp;

			AnimateCamera( prev_ap.time, m, fov, fp);
			eye.set(m.v[12], m.v[13], m.v[14]);
			ref.set(-m.v[8], -m.v[9], -m.v[10]);
			ref += eye;
			up.set(m.v[4], m.v[5], m.v[6]);

			KCL::Camera2 c;
			c.Perspective( fov, m_viewport_width, m_viewport_height, m_CullNear, m_CullFar);
			c.LookAt(eye, ref, up);
			c.Update();
			m_prev_vp = c.GetViewProjection();
		}
	}

	for( KCL::uint32 i = 0; i < m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		if(!actor->m_root)
		{
			continue;
		}

		if( actor->m_shader_track)
		{
			Vector4D v;
			float t = ap.time / 1000.0f;
			float tb = 0.0f;

			_key_node::Get( v, actor->m_shader_track, t, tb);

			actor->material_idx = 1 - v.x;

			for( KCL::uint32 j = 0; j < actor->m_meshes.size(); j++)
			{
				Mesh* m = actor->m_meshes[j];
				m->SetActiveMaterial( actor->material_idx);
			}
		}

		actor->m_root->animate( false, ap);
		actor->m_root->animate( true, prev_ap);
		actor->CalculateAABB();

		for( KCL::uint32 j = 0; j < actor->m_meshes.size(); j++)
		{
			Mesh* m = actor->m_meshes[j];
			if( m->m_mesh->m_vertex_matrix_indices.size())
			{
				m->m_mesh->UpdateNodeMatrices();
				m->m_world_pom = actor->m_root->m_world_pom;
			}
		}
		for( KCL::uint32 j = 0; j < actor->m_emitters.size(); j++)
		{
			Node *node = actor->m_emitters[j];
			if ( node->m_type == EMITTER1 || node->m_type == EMITTER2)
			{
				AnimatedEmitter* emitter = static_cast<AnimatedEmitter*>( node);
				emitter->Simulate( ap.time);
				emitter->DepthSort( *m_active_camera);
			}
		}
	}

	{
		std::set<KCL::Material*>::iterator i = m_materials_with_video.begin();

		while( i != m_materials_with_video.end())
		{

			(*i)->PlayVideo( m_animation_time * m_animation_multiplier / 1000.0f);
			i++;
		}
	}

	m_active_camera->SetNearFar( m_CullNear, m_CullFar);
	m_active_camera->Update();
}


void SceneHandler::AnimateCamera( KCL::uint32 time, Matrix4x4 &m, float &fov_, Vector3D &fp)
{
	Vector4D position;
	Vector4D orientation;
	Vector4D fov;
	Vector4D focus;
	float t = time / 1000.0f;
	float tb = 0.0f;
	Vector3D p;
	Quaternion q;
	int camera_index = 0;

	if (m_active_camera_index_track)
	{
		Vector4D index;
		_key_node::Get(index, m_active_camera_index_track, t, tb);
		camera_index = index.x;

		if (!m_camera_position_tracks[camera_index])
		{
			camera_index = 0;
		}
	}

	if (m_camera_position_tracks[camera_index])
	{
		_key_node::Get(position, m_camera_position_tracks[camera_index], t, tb);
	}
	if (m_camera_orientation_tracks[camera_index])
	{
		_key_node::Get(orientation, m_camera_orientation_tracks[camera_index], t, tb);
	}
	if( m_camera_fov_tracks[camera_index])
	{
		_key_node::Get( fov, m_camera_fov_tracks[camera_index], t, tb);
	}
	else
	{
		fov.x = 45;
	}

	if( m_scene_version == SV_25)
	{
		fov.x = 47.333f;
	}

	if( !m_camera_focus_position_focus_tracks[camera_index])
	{
		camera_index = 0;
	}

	if( m_camera_focus_position_focus_tracks[camera_index])
	{
		_key_node::Get( focus, m_camera_focus_position_focus_tracks[camera_index], t, tb);
	}

	p.set( position.x, position.y, position.z);
	m.translate( p);	
	
	q.set( orientation.w, orientation.x, orientation.y, orientation.z);
	q.getRotationMatrix(camRotateMatrix);

	camTranslateMatrix = m;
	camPitch.identity();
	camYaw.identity();

	if (m_scene_version == SV_40 || m_scene_version == SV_41)
	{
		m = camRotateMatrix * m;
	}
	else
	{
		Vector3D axis;
		float angle = 0.0f;
		q.toAngleAxis(angle, axis);
		m.rotate(-angle, axis);
	}

	fov_ = fov.x;
	fp.set( focus.x, focus.y, focus.z);
}

SceneHandler::SceneHandler() : m_progressPtr(NULL), m_light_dir(0.0,1.0,0.0), m_light_dir_orig(0.0,1.0,0.0)
{
	loadStatic = true;

	m_scene_version = KCL::SV_INVALID;
	//m_shader = 0;
	m_fps_camera = 0;
	m_animated_camera = 0;
	m_camera_near = 1.0f;
	m_fog_density = 0.0f;
	m_flameMaterial = 0;
	m_glowMaterial = 0;
	m_shadowCasterMaterial = 0;
	m_omniLightMaterial = 0;
	m_free_camera = false;
	m_play_time = 0;
	m_animation_time = 0;
	m_animation_multiplier = 1.0f;
	m_pvs = 0;
	
	m_viewport_x = 0;
	m_viewport_y = 0;
	m_viewport_height = 512;
	m_viewport_width = 512;
	m_CullNear = 0.001f;
	m_CullFar = 100.0f;

	m_frame = 0;

    m_tessellation_enabled = true ;

	m_color_texture_is_srgb = false;
	m_torches_enabled = false;
	m_soft_shadow_enabled = false;
	m_depth_of_field_enabled = false ;

	m_fps_cam_fov = 60.0f;
	m_fboEnvMap_size = 64;
	m_num_shadow_maps = 0;

	m_HDR_exposure = 0;
	m_active_camera_index_track = 0;
	for( uint32 i=0; i<32; i++)
	{
		m_camera_orientation_tracks[i] = 0;
		m_camera_position_tracks[i] = 0;
		m_camera_focus_position_focus_tracks[i] = 0;
		m_camera_fov_tracks[i] = 0;
	}

	m_fps_camera = new Camera2;
	m_active_camera = m_fps_camera;

	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;
	m_num_used_texture = -1;
	m_num_samples_passed = -1;
    m_pixel_coverage_sampleCount = 0;
    m_pixel_coverage_primCount = 0;
	m_num_instruction = -1;

	m_texture_compression_type = "";

	//m_spark_shader = 0;
	//m_spark_geometry_vbo = 0;
	//m_spark_geometry_ebo = 0;
	//m_spark_indices_count = 0;
	m_spark_geometry = 0;
	m_billboard_geometry = 0;
	m_lens_flare_mesh = 0;

	m_hud_target_actor = 0;
	m_mblur_enabled = false;
	m_mblurMaterial = 0;

	m_max_joint_num_per_mesh = MAX_DEFAULT_JOINT_NUM_PER_MESH;

	m_fps_camera->Perspective( m_fps_cam_fov, m_viewport_width, m_viewport_height, m_CullNear, m_CullFar);

	m_default_mesh_node_factory = new DefaultMeshNodeFactory();
	m_factory.RegisterFactory(m_default_mesh_node_factory, KCL::MESH);
}


SceneHandler::~SceneHandler()
{
	if( m_pvs)
	{
		for( KCL::uint32 i=0; i<m_rooms.size(); i++)
		{
			delete [] m_pvs[i];
		}
		delete [] m_pvs;
	}

	delete m_fps_camera;
	delete m_animated_camera;

	for (int i = 0; i < m_light_shapes.size(); ++i)
	{
		delete m_light_shapes[i];
	}
	
	for (size_t i = 0; i < m_irradiance_meshes.size(); ++i)
	{
		delete m_irradiance_meshes[i];
	}
	for(size_t i=0; i<m_sky_mesh.size(); ++i)
	{
		delete m_sky_mesh[i];
	}
	for(size_t i=0; i<m_meshes.size(); ++i)
	{
		delete m_meshes[i];
	}
	for(size_t i=0; i<m_rooms.size(); ++i)
	{
		delete m_rooms[i];
	}

	for(size_t i=0; i<m_portals.size(); ++i)
	{
		delete m_portals[i];
	}
	m_portals.clear();

	for(size_t i=0; i<m_room_connections.size(); ++i)
	{
		delete m_room_connections[i];
	}
	m_room_connections.clear();

	for(size_t i=0; i<m_actors.size(); ++i)
	{
		delete m_actors[i];
	}

	for(size_t i=0; i<m_materials.size(); ++i)
	{
		delete m_materials[i];
	}

	for(size_t i=0; i<m_textures.size(); ++i)
	{
		delete m_textures[i];
	}

    for(size_t i=0; i<m_occluders.size(); ++i)
	{
		delete m_occluders[i];
	}

	for(size_t i=0; i<m_mios2.size(); i++)
	{
		delete m_mios2[i];
	}

	for (size_t i=0; i<m_probes.size(); i++)
	{
		delete m_probes[i];
	}

	delete m_active_camera_index_track;
	delete m_HDR_exposure;

	for( uint32 i=0; i<32; i++)
	{
		delete m_camera_orientation_tracks[i];
		delete m_camera_position_tracks[i];
		delete m_camera_focus_position_focus_tracks[i];
		delete m_camera_fov_tracks[i];
	}

	delete m_lens_flare_mesh;
	delete m_default_mesh_node_factory;
}

std::string SceneHandler::ImagesDirectory() const
{
	std::string result = "images_";
	result += m_texture_compression_type;
	result += "/";
	return result;
}


std::string SceneHandler::EnvmapsDirectory() const
{
	std::string result = ImagesDirectory() + "envmaps/";
	return result;
}

void SceneHandler::AddGlow(KCL::Light* light, KCL::Actor* actor)
{
	//float w = 2.0f + light->m_intensity * 0.1f;
	float w = 1.5f;


	Mesh* m2 = GetMeshFactory().Create( "glow_mesh", light, actor);

	m2->SetMaterials( m_glowMaterial, m_glowMaterial);
	m2->m_mesh_variants[0] = Mesh3Factory().Create( "common/glow");
	m2->m_mesh = m2->m_mesh_variants[0];
	m2->m_mesh->ConvertToBillboard( -w, w, -w, w, 0.0f);
	m2->m_mesh->GenerateLOD( 1.0f);
#ifdef GFXB_DEPRECATED
	m2->m_color = light->m_diffuse_color * 0.5f;
#endif
	m2->CreateFlickeringAnimation( 22600, 5, 0.0f);

	m_meshes.push_back( m2->m_mesh);
}

KCL_Status SceneHandler::Process_Load_RTRT( uint32 max_uniform_num, const char* scenefile, int readingType)
{
	KCL::Mesh3::MAX_BONES = 32;

	if( strstr( scenefile, "_egypt."))
	{
		m_scene_version = SV_25;
	}
	else if( strstr( scenefile, "_trex."))
	{
		m_scene_version = SV_27;
	}
	else if (strstr(scenefile, "_martini.") || strstr(scenefile, "_snake.") || strstr(scenefile, "_bear.") || strstr(scenefile, "_bridge.") || strstr(scenefile, "_nbody.") || strstr(scenefile, "_bowl."))
	{
		m_scene_version = SV_40;
	}
	else
	{
		INFO( "Process_Load: invalid scene!");
	}

	LoadParams(scenefile);

	KCL_Status status = OnParamsLoaded();
	if ( status != KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	switch (status = g_os->LoadingCallback(0))
	{
		case KCL_TESTERROR_NOERROR:
			break;
		default:
			return status;
	}

    IncrementProgressTo(0.1f);

	KCL_Status t = ReadEntities(readingType);

    IncrementProgressTo(0.2f);

	if( t != KCL_TESTERROR_NOERROR)
	{
		return t;
	}

	Move(0);

	return KCL_TESTERROR_NOERROR;
}


bool SceneHandler::InitSceneVersion(const char* scenefile)
{
	if (strstr(scenefile, "_egypt."))
	{
		m_scene_version = SV_25;
	}
	else if (strstr(scenefile, "_trex."))
	{
		m_scene_version = SV_27;
	}
	else if (strstr(scenefile, "_3."))
	{
		m_scene_version = SV_30;
	}
	else if (strstr(scenefile, "_31."))
	{
		m_scene_version = SV_31;
	}
	else if (strstr(scenefile, "_4."))
	{
		m_scene_version = SV_40;
	}
	else if (strstr(scenefile, "_41."))
	{
		m_scene_version = SV_41;
	}
	else if (strstr(scenefile, "_5."))
	{
		m_scene_version = SV_50;
	}
	else if (strstr(scenefile, "_5_OVR."))
	{
		m_scene_version = SV_50_OVR;
	}
	else if (strstr(scenefile, "_vdb."))
	{
		m_scene_version = SV_VDB;
	}
	else if (strstr(scenefile, "adas."))
	{
		m_scene_version = SV_ADAS;
	}
	else
	{
		INFO("Process_Load: invalid scene!");
		return false;
	}
	return true;
}


KCL_Status SceneHandler::Process_Load(uint32 max_uniform_num, const char* scenefile)
{
	if (m_scene_version == SV_INVALID)
	{
		return KCL::KCL_TESTERROR_INVALID_SCENE_VERSION;
	}

	if( m_scene_version == SV_27)
	{
		m_mblur_enabled = true;
	}

	if( m_scene_version > SV_25)
	{
		if( max_uniform_num < 230)
		{
			m_max_joint_num_per_mesh = 16;
		}
	}

	KCL::Mesh3::MAX_BONES = 32;

	LoadParams(scenefile);

	KCL_Status status = OnParamsLoaded();
	if ( status != KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	if( m_torches_enabled)
	{
		m_glowMaterial = CreateGlowMaterial();
		m_materials.push_back(m_glowMaterial);
	}

	m_shadowCasterMaterial = CreateShadowCasterMaterial();
	if( m_shadowCasterMaterial)
	{
		m_materials.push_back(m_shadowCasterMaterial);
	}

	if( m_mblur_enabled && (m_scene_version == KCL::SV_27) )
	{
		m_mblurMaterial = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("mblur", NULL, NULL);
		assert(m_mblurMaterial);


		InitMaterial( m_mblurMaterial);

		m_materials.push_back(m_mblurMaterial);
	}

	if( m_scene_version == KCL::SV_25)
	{
		m_flameMaterial = CreateFlameMaterial();
		m_materials.push_back(m_flameMaterial);

		m_shadowStaticReceiverMaterial = CreateShadowReceiverMaterial();
		if( m_shadowStaticReceiverMaterial)
		{
			m_materials.push_back(m_shadowStaticReceiverMaterial);
		}
		m_planarReflectionMaterial = CreatePlanarReflectionMaterial();
		m_materials.push_back(m_planarReflectionMaterial);
	}
	else if( m_scene_version == KCL::SV_27)
	{
		m_shadowStaticReceiverMaterial = CreateShadowReceiverMaterial();
		if( m_shadowStaticReceiverMaterial)
		{
			m_materials.push_back(m_shadowStaticReceiverMaterial);
		}

		m_planarReflectionMaterial = CreatePlanarReflectionMaterial();
		m_materials.push_back(m_planarReflectionMaterial);

		m_smokeMaterial = CreateSmokeMaterial();
		m_materials.push_back(m_smokeMaterial);

		m_steamMaterial = CreateSteamMaterial();
		m_materials.push_back( m_steamMaterial);

		{
			m_particle_geometry = Mesh3Factory().Create( "common/particle");
			m_particle_geometry->ConvertToParticle( 100);
			m_particle_geometry->GenerateLOD( 100.0f);
			m_meshes.push_back( m_particle_geometry);
		}
	}
	else if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)
	{
		m_lensFlareMaterial = CreateLensFlareMaterial();
		m_materials.push_back(m_lensFlareMaterial);

		m_envMapGenMaterial = CreateEnvMapGenMaterial();
		m_materials.push_back(m_envMapGenMaterial);

		{
			m_instanced_fire_material = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("instanced_fire", NULL, NULL);
			assert(m_instanced_fire_material);

			m_materials.push_back( m_instanced_fire_material);

			InitMaterial( m_instanced_fire_material);
		}
		{
			m_instanced_smoke_material = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("instanced_smoke", NULL, NULL);
			assert(m_instanced_smoke_material);

			m_materials.push_back( m_instanced_smoke_material);

			InitMaterial( m_instanced_smoke_material);
		}
		{
			m_instanced_spark_material = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("instanced_spark", NULL, NULL);
			assert(m_instanced_spark_material);

            m_instanced_spark_material->m_textures[0] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/spark_sprite.png");

			m_textures.push_back( m_instanced_spark_material->m_textures[0] );

			m_materials.push_back( m_instanced_spark_material);

			InitMaterial( m_instanced_spark_material);
		}
		{
			m_lens_flare_mesh = GetMeshFactory().Create("lens_flare_mesh", 0, 0);

			m_lens_flare_mesh->SetMaterials( m_lensFlareMaterial, m_lensFlareMaterial);
			m_lens_flare_mesh->m_mesh_variants[0] = Mesh3Factory().Create( "common/lens_flare");
			m_lens_flare_mesh->m_mesh = m_lens_flare_mesh->m_mesh_variants[0];
			m_lens_flare_mesh->m_mesh->ConvertToLensFlare();
			m_lens_flare_mesh->m_mesh->GenerateLOD( 100.0f);

			m_meshes.push_back( m_lens_flare_mesh->m_mesh);
		}
		{
			m_spark_geometry = Mesh3Factory().Create("common/spark_geom");
			m_spark_geometry->ConvertToSpark();
			m_spark_geometry->GenerateLOD( 100.0f);

			m_meshes.push_back( m_spark_geometry);
		}
		{
			m_billboard_geometry = Mesh3Factory().Create( "common/billboard");
			m_billboard_geometry->ConvertToBillboard( -1, 1, -1, 1, 0);
			m_billboard_geometry->GenerateLOD( 100.0f);

			m_meshes.push_back( m_billboard_geometry);
		}
	}
	else if( m_scene_version == KCL::SV_40 || m_scene_version == KCL::SV_41)
	{
		m_smokeMaterial = CreateSmokeMaterial();
		m_materials.push_back(m_smokeMaterial);
	}
	else if (m_scene_version == KCL::SV_50)
	{
		Texture *default_texture = TextureFactory().CreateAndSetup(KCL::Texture_2D, (this->ImagesDirectory() + "/no_texture.tga").c_str());
		m_textures.push_back(default_texture);
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.1f);

	KCL_Status t = ReadEntities();

    IncrementProgressTo(0.2f);

	if( t != KCL_TESTERROR_NOERROR)
	{
		return t;
	}

	Move( 0);

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	for(size_t i=0; i<m_actors.size(); ++i)
	{
		Actor* a = m_actors[i];
		for(size_t j=0; j<a->m_lights.size(); ++j)
		{
			Light* light = a->m_lights[j];

			if( !light->m_has_glow)
			{
				continue;
			}

			//TODO: handle glow
			if ( m_scene_version != SV_50)
			{
				AddGlow(light, a);
			}
		}

		AnimateParameter ap( Matrix4x4(), 0);
		a->m_root->animate( false, ap);
		a->CalculateAABB();
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

#if 0
	AnimateParameter ap( Matrix4x4(), 1);

	for( KCL::uint32 i = 0; i < m_actors.size(); i++)
	{
		Actor *actor = m_actors[i];

		actor->m_root->animate( false, ap);

		for( int j = 100; j < 1000; j++)
		{
			for( KCL::uint32 k = 0; k < actor->m_emitters.size(); k++)
			{
				AnimatedEmitter *emitter = actor->m_emitters[k];
				emitter->Simulate( j);
			}
		}
		for( int j = 100; j > 0; j--)
		{
			for( KCL::uint32 k = 0; k < actor->m_emitters.size(); k++)
			{
				AnimatedEmitter *emitter = actor->m_emitters[k];
				emitter->Simulate( j);
			}
		}
	}
#endif

	OnLoaded();

    IncrementProgressTo(0.3f);
	ProgressBar::AdvanceProgress("Scene::Process Loaded");

	return KCL_TESTERROR_NOERROR;
}


KCL::Material* SceneHandler::CreateFlameMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("torch", NULL, NULL);
	assert(m);

	m->m_textures[0] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/torch.png");
    m->m_textures[1] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/noise2d.png");
    if(!m->m_textures[0] || !!m->m_textures[1])
    {
        //TODO: handle error!!!
    }

    m_textures.push_back( m->m_textures[0]);
    m_textures.push_back( m->m_textures[1]);

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateGlowMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("glow", NULL, NULL);
	assert(m);

	for( KCL::uint32 i=0; i<5; i++)
	{
		m->m_textures[i] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/glow.png");
        if(!m->m_textures[i])
        {
            //TODO: handle error!!!
        }

        m_textures.push_back( m->m_textures[i]);
	}

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateShadowCasterMaterial()
{
	KCL::Material *m = 0;

	if( m_shadow_method_str == "depth map(depth)")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_caster_depth_depth", NULL, NULL);
		assert(m);
	}
	else if( m_shadow_method_str == "depth map(color)")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_caster_depth_color", NULL, NULL);
		assert(m);
	}
	else if( m_shadow_method_str == "simple projective")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_caster_projective", NULL, NULL);
		assert(m);
	}

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateShadowReceiverMaterial()
{
	KCL::Material *m = 0;

	if( m_shadow_method_str == "depth map(depth)")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_receiver_depth_depth", NULL, NULL);
		assert(m);
	}
	else if( m_shadow_method_str == "depth map(color)")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_receiver_depth_color", NULL, NULL);
		assert(m);
	}
	else if( m_shadow_method_str == "simple projective")
	{
		m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("shadow_receiver_projective", NULL, NULL);
		assert(m);
	}

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateOmniLightMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("omni_light_material", NULL, NULL);
	assert(m);

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreatePlanarReflectionMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("planar_reflection", NULL, NULL);
	assert(m);

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateLensFlareMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("lens_flare", NULL, NULL);
	assert(m);

    m->m_textures[0] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/lensflaref.png");
    if(!m->m_textures[0])
    {
        //TODO: handle error!!!
    }
	m_textures.push_back(m->m_textures[0]);

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateEnvMapGenMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("env_map_gen", NULL, NULL);
	assert(m);

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateSmokeMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("smoke", NULL, NULL);
	assert(m);

    m->m_textures[0] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/steam_sprite.png");
	KCL::AssetFile world_png(ImagesDirectory() + std::string("world.png"));
	m->m_textures[1] = TextureFactory().CreateAndSetup(KCL::Texture_2D, world_png.getFilename().c_str());
    m->m_textures[2] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/smoke_mask.png");
    m->m_textures[3] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/smoke_normal.png");

	m_textures.push_back( m->m_textures[0] );

	if(m->m_textures[1])
	{
		m_textures.push_back( m->m_textures[1] );
	}
	else
	{
		m->m_textures[1] = 0;
	}

	m_textures.push_back( m->m_textures[2] );
	m_textures.push_back( m->m_textures[3] );

	InitMaterial( m);

	return m;
}


KCL::Material* SceneHandler::CreateSteamMaterial()
{
	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create("steam", NULL, NULL);
	assert(m);

    m->m_textures[0] = TextureFactory().CreateAndSetup( KCL::Texture_2D, "common/steam_sprite.png");
	m_textures.push_back(m->m_textures[0]);

	InitMaterial( m);

	return m;
}


void SceneHandler::InitMaterial( KCL::Material* m)
{
	if(!m)
	{
		return;
	}

	if( strstr( m->m_name.c_str(), "sky"))
	{
		m->m_material_type = KCL::Material::SKY;
	}
	else if( strstr( m->m_name.c_str(), "decal"))
	{
		m->m_material_type = KCL::Material::DECAL;
		if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)
		{
			m->m_is_decal = true;
		}
	}
	else if( strstr( m->m_name.c_str(), "torch"))
	{
		if (m_scene_version == KCL::SV_25)
		{
			m->m_material_type = KCL::Material::FLAME;
			m->m_is_transparent = true;

			m->m_defines.insert("AALIGNED");
			m->m_defines.insert("DEP_TEXTURING");
		}
	}
	else if( strstr( m->m_name.c_str(), "glow"))
	{
		m->m_material_type = KCL::Material::FLAME;
		m->m_is_transparent = true;
	}
	else if( strstr( m->m_name.c_str(), "lens_flare"))
	{
		m->m_material_type = KCL::Material::FLAME;
		m->m_is_transparent = true;
	}
	else if( strstr( m->m_name.c_str(), "foliage_mat"))
	{
		m->m_material_type = KCL::Material::SKY;
		m->m_defines.insert("ALPHA_TEST");
		m->m_defines.insert("FOG");
		m->m_defines.insert("MASK");
	}
	else if( strstr( m->m_name.c_str(), "vollight"))
	{
		m->m_material_type = KCL::Material::LIGHTSHAFT;

		m->m_is_transparent = true;
	}
	else if( strstr( m->m_name.c_str(), "armor_shield"))
	{
		m->m_specular_exponent = 64.0f;
		m->m_specular_intensity = 2.0f;
		m->m_reflect_intensity = 0.1f;
	}
	else if( strstr( m->m_name.c_str(), "armor"))
	{
		m->m_specular_exponent = 16.0f;
		m->m_specular_intensity = 2.5f;
		m->m_reflect_intensity = 0.5f;
	}
	else if( strstr( m->m_name.c_str(), "anubis"))
	{
		m->m_specular_exponent = 12.0f;
		m->m_specular_intensity = 3.0f;
		m->m_reflect_intensity = 0.85f;
	}
	else if( strstr( m->m_name.c_str(), "weapon"))
	{
		m->m_specular_exponent = 20.0f;
		m->m_specular_intensity = 1.5f;
		m->m_reflect_intensity = 0.1f;
	}
	else if( strstr( m->m_name.c_str(), "nagyter_fstatue_export") )
	{
		m->m_specular_exponent = 15.0f;
		m->m_specular_intensity = 2.5f;
		m->m_reflect_intensity = 0.8f;
	}
	else if( strstr( m->m_name.c_str(), "second_room_shaded_column") )
	{
		m->m_specular_exponent = 15.0f;
		m->m_specular_intensity = 2.5f;
		m->m_reflect_intensity = 0.8f;
	}
	else if( strstr( m->m_name.c_str(), "palm_leaf1") )
	{
		m->m_material_type = KCL::Material::FOLIAGE;

		m->m_defines.insert("ALPHA_TEST");
		m->m_defines.insert("REFLECTION");
		m->m_defines.insert("WIDE_DIFFUSE_CLAMP");

		m->m_specular_exponent = 128.0;
		m->m_specular_intensity = 1.5f;
		m->m_reflect_intensity = 0.4f;
	}
	else if( strstr( m->m_name.c_str(), "trees") ||
		strstr( m->m_name.c_str(), "jungle_01_Trans") ||
		strstr( m->m_name.c_str(), "jungle_02_Trans") ||
		strstr( m->m_name.c_str(), "palmhd_base")
		)
	{
		m->m_material_type = KCL::Material::FOLIAGE;

		m->m_defines.insert("ALPHA_TEST");
		m->m_defines.insert("REFLECTION");
		m->m_defines.insert("WIDE_DIFFUSE_CLAMP");

		m->m_specular_exponent = 32.0f;
		m->m_specular_intensity = 0.8f;
		m->m_reflect_intensity = 0.1f;
		m->m_diffuse_intensity = 0.3f;
	}
	else if( strstr( m->m_name.c_str(), "jungle_waterfall"))
	{
		m->m_material_type = KCL::Material::WATER;

		m->m_is_transparent = true;

		m->m_transparency = 1.0f;
		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.7f;

		m->m_defines.insert("TRANSPARENCY");
	}
	else if( strstr( m->m_name.c_str(), "jungle_water") )
	{
		m->m_material_type = KCL::Material::WATER;

		m->m_is_transparent = true;

		m->m_transparency = 0.45f;
		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.4f;

		m->m_defines.insert("TRANSPARENCY");
	}
	else if( strstr( m->m_name.c_str(), "trex"))
	{
		m->m_specular_exponent = 8.0f;
		m->m_specular_intensity = 0.75f;
		m->m_reflect_intensity = 1.0f;
	}
	else if( strstr( m->m_name.c_str(), "motor"))
	{
		m->m_specular_exponent = 40.0f;
		m->m_specular_intensity = 3.0f;
		m->m_reflect_intensity = 0.25f;
	}
	else if( strstr( m->m_name.c_str(), "biker"))
	{
		m->m_specular_exponent = 64.0f;
		m->m_specular_intensity = 1.0f;
		m->m_reflect_intensity = 0.4f;
	}
	else if( strstr( m->m_name.c_str(), "floor"))
	{
		m->m_reflect_intensity = 0.7f;
	}
	else if( strstr( m->m_name.c_str(), "sirkamra_falak"))
	{
		m->m_reflect_intensity = 0.7f;
	}
	else if( strstr( m->m_name.c_str(), "cobra_2_mat") || strstr( m->m_name.c_str(), "cobra1_mat"))
	{
		m->m_reflect_intensity = 0.85f;

		m->m_defines.insert("REFLECTION");
	}
	else if( strstr( m->m_name.c_str(), "robot_Armor_mat_specular2"))
	{
		m->m_defines.insert("STIPPLE");
		m->m_defines.insert("TRANSITION_EFFECT");

		m->m_specular_exponent = 256.0f;
		m->m_specular_intensity = 128.0;
		m->m_reflect_intensity = 0.7f;

		//TODO: serialize these...
		m->m_fresnel_params = KCL::Vector3D(6.0f, 10.0f, 1.0f);
	}
	else if( strstr( m->m_name.c_str(), "robot_Body_mat_specular2"))
	{
		m->m_defines.insert("STIPPLE");
		m->m_defines.insert("TRANSITION_EFFECT");

		m->m_specular_exponent = 64.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 0.15f;

		m->m_fresnel_params = KCL::Vector3D(6.0f, 10.0f, 1.0f);
	}
	else if( strstr( m->m_name.c_str(), "robot_Armor"))
	{
		m->m_specular_exponent = 256.0f;
		m->m_specular_intensity = 128.0;
		m->m_reflect_intensity = 0.7f;

		m->m_fresnel_params = KCL::Vector3D(6.0f, 10.0f, 1.0f);
	}
	else if( strstr( m->m_name.c_str(), "robot_Body"))
	{
		m->m_specular_exponent = 64.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 0.15f;

		m->m_fresnel_params = KCL::Vector3D(6.0f, 10.0f, 1.0f);
	}
	else if( strstr( m->m_name.c_str(), "heli_mat_specular"))
	{
		m->m_fresnel_params = KCL::Vector3D(16.0f, 10.0f, 0.0f);
	}
	else if(
		strstr( m->m_name.c_str(), "ford_mat") ||
		strstr( m->m_name.c_str(), "truck_Texture_mat") ||
		strstr( m->m_name.c_str(), "kombi_mat") ||
		strstr( m->m_name.c_str(), "taxi_mat") ||
		strstr( m->m_name.c_str(), "greyCar_mat")
		)
	{
		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 16.0;
		m->m_reflect_intensity = 0.75f;
	}
	else if( strstr( m->m_name.c_str(), "walkway"))
	{
		m->m_specular_exponent = 32.0f;
		m->m_specular_intensity = 16.0;
		m->m_reflect_intensity = 0.1f;
	}
	else if( strstr( m->m_name.c_str(), "muzzle"))
	{
		m->m_material_type = KCL::Material::FLAME;

		m->m_is_transparent = true;
		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.5f;
	}
	else if( strstr( m->m_name.c_str(), "parallax"))
	{
		m->m_specular_exponent = 1024.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.0f;

		m->m_defines.insert("RELIEF");
		m->m_defines.insert("DEP_TEXTURING");
	}
	else if( strstr( m->m_name.c_str(), "ground"))
	{
		m->m_specular_exponent = 1024.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 0.0f;
	}
	else if( strstr( m->m_name.c_str(), "scene_background_lambert110"))
	{
		m->m_material_type = KCL::Material::FLAME;

		m->m_is_transparent = true;
		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.5f;
	}
	else if( strstr( m->m_name.c_str(), "ms_text"))
	{
		m->m_defines.insert("ALPHA_TEST");

		m->m_specular_exponent = 512.0f;
		m->m_specular_intensity = 1.0;
		m->m_reflect_intensity = 0.5f;
	}
	else if( strstr( m->m_name.c_str(), "nagyter_water"))
	{
		m->m_material_type = KCL::Material::WATER;
		m->m_is_transparent = true;
		m->m_transparency = 10.0f;
		m->m_specular_exponent = 64.0f;
		m->m_specular_intensity = 10.0;
		m->m_reflect_intensity = 0.5f;

		m->m_defines.insert("ANIMATE_NORMAL");
		m->m_defines.insert("TRANSPARENCY");
	}
	else if( strstr( m->m_name.c_str(), "animatik_helicopter_heli_roto"))
	{
		m->m_defines.insert("TRANSPARENCY");
		m->m_is_transparent = true;
		m->m_transparency = 0.5f;
		m->m_reflect_intensity = 0.0f;
		m->m_is_decal = true;

		m->m_specular_exponent = 256.0f;
		m->m_specular_intensity = 128.0;
		m->m_reflect_intensity = 0.7f;
		m->m_material_type = KCL::Material::GLASS;
	}
	else if( strstr( m->m_name.c_str(), "heli_glass"))
	{
		m->m_defines.insert("TRANSPARENCY");
		m->m_material_type = KCL::Material::GLASS;
		m->m_is_transparent = true;
		m->m_transparency = 10.0f;

		m->m_specular_exponent = 1024.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 0.25f;
	}
	else if( strstr( m->m_name.c_str(), "glass") || strstr( m->m_name.c_str(), "car_headlight") || strstr( m->m_name.c_str(), "car_red_light"))
	{
		m->m_defines.insert("TRANSPARENCY");
		m->m_material_type = KCL::Material::GLASS;
		m->m_is_transparent = true;
		m->m_transparency = 0.25f;

		m->m_specular_exponent = 1024.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 1.0f;
	}
	else if( strstr( m->m_name.c_str(), "shadow_mat") && (m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31))
	{
		m->m_defines.insert("TRANSPARENCY");
		m->m_material_type = KCL::Material::GLASS;
		m->m_is_transparent = true;
		m->m_is_decal = true;
		m->m_transparency = 1.0f;

		m->m_specular_exponent = 1024.0f;
		m->m_specular_intensity = 4.0;
		m->m_reflect_intensity = 0.0f;
	}
	else if( strstr( m->m_name.c_str(), "trash1"))
	{
		m->m_defines.insert("ALPHA_TEST");
	}
	else if( strstr( m->m_name.c_str(), "scrap"))
	{
		m->m_defines.insert("ALPHA_TEST");
	}
	else if( strstr( m->m_name.c_str(), "zebra"))
	{
		m->m_defines.insert("ALPHA_TEST");
	}
	else if( strstr( m->m_name.c_str(), "car_xray_light_mat"))
	{
		m->m_material_type = KCL::Material::XRAY;
		m->m_defines.insert("XRAY");
		m->m_is_transparent = true;
		m->m_reflect_intensity = 3.8f;
	}
	else if( strstr( m->m_name.c_str(), "car_xray_dark_mat"))
	{
		m->m_material_type = KCL::Material::XRAY;
		m->m_defines.insert("XRAY");
		m->m_is_transparent = true;
		m->m_reflect_intensity = 3.8f;
	}
	else if( strstr( m->m_name.c_str(), "car_xray_red_mat"))
	{
		m->m_material_type = KCL::Material::XRAY;
		m->m_defines.insert("XRAY");
		m->m_is_transparent = true;
		m->m_reflect_intensity = 0.5f;
	}
	else if( strstr( m->m_name.c_str(), "xray"))
	{
		m->m_material_type = KCL::Material::XRAY;
		m->m_defines.insert("XRAY");
		m->m_is_transparent = true;
		m->m_reflect_intensity = 0.4f;
	}

	if( strstr( m->m_name.c_str(), "specular"))
	{
		m->m_specular_exponent = 256.0f;
		m->m_specular_intensity = 8.0;
		m->m_reflect_intensity = 1.0f;
	}

	if( strstr( m->m_name.c_str(), "alphatest"))
	{
		m->m_defines.insert("ALPHA_TEST");
	}

	if(m->m_fresnel_params.x > 0.0f) //intensity
	{
		m->m_defines.insert("FRESNEL");
	}
}


KCL::Material* SceneHandler::CreateMaterial( const char *name)
{
	ProgressBar::AdvanceProgress();
	for( KCL::uint32 i=0; i<m_materials.size(); i++)
	{
		if( m_materials[i]->m_name == std::string( name))
		{
			return m_materials[i];
		}
	}

	KCL::Material *m = (KCL::Material*)m_factory.GetFactory(KCL::MATERIAL)->Create(name, NULL, NULL);
	assert(m);

	if (ParseMaterial(m, name))
	{
		m_materials.push_back( m);
		m->LoadParameters();
		return m;
	}
	else
	{
		delete m;
		m_materials[0]->LoadParameters();
		return m_materials[0];
	}
}

bool SceneHandler::ParseMaterial( KCL::Material *m, const char *name)
{
	m->SetDefaults();

	std::string matFileName;
	AssetFile *mat_file;
	if (KCL::AssetFile::Exists(std::string("materials/") + name))
	{
		mat_file = new AssetFile(std::string("materials/") + name);
	}
	else if (KCL::AssetFile::Exists(std::string("engine_materials/") + name))
	{
		mat_file = new AssetFile(std::string("engine_materials/") + name);
	}
	else
	{
		INFO("Material %s not found, replacing with missing_mat!\n", name);
		mat_file = new AssetFile("materials/missing_mat");
	}
	matFileName = mat_file->getFilename();

	//NOTE: ParseMaterialConfig a talalt file konyvtaraban keres csak
	bool skip_init_material = false;
	if (m_scene_version < SV_50)
	{
		skip_init_material = ParseMaterialConfig(m, matFileName.c_str());
	}

	if( !mat_file || !mat_file->Opened())
	{
		return false;
	}

	stringstream line_stream(mat_file->GetBuffer());
	string line;

	vector<string> tokens;
	while( getline(line_stream, line))
	{
		try
		{
			stringstream token_stream(line);

			tokens.clear();
			copy( istream_iterator<std::string>(token_stream),
				istream_iterator<std::string>(),
				back_inserter(tokens));
			if ( tokens.empty())
			{
				// Empty line
				continue;
			}

			if( tokens.at(0) ==  "array")
			{
				const std::string postfix = "_0001.png";
				string fn;
				string first_filename = tokens.at(1) + postfix;
				CheckImage( first_filename, ImagesDirectory(), fn);
				if( fn.length() >= postfix.length())
				{
					fn.resize( fn.length() - postfix.length());
				}
				m->m_texture_array = TextureFactory().CreateAndSetup( KCL::Texture_Array, fn.c_str());
				continue;
			}
			if (tokens.at(0) == "emissive")
			{
				ReadAnimation(m->m_emissive_track, tokens.at(1));
				continue;
			}
			if( tokens.at(0) == "u_animation")
			{
				ReadAnimation( m->m_translate_u_track, tokens.at(1));
				continue;
			}
			if( tokens.at(0) == "v_animation")
			{
				ReadAnimation( m->m_translate_v_track, tokens.at(1));
				continue;
			}
			if( tokens.at(0) == "theora")
			{
				m->LoadVideo(std::string("videos/" + tokens.at(1)).c_str());
				m_materials_with_video.insert( m);
				continue;
			}
			if (ParseMaterialTexture(m, tokens))
			{
				continue;
			}

			// Unknown token
			throw std::runtime_error("Parse error");
		}
		catch (KCL::IOException ioe)
		{
            INFO("Error in material %s in line: %s\n%s", matFileName.c_str(), line.c_str(), ioe.what());
		}
		catch (std::exception ex)
		{
			INFO("Error in material %s in line: %s\n%s", matFileName.c_str(), line.c_str(), ex.what());
		}
		catch (...)
		{
			INFO("Error in material %s can not parse line: %s", matFileName.c_str(), line.c_str());
		}
	}

	if (!skip_init_material)
	{
		if (m_scene_version == SV_40 || m_scene_version == SV_41)
		{
			INFO("Warning! Can not find config file for material: %s", matFileName.c_str());
		}
		InitMaterial( m);
	}

	delete mat_file;

	return true;
}

bool SceneHandler::ParseMaterialConfig( KCL::Material * m, const char *filename)
{
	string setup_filename = filename;
	setup_filename += ".cfg";

	if (!AssetFile::Exists(setup_filename.c_str()))
	{
		return false;
	}
	AssetFile mat_file(setup_filename.c_str());

	stringstream line_stream(mat_file.GetBuffer());
	string line;

	vector<string> tokens;
	bool material_type_set = false;
	while( getline(line_stream, line))
	{
		try
		{
			stringstream token_stream(line);

			tokens.clear();
			copy( istream_iterator<std::string>(token_stream),
				istream_iterator<std::string>(),
				back_inserter(tokens));
			if ( tokens.empty())
			{
				// Empty line
				continue;
			}
			if( tokens.at(0) == "material_type")
			{
				if (!material_type_set && Material::GetMaterialTypeByName(tokens.at(1), m->m_material_type))
				{
					material_type_set = true;
					continue;
				}
				else
				{
					std::string error = "Unknown material type: " + tokens.at(1);
					throw std::runtime_error(error);
				}
			}

			int shader_index = Material::GetShaderIndexTypeByName(tokens.at(0));
			if ( shader_index >= 0)
			{
				m->m_shader_names[shader_index] = tokens.at(1);
				continue;
			}

			int define_index = Material::GetDefineIndexTypeByName(tokens.at(0));
			if ( define_index >= 0)
			{
				std::string define_list = tokens.at(1);
				size_t define_list_length = define_list.length();
				if ( define_list_length < 3 ||define_list[0] != '"' || define_list[define_list_length - 1] != '"')
				{
					throw std::runtime_error( "Invalid define list");
				}
                std::string define_str = define_list.substr( 1, define_list_length - 2);
				m->m_shader_defs[define_index].insert( define_str);

                // TODO: Remove these tempolary warnings!
                if (define_str == "HAS_CAR_AO")
                {
			        INFO("!!!! WARNING: %s %s is DEPRECATED!. Use: has_car_ao [0|1] !!!!", setup_filename.c_str(), line.c_str());
                    m->m_has_car_ao = true;
                }
                if (define_str == "HAS_CAR_PAINT")
                {
                    INFO("!!!! WARNING: %s %s is DEPRECATED!. Use: is_car_paint [0|1] !!!!", setup_filename.c_str(), line.c_str());
                    m->m_is_car_paint = true;
                }
                if (define_str == "IS_CAR")
                {
                    INFO("!!!! WARNING: %s %s is DEPRECATED!. Use: use_world_ao [0|1] !!!!", setup_filename.c_str(), line.c_str());
                    m->m_use_world_ao = true;
                }
                if (define_str == "IS_BILLBOARD")
                {
                    INFO("!!!! WARNING: %s %s is DEPRECATED!. Use: billboard [0|1] !!!!", setup_filename.c_str(), line.c_str());
                    m->m_is_billboard = true;
                }
                if (define_str == "USE_TESSELLATION")
                {
                    INFO("!!!! WARNING: %s %s is DEPRECATED!. Use: tesselated [0|1] !!!!", setup_filename.c_str(), line.c_str());
                    if (m_tessellation_enabled)
                    {
                        m->m_is_tesselated = true;
                    }
                }
                //////////////////
                continue;
			}

            //GFXB4
			if (tokens.at(0) == "aniso")
			{
                m->m_is_aniso = atoi( tokens.at(1).c_str());
				continue;
			}
			if (tokens.at(0) == "has_emissive")
			{
                m->m_has_emissive_channel = tokens.at(1) != "0";
				continue;
			}
			if (tokens.at(0) == "disp_opacity_mode")
			{
                if(tokens.at(1) == "alphatest" || tokens.at(1) == "1")
                {
                    m->m_opacity_mode = Material::ALPHA_TEST;
                }
                else if(tokens.at(1) == "alphablend" || tokens.at(1) == "2")
                {
                    m->m_material_type = KCL::Material::GLASS;
                    m->m_opacity_mode = Material::ALPHA_BLEND;
                    m->m_is_transparent = true;
                }
                else if(tokens.at(1) == "displacement_local" || tokens.at(1) == "3")
                {
					if(m_tessellation_enabled)
                    {
                        m->m_displacement_mode = Material::DISPLACEMENT_LOCAL;
                        m->m_is_tesselated = true;
					}
                }
				else if (tokens.at(1) == "displacement_bezier" || tokens.at(1) == "displacement_abs" || tokens.at(1) == "4")
                {
					if(m_tessellation_enabled)
                    {
                        m->m_displacement_mode = Material::DISPLACEMENT_ABS;
                        m->m_is_tesselated = true;
					}
                }
				continue;
			}
            if (tokens.at(0) == "tesselated")
			{
                m->m_is_tesselated = tokens.at(1) != "0" && m_tessellation_enabled;
				continue;
			}
            if (tokens.at(0) == "tessellation_scale")
			{
				m->m_tessellation_factor.y = ng::atof(tokens.at(1).c_str());
				continue;
			}
            if (tokens.at(0) == "tessellation_bias")
			{
                m->m_tessellation_factor.z = ng::atof( tokens.at(1).c_str());
				continue;
			}
            if (tokens.at(0) == "tessellation_max_distance")
			{
				m->m_tessellation_factor.w = ng::atof(tokens.at(1).c_str());
                if(m->m_tessellation_factor.w == 0.0)
                {
                    m->m_tessellation_factor.w = 1000000.0;
                }
				continue;
			}
			if (tokens.at(0) == "shadow_caster")
			{
				m->m_is_shadow_caster = atoi( tokens.at(1).c_str());
				continue;
			}
			if (tokens.at(0) == "reflection_caster")
			{
				m->m_is_reflection_caster = atoi( tokens.at(1).c_str());
				continue;
			}
			if (tokens.at(0) == "shadow_only")
			{
				m->m_is_shadow_only = atoi( tokens.at(1).c_str());
				continue;
			}
			if (tokens.at(0) == "billboard")
			{
                m->m_is_billboard = atoi( tokens.at(1).c_str());
				continue;
			}
			if (tokens.at(0) == "translucent_lighting_strength")
			{
                m->m_translucent_lighting_strength = ng::atof( tokens.at(1).c_str());
				continue;
			}
            if (tokens.at(0) == "two_sided")
			{
				m->m_is_two_sided = atoi( tokens.at(1).c_str());
				continue;
			}
            if (tokens.at(0) == "is_car_paint")
			{
                m->m_is_car_paint = tokens.at(1) != "0";
				continue;
			}
            if (tokens.at(0) == "use_world_ao")
			{
                m->m_use_world_ao = tokens.at(1) != "0";
				continue;
			}
            if (tokens.at(0) == "has_car_ao")
			{
                m->m_has_car_ao = tokens.at(1) != "0";
				continue;
			}
			if (tokens.at(0) == "is_occluder")
			{
                m->m_is_occluder = tokens.at(1) != "0";
				continue;
			}

            //NOT GFXB4
			if (tokens.at(0) == "diffuse_intensity")
			{
				m->m_diffuse_intensity = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) == "specular_intensity")
			{
				m->m_specular_intensity = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "reflect_intensity")
			{
				m->m_reflect_intensity = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "specular_exponent")
			{
				m->m_specular_exponent = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "fresnel_intensity")
			{
				m->m_fresnel_params.x = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "fresnel_exponent")
			{
				m->m_fresnel_params.y = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "fresnel_colorboost")
			{
				m->m_fresnel_params.z = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "transparency")
			{
				m->m_transparency = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "is_transparent")
			{
				m->m_is_transparent = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if( tokens.at(0) ==  "is_reflective")
			{
				m->m_is_reflective = ng::atof(tokens.at(1).c_str());
				continue;
			}
			if (ParseMaterialTexture(m, tokens))
			{
				continue;
			}

			// Unknown token
			throw std::runtime_error("Parse error");
		}
		catch (KCL::IOException ioe)
		{
			INFO("Error in material %s in line: %s\n%s", setup_filename.c_str(), line.c_str(), ioe.what());
		}
		catch (std::exception ex)
		{
			INFO("Error in material %s in line: %s\n%s", setup_filename.c_str(), line.c_str(), ex.what());
		}
		catch (...)
		{
			INFO("Error in material %s can not parse line: %s", setup_filename.c_str(), line.c_str());
		}
	}

	if (!material_type_set)
	{
		//INFO("Warning! Material type is not defined in material: %s", filename);
	}

	if ( m->m_shader_names[0].empty())
	{
		//INFO("Warning! shader0 is not defined in material: %s", filename);
	}

	return true;
}


bool SceneHandler::ParseMaterialTexture( KCL::Material *m, const std::vector<std::string> &tokens)
{
	KCL::Material::TextureImageType texture_type = KCL::Material::COLOR;
	if (KCL::Material::GetTextureImageTypeByName(tokens.at(0), texture_type))
	{
		if (m->m_textures[texture_type])
		{
			//INFO("Warning! Texture channel %s is already defined in material: %s!!!", tokens.at(0).c_str(), m->m_name.c_str());
			return true;
		}

		float repeat_u = ng::atof(tokens.at(2).c_str());
		float repeat_v = ng::atof(tokens.at(3).c_str());
		int wrap_u = atoi(tokens.at(4).c_str());
		int wrap_v = atoi(tokens.at(5).c_str());

		std::string texture_file;
		CheckImage( tokens.at(1), ImagesDirectory(), texture_file);

		if( texture_file == "")
        {
		    INFO("Warning!: Can not read texture: %s", tokens.at(1).c_str());
        }
		KCL::Texture * texture = NULL;
		for (KCL::uint32 i = 0; i < m_textures.size(); i++)
		{
			if( m_textures[i] && m_textures[i]->getName() == texture_file)
			{
				texture = m_textures[i];
				break;
			}
		}

		if( !texture && loadStatic)
		{
			KCL::uint32 createFlags = 0;

			bool is_srgb = false;
            if( (texture_type == KCL::Material::COLOR || texture_type == KCL::Material::LIGHTMAP2 || texture_type == KCL::Material::MASK))
			{
				is_srgb = m_color_texture_is_srgb;
			}
            if(m->m_is_aniso)
            {
                if(texture_type == KCL::Material::COLOR)
                {
                    createFlags |= KCL::TC_AnisotropicFilter;
                }
            }

			if (m_scene_version == SV_50 || m_scene_version == SV_50_OVR)
			{
				// Turn on anisotropic filtering for every channels in Aztec Ruins
				createFlags |= KCL::TC_AnisotropicFilter;

				// Emissive channel is rendered to HDR target
				is_srgb = texture_type == KCL::Material::EMISSION;
			}

			for (unsigned int i = 6; i < tokens.size(); i++)
			{
				std::string param(tokens[i]);
				std::transform(param.begin(), param.end(), param.begin(), ::tolower);

				if (param == "srgb")
				{
					is_srgb = true;
				}
				else if (param == "rgb")
				{
					is_srgb = false;
				}
				else if (param == "nomips")
				{
					createFlags |= KCL::TC_NoMipmap;
				}
				else if (param == "aniso")
				{
					createFlags |= KCL::TC_AnisotropicFilter;
				}
				else if (param == "noaniso")
				{
					createFlags = createFlags & ~KCL::TC_AnisotropicFilter;
				}
			}

			if (is_srgb)
			{
				createFlags |= KCL::TC_IsSRGB;
			}

            if(!wrap_u || !wrap_v)
            {
                createFlags |= KCL::TC_Clamp;
            }

			if (m_scene_version == SV_40 || m_scene_version == SV_41)
			{
				// Turn on anisotropic filtering in Car Chase
				createFlags = createFlags & ~KCL::TC_AnisotropicFilter;
			}

            texture = TextureFactory().CreateAndSetup( KCL::Texture_2D, texture_file.c_str(), createFlags);
			if( !texture)
			{
				INFO("Warning!: Can not read texture: %s", texture_file.c_str());
			}
			else
			{
				m_textures.push_back( texture);
			}
		}

		if (texture)
		{
			m->m_textures[texture_type] = texture;
			m->m_image_repeat_wrapmodes[texture_type].s = wrap_u;
			m->m_image_repeat_wrapmodes[texture_type].t = wrap_v;
			m->m_image_scales[texture_type].x = repeat_u;
			m->m_image_scales[texture_type].y = repeat_v;
		}
		return true;
	}
	return false;
}


KCL::Image2D* SceneHandler::CreateLightmap( std::string &lightmap_name)
{
	if( lightmap_name == "-")
	{
		return 0;
	}

	string filename;
	filename = ImagesDirectory() + std::string(lightmap_name);

	for( uint32 i=0; i<m_lightmaps.size(); i++)
	{
		if( filename == m_lightmaps[i]->getName())
		{
			return m_lightmaps[i];
		}
	}

	KCL::Image2D *img = new Image2D;

	if(loadStatic == true)
	{
		bool b = img->load ( filename.c_str());
		if( !b)
		{
			INFO("!warning: cant read texture: %s", filename.c_str());
		}
	}

	m_lightmaps.push_back( img);

	return img;
}


KCL::Node* SceneHandler::ReadEmitter(const std::string &name, Node *parent, Actor *actor)
{
	string filename;

	KCL::ObjectType emitter_type;
	switch (m_scene_version)
	{
		case KCL::SV_27:
			emitter_type = EMITTER1;
			break;

		case KCL::SV_30:
		case KCL::SV_31:
			emitter_type = EMITTER2;
			break;

		case KCL::SV_40:
			emitter_type = EMITTER4;
			break;

		case KCL::SV_50:
			emitter_type = EMITTER5;
			break;

		case KCL::SV_41:
		default:
			INFO("WARNING! SceneHander::ReadEmitter - Scene does not support emitters!");
			return NULL;
	}

	if (emitter_type != EMITTER5)
	{
		AnimatedEmitter* emitter = 0;

		emitter = (KCL::AnimatedEmitter*)m_factory.GetFactory(emitter_type)->Create(name, parent, actor);
		assert(emitter);
		emitter->m_emitter_name = name;

		emitter->LoadParameters(this);

		emitter->Start();
		emitter->Continue();

		return emitter;
	}
	else
	{
		KCL::Node* emitter = 0;

		emitter = (KCL::Node*)m_factory.GetFactory(emitter_type)->Create(name, parent, actor);
		assert(emitter);

		AssetFile emitter_file("emitters/" + name + ".json");
		if (!emitter_file.Opened())
		{
			JsonSerializer s(true);
			emitter->Serialize(s);
		}
		else
		{
			emitter->LoadParameters("emitters/" + name + ".json");
		}

		return emitter;
	}
}


KCL::Light* SceneHandler::ReadLight( const std::string &node, const std::string &shape, Node *parent, Actor *actor)
{
	Light* l = (KCL::Light*)m_factory.GetFactory(KCL::LIGHT)->Create( node, parent, actor);

	for ( int i = 0; i < m_light_shapes.size(); ++i)
	{
		if ( m_light_shapes[i]->m_light_name == l->m_light_name)
		{
			l->m_light_shape = m_light_shapes[i];
			break;
		}
	}
	if ( l->m_light_shape == nullptr)
	{
		LightShape *new_shape = new LightShape( shape);
		m_light_shapes.push_back( new_shape);
		m_light_shapes.back()->LoadParameters();
		l->m_light_shape = m_light_shapes.back();
	}
	ReadAnimation( l->m_intensity_track, shape + "_intensity_track");
	ReadAnimation( l->m_radius_track, shape + "_radius_track");
	ReadAnimation( l->m_attenuation_track, shape + "_atten_track");
	ReadAnimation( l->m_color_track, shape + "_color_track");
	l->LoadParameters();
	return l;
}


KCL::Light* SceneHandler::ReadLight( const std::string &name, Node *parent, Actor *actor)
{
	string filename;

	AssetFile light_file( "lights/" + name);
	if( !light_file.Opened())
	{
		INFO("!!!error: no light found - %s", name.c_str());
		return 0;
	}

	Light* l = (KCL::Light*)m_factory.GetFactory(KCL::LIGHT)->Create(name, parent, actor);

	while( !light_file.eof())
	{
		char string0[512];
		char string1[512];
		Vector3D color;
		char buff[4096];
		std::stringstream ss;

		light_file.Gets(buff, 4096);
		ss << buff;

		ss >> string0;

		if( strcmp( string0, "color") == 0)
		{
#ifdef GFXB_DEPRECATED
			ss >> color.x;
			ss >> color.y;
			ss >> color.z;
			l->m_diffuse_color = color;
		}
		else if( strcmp( string0, "type") == 0)
		{
			ss >> string1;
			if( strcmp( string1, "spot") == 0)
			{
				l->m_light_type = Light::SPOT;
			}
			else if( strcmp( string1, "omni") == 0)
			{
				l->m_light_type = Light::OMNI;
			}
			else if( strcmp( string1, "parallel") == 0)
			{
				l->m_light_type = Light::DIRECTIONAL;
			}
			else
			{
			}
		}
		else if( strcmp( string0, "intensity") == 0)
		{
			ss >> string1;
			l->m_intensity = ng::atof( string1);
			if( !l->m_intensity)
			{
				l->m_intensity = (float)atoi( string1);
			}
			l->m_radius = sqrtf( l->m_intensity * 24.0f);
		}
		else if (strcmp(string0, "fov") == 0 || strcmp(string0, "cone_angle") == 0)
		{
			ss >> string1;
			l->m_spotAngle = ng::atof( string1);
		}
		else if( strcmp( string0, "cast_shadow") == 0)
		{
			ss >> string1;
			l->m_is_shadow_caster = atoi( string1);
#endif
		}
		else if( strcmp( string0, "intensity_track") == 0)
		{
			ss >> string1;
			ReadAnimation( l->m_intensity_track, string1);
		}
		else if( strcmp( string0, "glow") == 0)
		{
			ss >> string1;
			l->m_has_glow = atoi( string1);
		}
		else if( strcmp( string0, "lens_flare") == 0)
		{
			ss >> string1;
			l->m_has_lensflare = atoi( string1);
		}
		else if( strcmp( string0, "flicker") == 0)
		{
			ss >> string1;
			l->m_is_flickering = atoi( string1);
		}

	}

	if( l->m_light_name.find( "pCone") != std::string::npos)
	{
		l->m_has_lightshaft = true;
	}

/*	if( l->m_light_name.find( "tank") != std::string::npos)
	{
		l->m_has_lightshaft = true;
	}
*/

	return l;
}


void KCL::SceneHandler::ReadBezierMeshGeometry(KCL::Mesh3 * mesh3, KCL::Actor *actor, AssetFile & file)
{
	KCL::uint32 num_vertices;
	KCL::uint32 num_indices;

	file.Read(&num_vertices, 4, 1);
	file.Read(&num_indices, 4, 1);

	mesh3->m_vertex_attribs3[0].resize( num_vertices);
	mesh3->m_vertex_indices[0].resize( num_indices);

	file.Read(&mesh3->m_vertex_indices[0][0], 2, num_indices);
	file.Read(mesh3->m_vertex_attribs3[0][0].v, 4, num_vertices * 3);

	mesh3->m_vertex_indices[1] = mesh3->m_vertex_indices[0];

	mesh3->m_num_patch_vertices = 16;
}


void KCL::SceneHandler::ReadMeshGeometry(KCL::Mesh3 * mesh3, KCL::Actor *actor, AssetFile & file)
{
	KCL::uint32 skin_data_exist;
	KCL::uint32 num_vertices;
	KCL::uint32 num_indices;

	file.Read(&num_vertices, 4, 1);
	file.Read(&num_indices, 4, 1);

	mesh3->m_vertex_attribs3[0].resize( num_vertices); // Position
	mesh3->m_vertex_attribs3[1].resize( num_vertices); // Normal
	mesh3->m_vertex_attribs3[2].resize( num_vertices); // Tangent
	mesh3->m_vertex_attribs2[0].resize( num_vertices); // Texcoord0
	mesh3->m_vertex_attribs2[1].resize( num_vertices); // Texcoord1
	mesh3->m_vertex_indices[0].resize( num_indices);

	file.Read(&mesh3->m_vertex_indices[0][0], 2, num_indices);
	file.Read(mesh3->m_vertex_attribs3[0][0].v, 4, num_vertices * 3);
	file.Read(mesh3->m_vertex_attribs3[1][0].v, 4, num_vertices * 3);
	file.Read(mesh3->m_vertex_attribs3[2][0].v, 4, num_vertices * 3);
	file.Read(mesh3->m_vertex_attribs2[0][0].v, 4, num_vertices * 2);
	file.Read(&skin_data_exist, 4, 1);

	if (m_scene_version < SV_50)
	{
		KCL::AssetFile color_file(file.getFilename() + "_color");
		if (color_file.Opened())
		{
			mesh3->m_vertex_attribs3[3].resize(num_vertices);
			color_file.Read(mesh3->m_vertex_attribs3[3][0].v, sizeof(float), num_vertices * 3);
		}
	}
	

	if( skin_data_exist)
	{
		std::vector<Node*> vertex_matrix_indices;

		vertex_matrix_indices.resize( num_vertices * 4);

		mesh3->m_vertex_matrix_indices.resize( num_vertices * 4);
		mesh3->m_vertex_attribs4[0].resize( num_vertices);

		std::set<Node*> used_bones;
		std::set<std::string> missing_bones;

		for( KCL::uint32 i=0; i<num_vertices; i++)
		{
			KCL::uint32 num_weights;

			mesh3->m_vertex_attribs4[0][i].set( 0, 0, 0, 0);
			file.Read(&num_weights, 4, 1);

			for( KCL::uint32 j = 0; j < num_weights; j++)
			{
				KCL::uint32 n;
				char tmp[512] = {0};
				float weight;

				file.Read(&n, 4, 1);
				file.Read(tmp, 1, n);
				file.Read(&weight, 4, 1);

				Node *bone = Node::SearchNode(actor->m_root, std::string(tmp));
				if (bone == nullptr)
				{
					missing_bones.insert(tmp);
					bone = actor->m_root;
				}

				vertex_matrix_indices[i * 4 + j] = bone;
				used_bones.insert(vertex_matrix_indices[i * 4 + j]);

				mesh3->m_vertex_attribs4[0][i].v[j] = weight;
			}
		}

		mesh3->m_nodes.assign( used_bones.begin(), used_bones.end());
		
		if(mesh3->m_nodes.size() > KCL::Mesh3::MAX_BONES)
		{
			INFO("Error! To many bones (%d) in %s", mesh3->m_nodes.size(), mesh3->m_name.c_str());
		}

		// Report the missing bones
		for (std::set<std::string>::const_iterator it = missing_bones.begin(); it != missing_bones.end(); it++)
		{
			INFO("Error! Can not find bone: %s for actor: %s mesh: %s", it->c_str(), actor->m_name.c_str(), mesh3->m_name.c_str());
		}

		for( KCL::uint32 i=0; i<num_vertices * 4; i++)
		{
			std::vector<Node*>::iterator iter = std::find( mesh3->m_nodes.begin(),  mesh3->m_nodes.end(), vertex_matrix_indices[i]);

			if( iter != mesh3->m_nodes.end())
			{
				mesh3->m_vertex_matrix_indices[i] = iter - mesh3->m_nodes.begin();
			}
			else
			{
				mesh3->m_vertex_matrix_indices[i] = 0;
			}
		}
		
		mesh3->InitBoneData();
	}
	else
	{
		file.Read(mesh3->m_vertex_attribs2[1][0].v, 4, num_vertices * 2);
	}

	if (!file.eof())
	{
		// Color
		mesh3->m_vertex_attribs3[3].resize(num_vertices);
		file.Read(mesh3->m_vertex_attribs3[3][0].v, sizeof(float), num_vertices * 3);
	}

	if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)// in SV_40 the Maya exporter does this
	{
		mesh3->VCacheOptimize();
	}

    if ( (m_scene_version == KCL::SV_40) ||
		(m_scene_version == KCL::SV_41) ||
		(m_scene_version == KCL::SV_50) ||
        (m_scene_version == KCL::SV_ADAS) )
    {
        // Car chase and Invasion uses manually created LODs
        mesh3->m_vertex_indices[1] = mesh3->m_vertex_indices[0];
    }
    else
    {
		mesh3->GenerateLOD( 100.0f);
    }
}

// Create instance oweners. max_instance zero means no limit for instances per instance owners.
void KCL::SceneHandler::CreateMeshInstanceOwner2( KCL::Mesh *mesh, KCL::int32 max_instances)
{
	if( mesh->m_mesh->m_instanceCount < 2)
	{
		return;
	}

	MeshInstanceOwner2 *mio2 = 0;

	for( size_t i = 0; i<m_mios2.size(); i++)
	{
		if( m_mios2[i]->m_mesh == mesh->m_mesh)
		{
            if( m_mios2[i]->m_material != mesh->m_material)
			{
                continue;
			}

            if ( max_instances > 0 && m_mios2[i]->m_instances.size() >= max_instances)
            {
                continue;
            }

            mio2 = m_mios2[i];
            break;
		}
	}

	if( !mio2)
	{
		mio2 = new MeshInstanceOwner2;
		mio2->m_mesh = mesh->m_mesh;
		mio2->m_material = mesh->m_material;

		m_mios2.push_back( mio2);
	}

	mio2->m_instances.push_back( mesh);
	mesh->m_mio2 = mio2;
}


Mesh3* KCL::SceneHandler::ReadGeometry( const std::string &filename__, Node *parent, KCL::Actor *actor, bool is_bezier, bool show_error = true)
{
	Mesh3* mesh3 = 0;
	std::string filename_;

	if( is_bezier)
	{
		filename_ = "bezier_meshes/" + filename__;
	}
	else
	{
		filename_ = "meshes/" + filename__;
	}

	KCL::AssetFile file(filename_);
	if (filename_.length())
	{
		for (KCL::uint32 i = 0; i < m_meshes.size(); i++)
		{
			if (m_meshes[i]->m_name == filename_)
			{
				mesh3 = m_meshes[i];
				++m_meshes[i]->m_instanceCount;
				return mesh3;
			}
		}
	}

	if( !file.Opened())
	{
		if (show_error)
		{
			INFO("!!!error: mesh not found for %s\n", filename_.c_str());
		}
		return 0;
	}

	mesh3 = Mesh3Factory().Create( filename_.c_str());
	if( is_bezier)
	{
		ReadBezierMeshGeometry(mesh3, actor, file);
	}
	else
	{
		ReadMeshGeometry(mesh3, actor, file);
	}


	m_meshes.push_back( mesh3);

	return mesh3;
}


void SceneHandler::CreateActor( KCL::Actor *actor, std::vector<MeshToActorRef> &mesh_refs)
{
	ProgressBar::AdvanceProgress();
	for( KCL::uint32 i = 0; i < mesh_refs.size(); i++)
	{
		if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

		MeshToActorRef &ref = mesh_refs[i];

		ref.m_mesh->m_mesh_variants[0] = ReadGeometry( ref.m_geometry_name, ref.m_parent, actor, ref.m_is_bezier);
		ref.m_mesh->m_mesh_variants[1] = ReadGeometry(std::string("lod/") + ref.m_geometry_name, ref.m_parent, actor, ref.m_is_bezier, false);
		ref.m_mesh->m_mesh_variants[2] = ReadGeometry(std::string("lod2/") + ref.m_geometry_name, ref.m_parent, actor, ref.m_is_bezier, false);
		ref.m_mesh->m_mesh = ref.m_mesh->m_mesh_variants[0];
		ref.m_mesh->m_geometry_name = ref.m_geometry_name;

		if( actor->m_shader_track)
		{
			KCL::Material* m1 = CreateMaterial(ref.m_material_name.c_str());
			KCL::Material* m2 = CreateMaterial((ref.m_material_name + std::string("2")).c_str());

			ref.m_mesh->SetMaterials(m1, m2);
		}
		else
		{
			KCL::Material* m1 = CreateMaterial(ref.m_material_name.c_str());
			KCL::Material* m2 = CreateMaterial(ref.m_material_name.c_str());

			ref.m_mesh->SetMaterials(m1, m2);
		}

		if( ref.m_mesh->m_mesh->m_vertex_matrix_indices.size())
		{
			if( m_max_joint_num_per_mesh != MAX_DEFAULT_JOINT_NUM_PER_MESH)
			{
				KCL::Node *parent;
				std::vector<Mesh3*> new_meshes;

				ref.m_mesh->m_mesh->AdjustMeshToJointNum( Mesh3Factory(), new_meshes, m_max_joint_num_per_mesh);

				if( ref.m_mesh->m_parent)
				{
					parent = ref.m_mesh->m_parent;
					ref.m_mesh->m_parent->RemoveChild( ref.m_mesh);
					//TODO: elofordulhat, hogy vannak gyerekek, azokat at kellene menteni
					ref.m_mesh->m_children.clear();
				}
				else
				{
					parent = actor->m_root;
				}

				for( uint32 n = 0; n < new_meshes.size(); n++)
				{
					m_meshes.push_back( new_meshes[n]);
					KCL::Mesh* new_mesh = GetMeshFactory().Create((new_meshes[n]->m_name + "_node").c_str(), ref.m_parent, actor);

					memcpy( (void*)new_mesh, (void*)ref.m_mesh, sizeof( KCL::Mesh));
					new_mesh->m_mesh = new_meshes[n];
				}

				delete ref.m_mesh;
			}
			else
			{
				if( !ref.m_mesh->m_parent && actor->m_root)
				{
					ref.m_mesh->m_name = "added skinned mesh";
					ref.m_mesh->m_parent = actor->m_root;
					actor->m_root->m_children.push_back( ref.m_mesh);
				}
			}
		}
	}

	mesh_refs.clear();

	if( !actor->m_root)
	{
		delete actor;
	}
	else
	{
		m_actors.push_back( actor);

		actor->ComplementFire(m_factory.GetFactory(EMITTER2));
	}
}


KCL_Status SceneHandler::LoadActors()
{
	char buff[BUFF_SIZE] = { 0 };

	AssetFile actors_file("actors.txt");

	if(actors_file.Opened())
	{
		Actor *actor = 0;

		std::string actor_name;
		std::vector<MeshToActorRef> mesh_refs;

		while (!actors_file.eof())
		{
			if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

			Matrix4x4 m1;
			Matrix4x4 m2;
			bool is_actor = false;


			std::string joint_name;
			std::string parent_name;
			std::string entity_type;
			std::stringstream is;

			actors_file.Gets(buff, BUFF_SIZE);

			if (strlen(buff) < 1)
			{
				break;
			}
			if (buff[0] == '\n')
			{
				continue;
			}


			is << buff;
			is >> joint_name;
			is >> parent_name;
			is >> entity_type;

			if (strstr(joint_name.c_str(), "xActor") && parent_name == "_")
			{
				if (actor)
				{
					CreateActor(actor, mesh_refs);
				}

				actor_name = joint_name;

				actor = new Actor(actor_name.c_str(), true);

				if (m_scene_version >= SV_40) //read flags
				{
					is >> actor->m_flags;
				}

				if (strstr(actor_name.c_str(), "anubis"))
				{
					actor->m_aabb_bias.set(0.5f, 0.5f, 0.5f);
				}
				if (strstr(actor_name.c_str(), "xActor_cave_shadow"))
				{
					actor->m_aabb_bias.set(20, 4, 20);
					actor->m_aabb_scale.set(1, 1, 1);
				}
				if (strstr(actor_name.c_str(), "xActor_tracks_decals"))
				{
					actor->m_aabb_bias.set(200, 200, 200);
					actor->m_is_shadow_caster = false;
				}
				if (strstr(actor_name.c_str(), "xActor_trex_decals"))
				{
					actor->m_aabb_bias.set(200, 200, 200);
					actor->m_is_shadow_caster = false;
				}
				if (strstr(actor_name.c_str(), "steam"))
				{
					actor->m_is_shadow_caster = false;
				}
				if (strstr(actor_name.c_str(), "dust"))
				{
					actor->m_is_shadow_caster = false;
				}
				if (strstr(actor_name.c_str(), "car"))
				{
					actor->m_aabb_scale.set(2, 2, 2);
				}

				is_actor = true;


				ReadAnimation(actor->m_shader_track, actor_name + std::string("_shader_track"));

				if (strstr(actor->m_name.c_str(), "crosshair"))
				{
					m_hud_target_actor = actor;
				}
			}

			Node *parent = 0;

			if (actor->m_root)
			{
				parent = Node::SearchNode(actor->m_root, parent_name);
			}

			Node *node = 0;

			if (entity_type == "_")
			{
			}
			else if (entity_type == "mesh" || entity_type == "skinned_mesh" || entity_type == "bezier_mesh")
			{
				MeshToActorRef ref;

				is >> ref.m_geometry_name;
				is >> ref.m_material_name;
				ref.m_is_bezier = false;
				if (entity_type == "bezier_mesh")
				{
					ref.m_is_bezier = true;
				}
				ref.m_mesh = GetMeshFactory().Create(joint_name.c_str(), parent, actor);
				ref.m_parent = parent;
				ref.m_mesh->SetGuid(ref.m_geometry_name);
				mesh_refs.push_back(ref);
				node = ref.m_mesh;
			}
			else if (entity_type == "light")
			{
				std::string light_name;
				is >> light_name;
				
				if (m_scene_version == SV_50 || m_scene_version == SV_50_OVR)
				{
					node = ReadLight(joint_name, light_name, parent, actor);
				}
				else
				{
					node = ReadLight(light_name, parent, actor);
				}
			}
			else if (entity_type == "emitter")
			{
				std::string emitter_name;
				is >> emitter_name;

				node = ReadEmitter(emitter_name, parent, actor);
			}
			else
			{

			}

			if (is_actor)
			{
				continue;
			}

			if (!node)
			{
				node = new Node(joint_name, NODE, parent, actor);

				if ((parent == NULL) && (actor_name != parent_name))
				{
					INFO("!!!error: Memory Leak! parent %s for joint %s doesn't exist.",  parent_name.c_str(), node->m_name.c_str());
				}
			}

			if (parent_name == actor_name)
			{
				if (actor->m_root)
				{
					INFO("!!!error: multiple root joints of %s - %s\n", actor->m_name.c_str(), joint_name.c_str());
				}

				actor->m_root = node;
			}

			for (int i = 0; i < 16; ++i)
			{
				is >> m1.v[i];
			}

			for (int i = 0; i < 16; ++i)
			{
				is >> m2.v[i];
			}

			node->m_name = joint_name;
			node->m_local_pom = m1;
			node->m_invert_base_pose = m2;

			node->LoadPositionTrack();
			node->LoadOrientationTrack();
			node->LoadVisibilityTrack();
			node->LoadAlphaTrack();
			node->LoadSecondaryAnimCcontrolTrack();
		}

		if (actor)
		{
			CreateActor(actor, mesh_refs);
		}

	}
	return KCL::KCL_TESTERROR_NOERROR;
}


KCL_Status SceneHandler::LoadRooms()
{
	char buff[BUFF_SIZE] = { 0 };
	AssetFile rooms_file("rooms.txt");

	if (rooms_file.Opened())
	{
		while (!rooms_file.eof())
		{
			if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

			rooms_file.Gets(buff, BUFF_SIZE);
			std::stringstream ss;
			ss << buff;
			std::string name;
			std::string parent_name;
			std::string mesh_name;
			std::string material_name;

			ss >> name;
			ss >> parent_name;
			ss >> mesh_name;
			ss >> material_name;

			Mesh *mesh = GetMeshFactory().Create(name.c_str(), 0, 0);

			for (int i = 0; i < 16; ++i)
			{
				ss >> mesh->m_local_pom.v[i];
			}


			mesh->m_mesh_variants[0] = ReadGeometry(mesh_name, 0, 0, false);
			mesh->m_mesh_variants[1] = ReadGeometry(std::string("lod/") + mesh_name, 0, 0, false, false);
			mesh->m_mesh_variants[2] = ReadGeometry(std::string("lod2/") + mesh_name, 0, 0, false, false);
			mesh->m_mesh = mesh->m_mesh_variants[0];

			KCL::Material* mat1 = CreateMaterial(material_name.c_str());
			KCL::Material* mat2 = CreateMaterial(material_name.c_str());

			mesh->SetMaterials(mat1, mat2);

			if (mesh->m_materials[0]->m_name != material_name)
			{
				INFO("!!!error: material not found for %s\n", mesh_name.c_str());
			}

			if (mesh->m_mesh == NULL)
			{
				INFO("!!!error: a model is referencing a mesh file that is not found (pls check /meshes dir): %s\n", mesh_name.c_str());
			}

			mesh->m_mesh->m_uv0_scale = mesh->m_material->m_image_scales[0];

			if (mesh->m_material->m_is_occluder)
			{
				mesh->m_world_pom = mesh->m_local_pom;
				mesh->CalculateStaticAABB();
				m_occluders.push_back(mesh);
			}
			else if (strstr(parent_name.c_str(), "xRoom"))
			{
				XRoom *room = 0;
				for (KCL::uint32 i = 0; i<m_rooms.size(); i++)
				{
					if (m_rooms[i]->m_name == parent_name)
					{
						room = m_rooms[i];
						break;
					}
				}

				if (!room)
				{
					room = new XRoom(parent_name.c_str());
					m_rooms.push_back(room);
					room->m_pvs_index = static_cast<unsigned int>(m_rooms.size() - 1);
				}

				mesh->m_room = room;
				mesh->m_world_pom = mesh->m_local_pom;
				mesh->m_prev_world_pom = mesh->m_local_pom;
				mesh->CalculateStaticAABB();

				mesh->m_owner = room;

				room->m_meshes.push_back(mesh);
				room->m_aabb.Merge(mesh->m_aabb);

				m_aabb.Merge(room->m_aabb);
			}
			else if (strstr(parent_name.c_str(), "xSky"))
			{
				m_sky_mesh.push_back(mesh);
			}
			else if (strstr(parent_name.c_str(), "xIrradiance"))
			{
				mesh->m_world_pom = mesh->m_local_pom;
				mesh->m_prev_world_pom = mesh->m_local_pom;
				mesh->CalculateStaticAABB();
				m_irradiance_meshes.push_back(mesh);
			}

			switch (KCL_Status status = g_os->LoadingCallback(0))
			{
			case KCL_TESTERROR_NOERROR:
				break;
			default:
				return status;
			}
		}


		m_pvs = new bool*[m_rooms.size()];

		for (KCL::uint32 i = 0; i<m_rooms.size(); i++)
		{
			m_pvs[i] = new bool[m_rooms.size()];

			for (KCL::uint32 j = 0; j<m_rooms.size(); j++)
			{
				if (i == j)
				{
					m_pvs[i][j] = true;
				}
				else
				{
					m_pvs[i][j] = false;
				}
			}
		}
	}

	if (m_scene_version == KCL::SV_40)
	{
		for (KCL::uint32 i = 0; i<m_rooms.size(); i++)
		{
			KCL::XRoom *r = m_rooms[i];

			for (size_t j = 0; j<r->m_meshes.size(); ++j)
			{
				Mesh* m = r->m_meshes[j];

                // Car Chase instance count limit is 128
				CreateMeshInstanceOwner2(m, 128);
			}
		}

        // Remove the instance owners with only one instance
        std::vector<MeshInstanceOwner2*>::iterator it = m_mios2.begin();
        while (it != m_mios2.end())
        {
            MeshInstanceOwner2 *mio = *it;

            if (mio->m_instances.size() < 2)
            {
                // Remove the instance owner reference from the instances
                for (KCL::uint32 i = 0; i < mio->m_instances.size(); i++)
                {
                    mio->m_instances[i]->m_mio2 = NULL;
                }

                delete *it;

                it = m_mios2.erase(it);
            }
            else
            {
                it++;
            }
        }
	}
	return KCL::KCL_TESTERROR_NOERROR;
}


KCL_Status SceneHandler::LoadCubeLinks()
{
	char buff[BUFF_SIZE] = { 0 };

	AssetFile cubelinks_file("cubelinks.txt");

	if (cubelinks_file.Opened())
	{
		while (!cubelinks_file.eof())
		{
			if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

			cubelinks_file.Gets(buff, BUFF_SIZE);

			std::stringstream ss;
			std::string envmap_name;
			std::string mesh_name;
			ss << buff;
			ss >> envmap_name;
			ss >> mesh_name;

			// Find the mesh
            Node *node = SearchNodePartialMatch(mesh_name.c_str());
			Mesh *mesh = dynamic_cast<Mesh*>(node);
			if (!mesh)
			{
				//INFO( "Error in cubelinks.txt! Can not find mesh: %s", mesh_name.c_str());
				continue;
			}

			// Find the envmap
			KCL::int32 envmap_index = -1;
			for (KCL::uint32 i = 0; i < m_envmap_descriptors.size(); i++)
			{
				if (m_envmap_descriptors[i].m_name == envmap_name)
				{
					envmap_index = i;
				}
			}
			if (envmap_index == -1)
			{
				//INFO( "Error in cubelinks.txt! Can not find envmap: %s", envmap_name.c_str());
				continue;
			}
			mesh->m_envmap_id = envmap_index;
		}

	}
	return KCL::KCL_TESTERROR_NOERROR;
}


KCL_Status SceneHandler::LoadMeshFlags()
{
    char buff[BUFF_SIZE] = { 0 };

	AssetFile flags_file("mesh_flags.txt");

	if(flags_file.Opened())
	{
		while (!flags_file.eof())
		{
            if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

			flags_file.Gets(buff, BUFF_SIZE);

			std::stringstream ss;
			std::string mesh_name;
            KCL::uint32 flags;
			ss << buff;
			ss >> mesh_name;
			ss >> flags;

            // Find the mesh
			Node *node = SearchNode(mesh_name.c_str());
			Mesh *mesh = dynamic_cast<Mesh*>(node);
			if (mesh)
            {
				if (m_scene_version < SV_50)
				{
					mesh->m_flags |= flags;
				}
				else
				{
					mesh->m_flags = flags;
				}
			}
        }
    }
    return KCL::KCL_TESTERROR_NOERROR;
}


KCL_Status SceneHandler::ReadEntities(int readingType)
{
	char buff[BUFF_SIZE] = {0};

	if (KCL::File::Exists("animations/xHDR_hdr_exposure"))
	{
		bool b = ReadAnimation(m_HDR_exposure, "xHDR_hdr_exposure");
		if (!b)
		{
			INFO("Missing file: animations/xHDR_hdr_exposure");
			return KCL_TESTERROR_FILE_NOT_FOUND;
		}
	}

    if (KCL::File::Exists("animations/xCam_control_active_index_track"))
    {
        bool b = ReadAnimation(m_active_camera_index_track, "xCam_control_active_index_track");
        if (!b)
        {
            INFO("Missing file: animations/xCam_control_active_index_track");
            return KCL_TESTERROR_FILE_NOT_FOUND;
        }
    }
    else
    {
		if ( m_scene_version < SV_40)
		{
			INFO("Missing file: animations/xCam_control_active_index_track");
			return KCL_TESTERROR_FILE_NOT_FOUND;
		}
	}



	for( uint32 i = 0; i < 32; i++)
	{
		char fn[512];

		{
			sprintf( fn, "cam%u_position_track", i);
			ReadAnimation( m_camera_position_tracks[i], fn);
		}
		{
			sprintf( fn, "cam%u_orientation_track", i);
			ReadAnimation( m_camera_orientation_tracks[i], fn);
		}
		{
			sprintf( fn, "cam%uShape_fov_track", i);
			ReadAnimation( m_camera_fov_tracks[i], fn);
		}
		{
			sprintf( fn, "cam%u_focus_position_track", i);
			bool s = ReadAnimation( m_camera_focus_position_focus_tracks[i], fn);
			if (s)
			{
				INFO("Load camera focus track: %s", fn);
			}
		}
	}
	if (!m_camera_position_tracks[0])
	{
		INFO("There aren't any camera tracks! This may lead a crash...");
	}

	switch (KCL_Status status = g_os->LoadingCallback(0))
	{
	case KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	LoadActors();

	if (readingType == 1)
	{
		return KCL_TESTERROR_NOERROR;
	}

	LoadRooms();
	{
		AssetFile portals_file("portals.txt");

		if( portals_file.Opened())
		{
			while( !portals_file.eof())
			{
				if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

				Vector3D pos;
				std::stringstream ss;
				std::string name;
				std::string parent_name;

				portals_file.Gets(buff, BUFF_SIZE);

				ss << buff;
				ss >> name;
				ss >> parent_name;

				for (int i = 0; i < 3; ++i)
				{
					ss >> pos.v[i];
				}

				XPortal *portal = 0;
				for( KCL::uint32 i=0; i<m_portals.size(); i++)
				{
					if( m_portals[i]->m_name == parent_name)
					{
						portal = m_portals[i];
						break;
					}
				}

				if( !portal)
				{
					portal = new XPortal( parent_name.c_str());
					m_portals.push_back( portal);
				}

				portal->m_points.push_back( pos);

				switch (KCL_Status status = g_os->LoadingCallback(0))
				{
				case KCL_TESTERROR_NOERROR:
					break;
				default:
					return status;
				}
			}


			for( KCL::uint32 i=0; i<m_portals.size(); i++)
			{
				if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

				XPortal *p = m_portals[i];

				p->Init();
			}
		}
	}

    LoadMeshFlags();

	{
		if (m_scene_version == SV_40 || m_scene_version == SV_41)
        {
			//add the first envmap, which will only contain the sky
        	CubeEnvMapDescriptor desc;
			desc.m_name = "default_skyonly";
			desc.m_position = Vector3D(0.0f, 0.0f, 0.0f);
			m_envmap_descriptors.push_back(desc);
		}

		AssetFile envmaps_file("envmaps.txt");

		if( envmaps_file.Opened())
		{
			while( !envmaps_file.eof())
			{
				if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

				envmaps_file.Gets(buff, BUFF_SIZE);

				std::stringstream ss;
				Matrix4x4 m;
				std::string name;
				ss << buff;
				ss >> name;

				for (int i = 0; i < 16; ++i)
				{
					ss >> m.v[i];
				}

				CubeEnvMapDescriptor desc;
				desc.m_name = name;
				desc.m_position = Vector3D(m.v[12], m.v[13], m.v[14]);
				m_envmap_descriptors.push_back(desc);
			}

		}
	}

	LoadCubeLinks();

	{
		AssetFile cubemap_colors_file ("cubemap_colors.txt");

		if( cubemap_colors_file.Opened())
		{
			while( !cubemap_colors_file.eof())
			{
				if ( KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR ) break;

				cubemap_colors_file.Gets(buff, BUFF_SIZE);

				std::stringstream ss;
				std::string envmap_name;
				ss << buff;
				ss >> envmap_name;

				// Find the envmap
				KCL::int32 envmap_index = -1;
				for ( KCL::uint32 i = 0; i < m_envmap_descriptors.size(); i++)
				{
					if ( m_envmap_descriptors[i].m_name == envmap_name)
					{
						envmap_index = i;
					}
				}

				if ( envmap_index == -1)
				{
					INFO( "Error in cubemap_colors.txt! Can not find envmap: %s", envmap_name.c_str());
					continue;
				}

				// Read the colors
				CubeEnvMapDescriptor & desc = m_envmap_descriptors[envmap_index];
				for ( KCL::uint32 i = 0; i < 6; i++)
				{
					ss >> desc.m_ambient_colors[i].x;
					ss >> desc.m_ambient_colors[i].y;
					ss >> desc.m_ambient_colors[i].z;
				}
			}

		}
	}

	if( m_scene_version == SV_30 || m_scene_version == KCL::SV_31)
	{
		ConnectRooms( "xRoom1", "xRoom2b", 0);
		ConnectRooms( "xRoom1", "xRoom2", 0);
		ConnectRooms( "xRoom1", "xRoom4", 0);

		ConnectRooms( "xRoom2", "xRoom1", 0);
		ConnectRooms( "xRoom2", "xRoom2b", 0);
		ConnectRooms( "xRoom2", "xRoom3", 0);
		ConnectRooms( "xRoom2", "xRoom5", 0);

		ConnectRooms( "xRoom2b", "xRoom2", 0);

		ConnectRooms( "xRoom3", "xRoom2b", 0);
		ConnectRooms( "xRoom3", "xRoom2", 0);
		ConnectRooms( "xRoom3", "xRoom6", 0);

		ConnectRooms( "xRoom4", "xRoom1", 0);
		ConnectRooms( "xRoom4", "xRoom5", 0);
		ConnectRooms( "xRoom4", "xRoom4b", 0);

		ConnectRooms( "xRoom4b", "xRoom4", 0);

		ConnectRooms( "xRoom5", "xRoom4", 0);
		ConnectRooms( "xRoom5", "xRoom2", 0);
		ConnectRooms( "xRoom5", "xRoom6", 0);
		ConnectRooms( "xRoom5", "xRoom7", 0);

		ConnectRooms( "xRoom6", "xRoom5", 0);
		ConnectRooms( "xRoom6", "xRoom3", 0);

		ConnectRooms( "xRoom7", "xRoom5", 0);


		ConnectRoomsVisible( "xRoom1", "xRoom2b");
		ConnectRoomsVisible( "xRoom1", "xRoom2");
		ConnectRoomsVisible( "xRoom1", "xRoom3");
		ConnectRoomsVisible( "xRoom1", "xRoom4");
		ConnectRoomsVisible( "xRoom1", "xRoom5");
		ConnectRoomsVisible( "xRoom1", "xRoom6");
		ConnectRoomsVisible( "xRoom1", "xRoom7");

		ConnectRoomsVisible( "xRoom2", "xRoom1");
		ConnectRoomsVisible( "xRoom2", "xRoom2b");
		ConnectRoomsVisible( "xRoom2", "xRoom3");
		ConnectRoomsVisible( "xRoom2", "xRoom4");
		ConnectRoomsVisible( "xRoom2", "xRoom5");
		ConnectRoomsVisible( "xRoom2", "xRoom6");
		ConnectRoomsVisible( "xRoom2", "xRoom7");

		ConnectRoomsVisible( "xRoom2b", "xRoom1");
		ConnectRoomsVisible( "xRoom2b", "xRoom2b");
		ConnectRoomsVisible( "xRoom2b", "xRoom3");
		ConnectRoomsVisible( "xRoom2b", "xRoom4");
		ConnectRoomsVisible( "xRoom2b", "xRoom5");
		ConnectRoomsVisible( "xRoom2b", "xRoom6");
		ConnectRoomsVisible( "xRoom2b", "xRoom7");


		ConnectRoomsVisible( "xRoom3", "xRoom1");
		ConnectRoomsVisible( "xRoom3", "xRoom2");
		ConnectRoomsVisible( "xRoom3", "xRoom2b");
		ConnectRoomsVisible( "xRoom3", "xRoom4");
		ConnectRoomsVisible( "xRoom3", "xRoom4b");
		ConnectRoomsVisible( "xRoom3", "xRoom5");
		ConnectRoomsVisible( "xRoom3", "xRoom6");
		ConnectRoomsVisible( "xRoom3", "xRoom7");

		ConnectRoomsVisible( "xRoom4", "xRoom1");
		ConnectRoomsVisible( "xRoom4", "xRoom2");
		ConnectRoomsVisible( "xRoom4", "xRoom2b");
		ConnectRoomsVisible( "xRoom4", "xRoom3");
		ConnectRoomsVisible( "xRoom4", "xRoom4b");
		ConnectRoomsVisible( "xRoom4", "xRoom5");
		ConnectRoomsVisible( "xRoom4", "xRoom6");
		ConnectRoomsVisible( "xRoom4", "xRoom7");

		ConnectRoomsVisible( "xRoom4b", "xRoom4");
		ConnectRoomsVisible( "xRoom4b", "xRoom5");
		ConnectRoomsVisible( "xRoom4b", "xRoom6");

		ConnectRoomsVisible( "xRoom5", "xRoom1");
		ConnectRoomsVisible( "xRoom5", "xRoom2");
		ConnectRoomsVisible( "xRoom5", "xRoom2b");
		ConnectRoomsVisible( "xRoom5", "xRoom3");
		ConnectRoomsVisible( "xRoom5", "xRoom4");
		ConnectRoomsVisible( "xRoom5", "xRoom4b");
		ConnectRoomsVisible( "xRoom5", "xRoom6");
		ConnectRoomsVisible( "xRoom5", "xRoom7");

		ConnectRoomsVisible( "xRoom6", "xRoom1");
		ConnectRoomsVisible( "xRoom6", "xRoom2");
		ConnectRoomsVisible( "xRoom6", "xRoom2b");
		ConnectRoomsVisible( "xRoom6", "xRoom3");
		ConnectRoomsVisible( "xRoom6", "xRoom4");
		ConnectRoomsVisible( "xRoom6", "xRoom4b");
		ConnectRoomsVisible( "xRoom6", "xRoom5");
		ConnectRoomsVisible( "xRoom6", "xRoom7");


		ConnectRoomsVisible( "xRoom7", "xRoom1");
		ConnectRoomsVisible( "xRoom7", "xRoom2");
		ConnectRoomsVisible( "xRoom7", "xRoom2b");
		ConnectRoomsVisible( "xRoom7", "xRoom3");
		ConnectRoomsVisible( "xRoom7", "xRoom4");
		ConnectRoomsVisible( "xRoom7", "xRoom4b");
		ConnectRoomsVisible( "xRoom7", "xRoom5");
		ConnectRoomsVisible( "xRoom7", "xRoom6");
	}
	else if( m_scene_version == SV_27)
	{
		ConnectRooms( "xRoom_01", "xRoom_02", 0);
		ConnectRooms( "xRoom_02", "xRoom_03", "xPortal1");
		ConnectRooms( "xRoom_03", "xRoom_04", 0);
		ConnectRooms( "xRoom_04", "xRoom_05", 0);
		ConnectRooms( "xRoom_05", "xRoom_06", 0);

		ConnectRoomsVisible( "xRoom_01", "xRoom_02");
		ConnectRoomsVisible( "xRoom_01", "xRoom_03");

		ConnectRoomsVisible( "xRoom_02", "xRoom_01");
		ConnectRoomsVisible( "xRoom_02", "xRoom_03");
		ConnectRoomsVisible( "xRoom_02", "xRoom_04");

		ConnectRoomsVisible( "xRoom_03", "xRoom_01");
		ConnectRoomsVisible( "xRoom_03", "xRoom_02");
		ConnectRoomsVisible( "xRoom_03", "xRoom_04");
		ConnectRoomsVisible( "xRoom_03", "xRoom_05");

		ConnectRoomsVisible( "xRoom_04", "xRoom_01");
		ConnectRoomsVisible( "xRoom_04", "xRoom_02");
		ConnectRoomsVisible( "xRoom_04", "xRoom_03");
		ConnectRoomsVisible( "xRoom_04", "xRoom_05");
		ConnectRoomsVisible( "xRoom_04", "xRoom_06");

		ConnectRoomsVisible( "xRoom_05", "xRoom_02");
		ConnectRoomsVisible( "xRoom_05", "xRoom_03");
		ConnectRoomsVisible( "xRoom_05", "xRoom_04");
		ConnectRoomsVisible( "xRoom_05", "xRoom_06");

		ConnectRoomsVisible( "xRoom_06", "xRoom_05");
		ConnectRoomsVisible( "xRoom_06", "xRoom_04");
		ConnectRoomsVisible( "xRoom_06", "xRoom_03");
		//ConnectRoomsVisible( "xRoom_06", "xRoom_02");

	}
	else if( m_scene_version == SV_25)
	{
		ConnectRooms( "xRoom_Outside", "xRoom_Nagyter", "xPortal1");
		//ConnectRooms( "xRoom_Outside", "xRoom_Oldal", 0);
		//ConnectRooms( "xRoom_Outside", "xRoom_Pyramids", 0);
		ConnectRooms( "xRoom_Outside", "xRoom_Secondroom", "xPortal3");

		ConnectRooms( "xRoom_Nagyter", "xRoom_Hatso", 0);
		ConnectRooms( "xRoom_Nagyter", "xRoom_Outside", 0);
		ConnectRooms( "xRoom_Nagyter", "xRoom_Secondroom", "xPortal2");

		ConnectRooms( "xRoom_Secondroom", "xRoom_Nagyter", 0);
		ConnectRooms( "xRoom_Secondroom", "xRoom_Sand", 0);
		//ConnectRooms( "xRoom_Secondroom", "xRoom_Oldal", 0);
		ConnectRooms( "xRoom_Secondroom", "xRoom_Backroom", 0);


		ConnectRooms( "xRoom_Backroom", "xRoom_Sand", 0);
		ConnectRooms( "xRoom_Backroom", "xRoom_Sirkamra", 0);
		ConnectRooms( "xRoom_Backroom", "xRoom_Secondroom", 0);

		ConnectRooms( "xRoom_Sirkamra", "xRoom_Backroom", 0);

		ConnectRooms( "xRoom_Sand", "xRoom_Backroom", 0);
		ConnectRooms( "xRoom_Sand", "xRoom_Secondroom", 0);
		//ConnectRooms( "xRoom_Sand", "xRoom_Oldal", 0);
		ConnectRooms( "xRoom_Sand", "xRoom_Pyramids", 0);


		ConnectRoomsVisible( "xRoom_Outside", "xRoom_Nagyter");
		ConnectRoomsVisible( "xRoom_Outside", "xRoom_Oldal");
		ConnectRoomsVisible( "xRoom_Outside", "xRoom_Hatso");
		ConnectRoomsVisible( "xRoom_Outside", "xRoom_Pyramids");

		ConnectRoomsVisible( "xRoom_Nagyter", "xRoom_Hatso");
		ConnectRoomsVisible( "xRoom_Nagyter", "xRoom_Outside");
		ConnectRoomsVisible( "xRoom_Nagyter", "xRoom_Secondroom");
		ConnectRoomsVisible( "xRoom_Nagyter", "xRoom_Sand");


		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Nagyter");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Sand");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Oldal");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Outside");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Backroom");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Sirkamra");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Pyramids");
		ConnectRoomsVisible ( "xRoom_Secondroom", "xRoom_Hatso");

		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Sirkamra");
		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Secondroom");
		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Outside");
		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Oldal");
		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Pyramids");
		ConnectRoomsVisible ( "xRoom_Backroom", "xRoom_Sand");


		ConnectRoomsVisible ( "xRoom_Sirkamra", "xRoom_Secondroom");
		ConnectRoomsVisible ( "xRoom_Sirkamra", "xRoom_Outside");
		ConnectRoomsVisible ( "xRoom_Sirkamra", "xRoom_Oldal");
		ConnectRoomsVisible ( "xRoom_Sirkamra", "xRoom_Backroom");

		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Pyramids");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Oldal");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Secondroom");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Backroom");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Sirkamra");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Nagyter");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Hatso");
		ConnectRoomsVisible ( "xRoom_Sand", "xRoom_Outside");



		ConnectRooms( "xRoom_01", "xRoom_02", 0);
		ConnectRooms( "xRoom_02", "xRoom_01", 0);
		ConnectRooms( "xRoom_02", "xRoom_03", 0);
		ConnectRooms( "xRoom_03", "xRoom_02", 0);


		ConnectRoomsVisible ( "xRoom_01", "xRoom_02");
		ConnectRoomsVisible ( "xRoom_01", "xRoom_03");

		ConnectRoomsVisible ( "xRoom_02", "xRoom_01");
		ConnectRoomsVisible ( "xRoom_02", "xRoom_03");

		ConnectRoomsVisible ( "xRoom_03", "xRoom_01");
		ConnectRoomsVisible ( "xRoom_03", "xRoom_02");


		ConnectRooms( "xRoom1", "xRoom2", "xPortal1");
		ConnectRooms( "xRoom2", "xRoom3", "xPortal2");
		ConnectRooms( "xRoom3", "xRoom4", "xPortal3");
		ConnectRooms( "xRoom4", "xRoom5", "xPortal4");
		ConnectRooms( "xRoom4", "xRoom6", "xPortal5");
		ConnectRooms( "xRoom5", "xRoom6", "xPortal6");
	}

#ifdef GFXB_DEPRECATED
	if( m_torches_enabled)
	{
		std::vector<Light*> lights;

		if( m_scene_version == SV_25)
		{
			AssetFile lights_file("lights.txt");

			if( lights_file.Opened())
			{
				while( !lights_file.eof())
				{
					if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

					Light* light = (KCL::Light*)m_factory.GetFactory(KCL::LIGHT)->Create("", 0, 0);
					std::stringstream ss;

					lights_file.Gets(buff, BUFF_SIZE);

					ss << buff;

					ss >> light->m_name;

					for (int i = 0; i < 16; ++i)
					{
						ss >> light->m_local_pom.v[i];
					}

					light->m_world_pom = light->m_local_pom;

					ss >> light->m_diffuse_color.x;
					ss >> light->m_diffuse_color.y;
					ss >> light->m_diffuse_color.z;

					lights.push_back( light);
				}
			}


		}

		for(size_t i = 0; i < lights.size(); ++i)
		{
			if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

			Light* light = lights[i];

			if(!light)
			{
				continue;
			}

			if( light->m_light_type == Light::AMBIENT || light->m_light_type == Light::DIRECTIONAL || light->m_diffuse_color == Vector3D() )
			{
				continue;
			}
			float w = 0.3f;
			float h = 0.9f;

			{
				Actor *actor = new Actor( "torch", false);

				actor->m_root = new Node( "torch", NODE, 0, actor);

				actor->m_root->m_local_pom.translate( Vector3D( light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]));
				actor->m_aabb_bias.set( w / 2.0f, h / 2.0f, w / 2.0f);


				Mesh* m = GetMeshFactory().Create("flame_mesh", actor->m_root, actor);

				m->m_material = m_flameMaterial;
				m->m_mesh = Mesh3Factory().Create( "common/flame");
				m->m_mesh->ConvertToBillboard( -w / 2.0f, w / 2.0f, 0, h, 0.0f);
				m->m_mesh->GenerateLOD( 100.0f);
				m->m_color = light->m_diffuse_color;

				w *= 1.25f;
				Mesh* m2 = GetMeshFactory().Create("glow_mesh", m, actor);

				m2->m_material = m_glowMaterial;
				m2->m_mesh = Mesh3Factory().Create( "common/glow");
				m2->m_mesh->ConvertToBillboard( -w, w, -w, w, -0.4f);
				m2->m_mesh->GenerateLOD( 100.0f);
				m2->m_color = light->m_diffuse_color * 4.0f;

				m2->CreateFlickeringAnimation( 22600, 5, 0.1f);

				m_actors.push_back( actor);

				m_meshes.push_back( m->m_mesh);
				m_meshes.push_back( m2->m_mesh);
			}
		}

		for(size_t i = 0; i < lights.size(); ++i)
		{
			Light* light = lights[i];

			delete light;
		}
	}
#endif

	for(size_t i = 0; i < m_meshes.size(); ++i)
	{
		if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

		KCL::Mesh3 *m = m_meshes[i];
		for( KCL::uint32 j = 0; j < m->m_vertex_attribs2[0].size(); j++)
		{
			m->m_vertex_attribs2[0][j].x *= m->m_uv0_scale.x;
			m->m_vertex_attribs2[0][j].y *= m->m_uv0_scale.y;
		}
	}


	{
		Matrix4x4 bias;
		if (zRangeZeroToOneGlobalOpt)
		{
			bias = Matrix4x4 (0.5f, 0, 0, 0,
				0, 0.5f, 0, 0,
				0, 0, 1.0f, 0,
				0.5f, 0.5f, -1.0f, 1);
		} else {
			bias = Matrix4x4 (0.5f, 0, 0, 0,
				0, 0.5f, 0, 0,
				0, 0, 0.5f, 0,
				0.5f, 0.5f, -0.5f, 1);
		}

		KCL::Matrix4x4 scale, rot;

		rot.identity();
		rot.rotate( 90, KCL::Vector3D( 1, 0, 0));

		KCL::Matrix4x4::Scale_translate_to_fit( scale, m_aabb.GetMinVertex(), m_aabb.GetMaxVertex());

		m_world_fit_matrix = scale * bias * rot;
	}

	return KCL_TESTERROR_NOERROR;
}


void SceneHandler::Move( float delta)
{
	Vector3D direction = m_camera_position - m_camera_ref;
	direction.normalize();

	camTranslateMatrix.translate(direction * -delta);
	UpdateCamera();
}


void SceneHandler::StrafeUp(float delta)
{
	Matrix4x4 m;
	m = camPitch * camRotateMatrix * camYaw *  camTranslateMatrix;

	Vector3D up(m.v[4], m.v[5], m.v[6]);
	camTranslateMatrix.translate(up * -delta);
	UpdateCamera();
}


void SceneHandler::Strafe( float delta)
{
	Vector3D direction = m_camera_position - m_camera_ref;
	direction = Vector3D::cross(direction, Vector3D(0, 1, 0));
	direction.normalize();

	camTranslateMatrix.translate(direction * -delta);
	UpdateCamera();
}


void SceneHandler::UpdateCamera()
{
	Matrix4x4 m;
	m = camPitch * camRotateMatrix * camYaw *  camTranslateMatrix;

	m_camera_position.set(m.v[12], m.v[13], m.v[14]);
	m_camera_ref.set(-m.v[8], -m.v[9], -m.v[10]);
	m_camera_ref += m_camera_position;
}


void SceneHandler::Rotate( float delta)
{
	camYaw.rotate(delta, Vector3D(0, -1, 0));
	UpdateCamera();
}


void SceneHandler::Tilt( float delta)
{
	float aX = atan2(camPitch.v32, camPitch.v33);

	if ((Math::Deg(aX) + delta < 84) && (Math::Deg(aX) + delta > -84))
	{
		camPitch.rotate(delta, Vector3D(-1, 0, 0));
		UpdateCamera();
	}
}


void SceneHandler::Elevate( float delta)
{
	camTranslateMatrix.translate(Vector3D(0, -1, 0) * -delta);
	UpdateCamera();
}


void SceneHandler::SelectNextCamera(bool *is_freecamera)
{
	if (m_free_camera && !m_camera_position_tracks[0])
	{
		return;
	}

	m_free_camera = !m_free_camera;
	if (m_free_camera)
	{
		if (is_freecamera)
		{
			*is_freecamera = true;
		}
		INFO("freecamera");
	}
	else
	{
		if (is_freecamera)
		{
			*is_freecamera = false;
		}
		INFO("animation camera");
	}
    m_camera_position = m_animated_camera->GetEye();
    m_camera_ref = m_animated_camera->GetCenter();
	UpdateCamera();
    //m_camera_position = Vector3D(9.04, 1.86, 149.3);
    //m_camera_ref = Vector3D(8.08, 1.57, 149.38);
}


void SceneHandler::ConnectRooms( const char *r0, const char *r1, const char *portal_name)
{
	XRoom *room0 = 0;
	XRoom *room1 = 0;

	for( KCL::uint32 i=0; i<m_rooms.size(); i++)
	{
		if( m_rooms[i]->m_name == r0)
		{
			room0 = m_rooms[i];
		}
		if( m_rooms[i]->m_name == r1)
		{
			room1 = m_rooms[i];
		}
	}

	if( !room0 || !room1)
	{
		return;
	}

	for( KCL::uint32 i=0; i<m_room_connections.size(); i++)
	{
		XRoomConnection *rc = m_room_connections[i];

		if( rc->m_a == room0 && rc->m_b == room1)
		{
			return;
		}
		if( rc->m_b == room0 && rc->m_a == room1)
		{
			return;
		}
	}

	XRoomConnection *rc = new XRoomConnection( room0, room1);

	m_room_connections.push_back( rc);

	room0->m_connections.push_back( rc);
	room1->m_connections.push_back( rc);

	if( portal_name)
	{
		XPortal *portal = 0;

		for( KCL::uint32 i = 0; i < m_portals.size(); i++)
		{
			XPortal *p = m_portals[i];

			if( p && p->m_name == portal_name)
			{
				portal = p;
				break;
			}
		}
		if(portal)
		{
			rc->m_portal = portal;
			portal->ConnectRooms( room0, room1);
		}
		else
		{
			throw std::runtime_error("Missing portals.txt");
		}
	}
}


void SceneHandler::ConnectRoomsVisible( const char *r0, const char *r1)
{
	XRoom *room0 = 0;
	XRoom *room1 = 0;
	for( KCL::uint32 i=0; i<m_rooms.size(); i++)
	{
		if( m_rooms[i]->m_name == r0)
		{
			room0 = m_rooms[i];
		}
		if( m_rooms[i]->m_name == r1)
		{
			room1 = m_rooms[i];
		}
	}
	if( room0 && room1)
	{
		m_pvs[room0->m_pvs_index][room1->m_pvs_index] = true;
	}
}


KCL::XRoom* SceneHandler::SearchMyRoom( const KCL::Vector3D &p)
{
	XRoom *my_room = 0;

	for(size_t i=0; i<m_rooms.size(); ++i)
	{
		XRoom *r = m_rooms[i];

		if( r->m_aabb.Inside( p))
		{
			my_room = r;
		}
	}
	return my_room;
}


XRoom* SceneHandler::SearchMyRoomWithPlanes(const KCL::Vector3D &p)
{
	for (size_t i = 0; i < m_rooms.size(); ++i)
	{
		XRoom *r = m_rooms[i];

		if (r->m_planes.size() && XRoom::OVERLAP(r->m_planes.data(), r->m_planes.size(), p) != OVERLAP_OUTSIDE)
		{
			return r;
		}
	}
	return 0;
}


bool SceneHandler::IsPossiblyVisible( KCL::XRoom* where, KCL::AABB &aabb)
{
	if( where)
	{
		Vector3D c, he;
		std::vector<XRoom*> rooms;

		aabb.CalculateHalfExtentCenter( he, c);

		XRoom *r = SearchMyRoom( c);

		XRoom::Query( rooms, r, aabb);

		if( !rooms.size())
		{
			return true;
		}
		for( KCL::uint32 i=0; i<rooms.size(); i++)
		{
			if( m_pvs[where->m_pvs_index][rooms[i]->m_pvs_index])
			{
				return true;
			}
		}
		return false;
	}
	return true;
}


void SceneHandler::setTextureCompressionType(const std::string &texture_compression_type)
{
	if (texture_compression_type == "ETC1to565")
	{
		m_texture_compression_type = "ETC1";
		KCL::Texture::SetDecodeDetails(KCL::Image_ETC1, KCL::Image_RGB565);
	}
	else if (texture_compression_type == "ETC1to888")
	{
		m_texture_compression_type = "ETC1";
		KCL::Texture::SetDecodeDetails(KCL::Image_ETC1, KCL::Image_RGB888);
	}
	else
	{
        //no conversion at all
		KCL::Texture::SetDecodeDetails(KCL::ImageTypeAny, KCL::ImageTypeAny);
		m_texture_compression_type = texture_compression_type;
	}
	INFO("Texture type=%s", m_texture_compression_type.c_str());
}


std::string SceneHandler::GetVersionStr()
{
	if( m_scene_version == SV_25)
	{
		return "SV_25";
	}
	if( m_scene_version == SV_27)
	{
		return "SV_27";
	}
	if( m_scene_version == SV_30)
	{
		return "SV_30";
	}
	if( m_scene_version == SV_31)
	{
		return "SV_31";
	}
    if( m_scene_version == SV_40)
	{
		return "SV_40";
	}
	if( m_scene_version == SV_VDB)
	{
		return "SV_VDB";
	}

	INFO( "GetVersionStr: invalid scene!");

	return "invalid";
}


bool SceneHandler::CheckImage( const std::string &filename_, const std::string &images_dir, std::string &assetname)
{
	std::string filenames[2] =
	{
		filename_,
		filename_
	};

	if( filenames[0].length() > 3)
	{
		size_t i = filenames[0].length();

		filenames[0][i-3] = 'p';
		filenames[0][i-2] = 'v';
		filenames[0][i-1] = 'r';
	}

	for (KCL::uint32 i = 0; i < 2; i++)
	{
		AssetFile file(images_dir + filenames[i]);
		assetname = file.getFilename();
		if( assetname.length())
		{
			return true;
		}
	}

	return false;
}


bool KCL::SceneHandler::ReadAnimation(KCL::_key_node *&animation, const std::string &filename_)
{
	return ::ReadAnimation(animation, filename_);
}


void KCL::SceneHandler::IncrementProgressTo(float target)
{
	if (m_progressPtr == NULL) return;
	if (*m_progressPtr < target) *m_progressPtr = target;

	INFO("PROGRESS: %.2f", *m_progressPtr);
}


KCL::MeshFactory &KCL::SceneHandler::GetMeshFactory()
{
	return *(KCL::MeshFactory*)m_factory.GetFactory(KCL::MESH);
}


KCLFactories& KCL::SceneHandler::GetFactoryInstance()
{
	return m_factory;
}


KCL::Node* SceneHandler::SearchNodePartialMatch( const char *name) const
{
    // Search in the rooms
    Node *node = NULL;
	for ( KCL::uint32 i = 0; i < m_rooms.size() && !node; i++)
	{
		for ( KCL::uint32 j = 0; j < m_rooms[i]->m_meshes.size() && !node; j++)
		{
			node = Node::SearchNodePartialMatch( m_rooms[i]->m_meshes[j], name);
            if ( node)
            {
                return node;
            }
		}
	}
    // Search in the actors
	for ( KCL::uint32 i = 0; i < m_actors.size() && !node; i++)
	{
		node = Node::SearchNodePartialMatch( m_actors[i]->m_root, name);
        if ( node)
        {
            return node;
        }
	}
    return NULL;
}


KCL::Node* SceneHandler::SearchNode( const char *name) const
{
    // Search in the rooms
    Node *node = NULL;
	for ( KCL::uint32 i = 0; i < m_rooms.size() && !node; i++)
	{
		for ( KCL::uint32 j = 0; j < m_rooms[i]->m_meshes.size() && !node; j++)
		{
			node = Node::SearchNode( m_rooms[i]->m_meshes[j], name);
            if ( node)
            {
                return node;
            }
		}
	}
    // Search in the actors
	for ( KCL::uint32 i = 0; i < m_actors.size() && !node; i++)
	{
		node = Node::SearchNode( m_actors[i]->m_root, name);
        if ( node)
        {
            return node;
        }
	}
    return NULL;
}
