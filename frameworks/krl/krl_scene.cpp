/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "krl_scene.h"
#include "krl_mesh.h"
#include "krl_material.h"

using namespace KCL;

#define RR

void KRL_Scene::OnLoaded()
{
#ifdef RR
    if(m_scene_version == SV_25)
	{
		CreateMirror( "ferde_lejaroShape");
		CreateMirror( "polySurface3Shape");
		CreateMirror( "nagyter_waterShape");
		CreateMirror( "nagyter_floorShape");
		CreateMirror( "sirkamra_falShape1");
		CreateMirror( "sirkamra_falShape2");
		CreateMirror( "sirkamra_falShape3");
		CreateMirror( "sirkamra_floorShape");
    }
    else if(m_scene_version == SV_27)
    {
		CreateMirror( "water_planeShape");
		CreateMirror( "jWaterShape");
	}
#endif
}


KRL_Scene::KRL_Scene() : 
    m_do_instancing( true), 
    m_shader(NULL), 
    m_blur_shader(NULL),  
    m_altitude(10.0), 
    m_force_cast_shadows(false), 
    m_force_cast_reflections(false)
{
	m_azimuth = 0.0f;
	m_disabledRenderBits = 0;
	m_wireframe_render = false;

    m_featureToggle = false;

	m_global_shadowmaps[0] = 0;
	m_global_shadowmaps[1] = 0;

	for(int i = 0; i < 3; ++i)
	{
		m_pp_shaders[i] = NULL;
	}
	m_particleAdvect_shader = NULL;

	m_use_zprepass = false;
    m_force_highp = false;

	m_CullNear = 0.01f;;
	m_CullFar = 10000.0f;
	m_viewport_width = 0;
	m_viewport_height = 0;

	m_animated_camera = new Camera2;
	m_animation_time = 0;

	m_framebuffer_gamma_correction = false ;

	m_bloom = true ;

	KCL::AABB aabb;

	aabb.Set( KCL::Vector3D( -1, -1, -1), KCL::Vector3D( 1, 1, 1));
	aabb.CalculateVertices4D( m_box_vertices);

    m_adaptation_mode = 0;
    m_is_warmup = false;
}


KRL_Scene::~KRL_Scene()
{
	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		delete m_global_shadowmaps[i];
	}

	for(size_t i=0; i<m_cubemaps.size(); ++i)
	{
		delete m_cubemaps[i];
	}

	for(size_t i=0; i<m_planarmaps.size(); ++i)
	{
		delete m_planarmaps[i];
	}

	std::vector<KCL::MeshInstanceOwner*>::iterator mio = m_mios.begin();

	while( mio != m_mios.end())
	{
		delete *mio;
		mio++;
	}
}

void KRL_Scene::ResetMIOs()
{
	std::vector<KCL::MeshInstanceOwner*>::iterator mio = m_mios.begin();

	while( mio != m_mios.end())
	{
		memset(&(*mio)->m_prev_visibility_mask[0], 0, (*mio)->m_instances.size());
		mio++;
	}
}

void KRL_Scene::Animate() //overloaded!!!
{
	//std::vector<KCL::Mesh*> meshes_to_blur;
	SceneHandler::Animate();

	m_motion_blurred_meshes.clear();
	m_visible_meshes[0].clear();
	m_visible_meshes[1].clear();
	m_visible_meshes[2].clear();
	m_visible_lights.clear();
	m_lightshafts.clear();
	m_visible_instances.clear();

	Vector2D nearfar;
	m_visible_planar_maps.clear();
	//30-nal nincs pvs + nincs sky

	bool include_sky = true;

    if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31 || m_scene_version == KCL::SV_40 || m_scene_version == KCL::SV_50)
	{
		include_sky = false;
	}

	bool include_actors = true;

	/*
	if( m_scene_version == KCL::SV_50)
	{
		include_actors = false;
	}
	*/

	FrustumCull( m_active_camera, m_visible_planar_maps, m_visible_meshes, m_visible_instances, m_motion_blurred_meshes, m_pvs, nearfar, 0, include_sky, include_actors);

	std::vector<KCL::MeshInstanceOwner*>::iterator mioi = m_mios.begin();

	while( mioi != m_mios.end())
	{
		KCL::MeshInstanceOwner *mio = *mioi;

		mio->m_visible_instances.clear();

		for( uint32 i=0; i<m_visible_meshes[0].size(); i++)
		{
			KCL::Mesh *mesh = m_visible_meshes[0][i];

			if( mesh->m_user_data != mio)
			{
				continue;
			}

			mio->m_visible_instances.push_back( mesh);
			mio->m_visibility_mask[ mesh->m_offset1] = 1;
		}

		if( mio->m_visible_instances.size() )
		{
			mio->Update();
			m_visible_meshes[0].push_back( mio->m_mesh);
		}

		mioi++;
	}

	if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31) //Manhattan: so that light cones under the road with GEQUAL depth get rasterized properly
	{
		m_active_camera->SetNearFar( nearfar.x, std::max<float>(nearfar.y, 1000.0f)); 
	}
	else if ( m_scene_version == KCL::SV_50) // 5.0: Until the actor frustum culling is not correct
	{
		m_active_camera->SetNearFar( 0.1f, 10000.0f);
		m_active_camera->Update();
		//m_active_camera->SetNearFar(nearfar.x, nearfar.y);
	}
	else
	{
		m_active_camera->SetNearFar( nearfar.x, nearfar.y);
	}

	m_active_camera->Update();

	Vector3D light_dir = -m_light_dir;

	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		if( m_global_shadowmaps[i])
		{
			PrepareShadow( m_active_camera, m_global_shadowmaps[i], light_dir);
		}
	}

	for( KCL::uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
		PrepareReflection( m_active_camera, m_visible_planar_maps[i]);
	}

	if( m_scene_version == KCL::SV_27)
	{
		uint32 particle_data_index = 0;
		uint32 particle_data_offset = 0;

		for( KCL::uint32 i=0; i<m_actors.size(); i++)
		{
			Actor *actor = m_actors[i];

			for( KCL::uint32 j=0; j<actor->m_emitters.size(); j++)
			{
				KCL::uint32 particle_per_mesh = 0;
				KCL::uint32 mesh_per_emitter = 0;
				KCL::AnimatedEmitter *emitter = static_cast<AnimatedEmitter*>(actor->m_emitters[j]);

				for( uint32 i=0; i<emitter->VisibleParticleCount(); i++)
				{
					m_particle_data[particle_data_index].x = emitter->ParticleRenderAttribs()[i].m_position_x;
					m_particle_data[particle_data_index].y = emitter->ParticleRenderAttribs()[i].m_position_y;
					m_particle_data[particle_data_index].z = emitter->ParticleRenderAttribs()[i].m_position_z;
					m_particle_data[particle_data_index].w = emitter->ParticleRenderAttribs()[i].m_life_normalized;

					m_particle_color[particle_data_index].x = emitter->ParticleRenderAttribs()[i].m_color_x;
					m_particle_color[particle_data_index].y = emitter->ParticleRenderAttribs()[i].m_color_y;
					m_particle_color[particle_data_index].z = emitter->ParticleRenderAttribs()[i].m_color_z;
					m_particle_color[particle_data_index].w = emitter->ParticleRenderAttribs()[i].m_fractional_lifetime;

					particle_per_mesh++;

					if( particle_per_mesh == MAX_PARTICLE_PER_MESH)
					{
						emitter->m_meshes[mesh_per_emitter].m_primitive_count = MAX_PARTICLE_PER_MESH * 6;
						emitter->m_meshes[mesh_per_emitter].m_offset1 = particle_data_offset;
						particle_data_offset += MAX_PARTICLE_PER_MESH;
						
						m_visible_meshes[1].push_back( &emitter->m_meshes[mesh_per_emitter]);

						particle_per_mesh = 0;
						mesh_per_emitter++;
					}

					particle_data_index++;
				}
				if( particle_per_mesh)
				{
					emitter->m_meshes[mesh_per_emitter].m_primitive_count = particle_per_mesh * 6;
					emitter->m_meshes[mesh_per_emitter].m_offset1 = particle_data_offset;
					particle_data_offset += particle_per_mesh;

					m_visible_meshes[1].push_back( &emitter->m_meshes[mesh_per_emitter]);
				}
			}
		}
	}
	if (m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31 || m_scene_version == KCL::SV_40 || m_scene_version == KCL::SV_50)
	{
		for( KCL::uint32 i=0; i<m_actors.size(); i++)
		{
			Actor *actor = m_actors[i];

			for( KCL::uint32 j=0; j<actor->m_lights.size(); j++)
			{
				Light *l = actor->m_lights[j];

				if( l->m_visible)
				{
					float r = l->m_radius;

					if( l->m_light_type == Light::OMNI)
					{
						KCL::AABB aabb;
						Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

						aabb.SetWithHalfExtentCenter( KCL::Vector3D( r, r, r), light_pos);

						if( !m_active_camera->IsVisible( &aabb))
						{
							l->m_visible = false;
						}	
					}
					else if( l->m_light_type == Light::SPOT)
					{
						KCL::AABB aabb;

						{
							KCL::Matrix4x4 m2;

							l->m_light_projection.identity();
							l->m_light_projection.perspective( l->m_spotAngle, 1.0f, 0.01f, r);
							KCL::Matrix4x4::Invert4x4( l->m_light_projection, l->m_inv_light_projection);

							m2 = l->m_inv_light_projection * l->m_world_pom;

							for( KCL::uint32 i=0; i<8; i++)
							{
								mult4x4( m2, KCL::Vector3D( m_box_vertices[i].v), l->m_frustum_vertices[i]);
							}
						}

						for( uint32 i=0; i<8; i++)
						{
							aabb.Merge( l->m_frustum_vertices[i]);
						}
						if( !m_active_camera->IsVisible( &aabb))
						{
							l->m_visible = false;
						}	
					}
				}

				if( l->m_visible)
				{
					m_visible_lights.push_back( l);

					if( l->m_has_lightshaft)
					{
						m_lightshafts.push_back( l);
					}
				}
			}
		}

		if ( m_scene_version != SV_40 && m_scene_version != SV_50)
		{
			for ( uint32 j = 0; j<m_visible_meshes[0].size(); j++)
			{
				const float distance_threshold = 30.0f;
				const float distance_threshold2 = distance_threshold * distance_threshold;
				uint32 lod_idx = 0;

				KCL::Mesh* m = m_visible_meshes[0][j];

				if ( m->m_mesh_variants[1])
				{
					float d = KCL::Vector3D::distance2( m_active_camera->GetEye(), KCL::Vector3D( &m->m_world_pom.v[12]));

					if ( d > distance_threshold2)
					{
						lod_idx = 1;
					}
				}

				m->m_mesh = m->m_mesh_variants[lod_idx];
			}
		}
	}
}


void KRL_Scene::PrepareReflection( Camera2* camera, KCL::PlanarMap *pm)
{
	Vector2D nearfar;
	Vector3D eye;
	Vector3D center;
	Vector3D up;
	std::vector<KCL::PlanarMap*> visible_planar_maps;
	std::vector<KCL::Mesh*> meshes_to_blur;
	std::vector< std::vector<KCL::Mesh*> > visible_instances;
	
	eye = camera->GetEye();
	center = camera->GetCenter();
	up = camera->GetUp();

	pm->m_camera.Perspective( camera->GetFov(), m_viewport_width, m_viewport_height, 0.1f, 100.0f);
	pm->m_camera.LookAt( eye, center, up);
	pm->m_camera.Update( true, &pm->m_plane);


	pm->m_visible_meshes[0].clear();
	pm->m_visible_meshes[1].clear();


#if 1
	std::vector<Mesh*> visible_meshes[2];
	FrustumCull( &pm->m_camera, visible_planar_maps, visible_meshes, visible_instances, meshes_to_blur, m_pvs, nearfar, pm->m_room, true, true);

	for( KCL::uint32 j=0; j<2; j++)
	{
		for( KCL::uint32 k=0; k<visible_meshes[j].size(); k++)
		{
			const Mesh *s = visible_meshes[j][k];

			for( KCL::uint32 l=0; l<pm->m_receiver_meshes.size(); l++)
			{
				if( visible_meshes[j][k] == pm->m_receiver_meshes[l])
				{
					s = 0;
					break;
				}
			}

			if( s)
			{
				pm->m_visible_meshes[j].push_back( visible_meshes[j][k]);
			}
		}
	}

	std::vector<KCL::MeshInstanceOwner*>::iterator mioi = m_mios.begin();

	while( mioi != m_mios.end())
	{
		KCL::MeshInstanceOwner *mio = *mioi;

		if( mio->m_visible_instances.size() )
		{
			pm->m_visible_meshes[0].push_back( mio->m_mesh);
		}

		mioi++;
	}

#else
	FrustumCull( &pm->m_camera, pm->m_visible_meshes, 0, nearfar);

	pm->m_camera.SetNearFar( nearfar.x, nearfar.y);
	pm->m_camera.Update( true, &pm->m_plane);

#endif
}


void KRL_Scene::Get2Closest(const Vector3D &pos, KRL_CubeEnvMap *&firstClosest, KRL_CubeEnvMap *&secondClosest, float &lambda)
{
	float close1st = FLT_MAX;
	float close2nd = FLT_MAX;

	assert(m_cubemaps.size() > 0);

	if(m_cubemaps.size() == 1)
	{
		firstClosest = m_cubemaps[0];
		secondClosest = m_cubemaps[0];
		lambda = 1.0f;
		return;
	}

	secondClosest = 0;
	firstClosest = 0;

	for(size_t i=0; i<m_cubemaps.size(); ++i)
	{
		float tmp = Vector3D::distance(m_cubemaps[i]->GetPosition(), pos);
		if( tmp < close1st)
		{
			close2nd = close1st;
			close1st = tmp;
			secondClosest = firstClosest;
			firstClosest = m_cubemaps[i];
		}
		else if( tmp < close2nd)
		{
			close2nd = tmp;
			secondClosest = m_cubemaps[i];
		}
	}
	lambda = close2nd / (close1st + close2nd);
}


void resizeCamera(Camera2* cam, KCL::uint32 w, KCL::uint32 h)
{
	cam->Perspective(cam->GetFov(), w, h, cam->GetNear(), cam->GetFar());
	cam->Update();
}


void KRL_Scene::Resize(KCL::uint32 w, KCL::uint32 h)
{
	m_viewport_width = w;
	m_viewport_height = h;

	if(m_fps_camera)
	{
		resizeCamera(m_fps_camera, w, h);
	}

	if(m_animated_camera)
	{
		resizeCamera(m_animated_camera, w, h);
	}
}


void KRL_Scene::FrustumCull( Camera2* camera, std::vector<KCL::PlanarMap*> &visible_planar_maps, std::vector<Mesh*> visible_meshes[3], std::vector< std::vector<KCL::Mesh*> > &visible_instances, std::vector<Mesh*> &meshes_to_blur, bool **pvs, Vector2D &nearfar, XRoom *r, bool include_sky, bool include_actors, std::vector<Actor*>* exlude_list, CullingAlgorithm *culling_algorithm)
{
	XRoom *my_room = r ? r : SearchMyRoom( camera->GetEye());

	nearfar.set( FLT_MAX, -FLT_MAX);

	if( my_room)
	{
		std::vector<XRoom*> visible_rooms;
		Vector4D p[] = 
		{
			-camera->GetCullPlane(0),
			-camera->GetCullPlane(1),
			-camera->GetCullPlane(2),
			-camera->GetCullPlane(3),
			-camera->GetCullPlane(4),
			-camera->GetCullPlane(5),
		};

		p[5].w = - Vector3D::dot( Vector3D( p[5].x, p[5].y, p[5].z), camera->GetEye());

        XRoom::FrustumCull( visible_rooms, visible_planar_maps, visible_meshes, visible_instances, my_room, camera, p, 4, my_room, pvs, culling_algorithm);

		for( uint32 i=0; i<visible_rooms.size(); i++)
		{
			XRoom *r = visible_rooms[i];

			Vector2D mm = r->m_aabb.DistanceFromPlane( camera->GetCullPlane( KCL::CULLPLANE_NEAR));

			if( mm.x < nearfar.x)
			{
				nearfar.x = mm.x;
			}
			if( mm.y > nearfar.y)
			{
				nearfar.y = mm.y;
			}
		}
	}
	else
	{
        std::vector<KCL::MeshInstanceOwner2*> visible_mios;
        XRoom::m_counter++;
		for(size_t i=0; i<m_rooms.size(); ++i)
		{
			XRoom *r = m_rooms[i];

			if( camera->IsVisible( &r->m_aabb))
			{
				for(size_t j=0; j<r->m_meshes.size(); ++j)
				{
					Mesh* m = r->m_meshes[j];

                    //NOTE: this can NOT be used for shadows
                    if( m->m_material->m_is_shadow_only)
		            {
			            continue;
		            }

                    if( m->m_frame_when_rendered == XRoom::m_counter)
		            {
			            continue;
		            }

		            m->m_frame_when_rendered = XRoom::m_counter;

					if( camera->IsVisible( &m->m_aabb))
					{
                        if( m->m_mio2)
			            {
				            if( m->m_mio2->m_frame_when_rendered != XRoom::m_counter)
				            {
					            m->m_mio2->m_frame_when_rendered = XRoom::m_counter;

					            m->m_mio2->m_visible_instances.clear();

                                visible_mios.push_back( m->m_mio2);
				            }

				            m->m_mio2->m_visible_instances.push_back( m);
			            }
                        else
                        {
						    visible_meshes[m->m_material->m_is_decal ? 2 : m->m_material->m_is_transparent].push_back( m);
                        }
					}
				}
			}
			Vector2D mm = r->m_aabb.DistanceFromPlane( camera->GetCullPlane( KCL::CULLPLANE_NEAR));

			if( mm.x < nearfar.x)
			{
				nearfar.x = mm.x;
			}
			if( mm.y > nearfar.y)
			{
				nearfar.y = mm.y;
			}
		}



        visible_instances.resize( visible_mios.size());

        int front_counter = 0;
        int back_counter = static_cast<int>(visible_mios.size() - 1);
	    for( size_t i=0; i<visible_mios.size(); i++)
	    {
            if (visible_mios[i]->m_visible_instances[0]->m_material->m_is_tesselated)
            {
                visible_instances[front_counter++] = visible_mios[i]->m_visible_instances;
            }
            else
            {
		        visible_instances[back_counter--] = visible_mios[i]->m_visible_instances;
            }
	    }
	}

	nearfar.x = nearfar.x < 0.0f || nearfar.x == FLT_MAX ? m_camera_near : nearfar.x;
	nearfar.y = nearfar.y < 0.0f || nearfar.y == -FLT_MAX ? 1000.0f : nearfar.y + 5;

    bool reflection_culling = culling_algorithm && (culling_algorithm->m_type == CullingAlgorithm::CA_REFL) && !m_force_cast_reflections;

	if( include_actors)
	{
        m_visible_actors.clear();

		for( uint32 i=0; i<m_actors.size(); i++)
		{
			Actor *actor = m_actors[i];

            if(reflection_culling && !(actor->m_flags & KCL::Mesh::OF_REFLECTION_CASTER))
            {
                continue;
            }

            bool found = false;
            if(exlude_list)
            {
                for(size_t i=0; i<exlude_list->size(); ++i)
                {
                    if(actor == (*exlude_list)[i])
                    {
                        found = true;
                        break;
                    }
                }
                if(found)
                {
                    continue;
                }
            }
		
			if( !camera->IsVisible( &actor->m_aabb))
			{
				continue;
			}

			if( !IsPossiblyVisible( my_room, actor->m_aabb))
			{
				continue;
			}

            m_visible_actors.push_back(actor);

			for( uint32 i=0; i<actor->m_meshes.size(); i++)
			{
				Mesh* m = actor->m_meshes[i];

				if( m->m_visible)
				{
					visible_meshes[m->m_material->m_is_decal ? 2 : m->m_material->m_is_transparent].push_back( m);

					if( m->m_is_motion_blurred)
					{
						meshes_to_blur.push_back( m);
					}
				}
			}
		}
	}

	if( include_sky)
	{
		for( uint32 i=0; i<m_sky_mesh.size(); i++)
		{
			visible_meshes[0].push_back( m_sky_mesh[i]);
		}
	}
}


void KRL_Scene::PrepareShadow( Camera2* camera, KRL_ShadowMap *sm, KCL::Vector3D &light_dir)
{
#if defined HAVE_DX || defined USE_METAL
    static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
		0, -0.5f, 0, 0,
		0, 0, 1.0f, 0,
		0.5f, 0.5f, 0.0f, 1);
#elif defined USE_ANY_GL
	static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
		0, 0.5f, 0, 0,
		0, 0, 0.5f, 0,
		0.5f, 0.5f, 0.5f, 1);
#endif


	m_shadow_focus_aabb.Reset();

	for( uint32 j=0; j<2; j++)
	{
		sm->m_caster_meshes[j].clear();
		sm->m_receiver_meshes[j].clear();
	}

	XRoom *my_room = SearchMyRoom( m_active_camera->GetEye());

	std::vector<Actor*> actors;
	
	if( m_scene_version == KCL::SV_25)
	{
		actors = m_actors;
	}
	else if( m_scene_version == KCL::SV_27)
	{
		if( sm == m_global_shadowmaps[0])
		{
			for(size_t i=0; i<m_actors.size(); ++i)
			{
				Actor* actor = m_actors[i];

				if( !actor->m_root->m_visible)
				{
					continue;
				}

				if( actor->m_name.find( "girl") != std::string::npos)
				{
					actors.push_back( actor);
				}
				if( m_animation_time > 50000)
				{
					if( actor->m_name.find( "cave") != std::string::npos)
					{
						actors.push_back( actor);
					}
				}
			}
		}
		else
		{
			for(size_t i=0; i<m_actors.size(); ++i)
			{
				Actor* actor = m_actors[i];
				if( actor->m_name == "xActor_trex")
				{
					actors.push_back( actor);
				}
			}
		}
	}
	else if( m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)
	{
		for(size_t i=0; i<m_actors.size(); ++i)
		{
			Actor* actor = m_actors[i];

			if( !actor->m_root->m_visible)
			{
				continue;
			}

			if( actor->m_name.find( "robot") != std::string::npos)
			{
				actors.push_back( actor);
			}
		}
	}
	else if( m_scene_version == KCL::SV_40)
	{
		if( sm == m_global_shadowmaps[0])
		{
			actors.push_back( m_actors[0]);
		}
		else
		{
			actors.push_back( m_actors[1]);
		}
	}
	for(size_t i=0; i<actors.size(); ++i)
	{
		AABB actor_shadow_volume;
		Actor* actor = actors[i];

		if( !actor->m_is_shadow_caster)
		{
			continue;
		}

		AABB aabb2( actor->m_aabb.GetMinVertex() + light_dir * 2.0f, actor->m_aabb.GetMaxVertex() + light_dir * 2.0f);
		
		actor_shadow_volume.Merge( actor->m_aabb);
		actor_shadow_volume.Merge( aabb2);

		if( !IsPossiblyVisible( my_room, actor_shadow_volume))
		{
			continue;
		}

		if( m_active_camera->IsVisible( &actor_shadow_volume))
		{
			sm->m_caster_meshes[0].insert( sm->m_caster_meshes[0].end(), actor->m_meshes.begin(), actor->m_meshes.end());

			m_shadow_focus_aabb.Merge( actor_shadow_volume);
		}
	}

	if(m_shadow_focus_aabb.IsEmpty())
		return;

	Vector3D he, c;
	m_shadow_focus_aabb.CalculateHalfExtentCenter( he, c);


	if( m_scene_version < KCL::SV_30)
	{
		std::vector<Mesh*> receiver_meshes;

		XRoom *r = SearchMyRoom( c);
		XRoom::Query( receiver_meshes, r, m_shadow_focus_aabb);

		for( uint32 i=0; i<receiver_meshes.size(); i++)
		{
			if( m_scene_version == KCL::SV_27 && receiver_meshes[i]->m_material->m_textures[3])
			{
				continue;
			}
			sm->m_receiver_meshes[0].push_back( receiver_meshes[i]);
		}
	}

	Vector3D view_dir = m_active_camera->GetCenter() - m_active_camera->GetEye();
	view_dir.normalize();

	sm->m_camera.OrthoFocusLispsm( &m_shadow_focus_aabb, m_active_camera->GetEye(), view_dir, light_dir, m_active_camera->GetNear());
	sm->m_camera.Update();
	sm->m_matrix = sm->m_camera.GetViewProjection() * shadowM;
} 


void KRL_Scene::CreateMirror( const char* name)
{
	KCL::PlanarMap * pm = 0;
	std::vector<Mesh*> out;
	Vector3D t;
	Vector3D A;
	Vector3D B;
	Vector3D C;
	Vector3D e1;
	Vector3D e2;
	Vector3D n;
	Vector3D p;
	XRoom *room = 0;

	for( uint32 i=0; i<m_rooms.size(); i++)
	{
		for( uint32 j=0; j<m_rooms[i]->m_meshes.size(); j++)
		{
			if( strstr( m_rooms[i]->m_meshes[j]->m_mesh->m_name.c_str(), name))
			{
				room = m_rooms[i];
				out.push_back( m_rooms[i]->m_meshes[j]);
			}
		}
	}

	if( out.size())
	{
		pm = KCL::PlanarMap::Create( m_viewport_width / 2, m_viewport_height / 2, name);

		pm->m_room = out[0]->m_room;

		for( uint32 i=0; i<out.size(); i++)
		{
			pm->m_receiver_meshes.push_back( out[i]);
			pm->m_aabb.Merge( out[i]->m_aabb);
		}


		t = out[0]->m_mesh->m_vertex_attribs3[0][out[0]->m_mesh->m_vertex_indices[0][0]];
		mult4x3( out[0]->m_world_pom, t, A);

		t = out[0]->m_mesh->m_vertex_attribs3[0][out[0]->m_mesh->m_vertex_indices[0][1]];
		mult4x3( out[0]->m_world_pom, t, B);

		t = out[0]->m_mesh->m_vertex_attribs3[0][out[0]->m_mesh->m_vertex_indices[0][2]];
		mult4x3( out[0]->m_world_pom, t, C);


		e1 = B - A;
		e2 = C - A;

		n = Vector3D::cross( e1, e2);

		n.normalize();

		p.set( A.v);
		pm->m_plane.set( n.x, n.y, n.z, -Vector3D::dot( p, n));

		if(room)
		{
			room->m_planar_maps.push_back( pm);
		}
		m_planarmaps.push_back( pm);
	}
}

void KRL_Scene::CollectInstances( std::vector<KCL::Mesh*> &visible_meshes)
{
	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		Mesh* sm = (Mesh*)visible_meshes[j];
		KRL::Mesh3 *km = (KRL::Mesh3 *)sm->m_mesh;

		km->m_instances.clear();
		km->m_is_rendered.clear();
	}

	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		KCL::Mesh* m = visible_meshes[j];
		KRL::Mesh3 *km = (KRL::Mesh3*)m->m_mesh;
		KRL::Mesh3::InstanceData id;

		id.mv = m->m_world_pom;
		KCL::Matrix4x4::InvertModelView( id.mv, id.inv_mv);

		km->m_instances[m->m_material].push_back( id);
	}
}


void KRL_Scene::SetWireframeRenderEnabled(bool value)
{
    this->m_wireframe_render = value;
}

#include <algorithm>


KCL::Vector3D origin;
KCL::Vector3D plane_normal;

bool sorter( const KCL::Vector3D& lhs, const KCL::Vector3D& rhs)
{
	KCL::Vector3D v = KCL::Vector3D::cross( (lhs - origin), (rhs - origin));
	return KCL::Vector3D::dot( v, plane_normal) < 0;
}


void LightShaft::CreateCone( const KCL::Matrix4x4& transform, Vector4D plane, bool isCameraShaft)
{
#define SLICE_CONE_EDGES		36
#define MAX_SLICES				48
	static Vector3D coneVerts[SLICE_CONE_EDGES + 1];
	static Vector3D points[SLICE_CONE_EDGES + 1];
	static float d[SLICE_CONE_EDGES + 1];
	static uint16 edgeList[4 * SLICE_CONE_EDGES];

	// build edge list
	for(int i = 0; i < SLICE_CONE_EDGES; i++)
	{
		edgeList[4 * i + 0] = 0;
		edgeList[4 * i + 1] = i + 1;
		edgeList[4 * i + 2] = i + 1;
		edgeList[4 * i + 3] = i + 2;
	}

	// cycle the first vertex
	edgeList[4 * SLICE_CONE_EDGES - 1] = 1;

	// Adjust 'tessellated' cone radius so what it wholly contains original cone, i.e. make sure we generated piramid based on outer circle radius, rather than inner circle radius
	float adjustRadius = 1/cos(3.141592654f / SLICE_CONE_EDGES);

	// generate verts
	coneVerts[0] = Vector3D(0, 0, -1);
	for(int i = 0; i < SLICE_CONE_EDGES; i++)
	{
		float angle = 2.0f * 3.141592654f * i / SLICE_CONE_EDGES;
		float sa = sinf(angle)*adjustRadius;
		float ca = cosf(angle)*adjustRadius;
		coneVerts[i + 1].x = sa;
		coneVerts[i + 1].y = ca;
		coneVerts[i + 1].z = 1.0f;
		//coneVerts[i + 1].z = 0.999f; // this makes cones more clearly identifiable - debug feature
		//coneVerts[i + 1].w = 1.0f;
	}

	// transform to the light space
	for(int i = 0; i < SLICE_CONE_EDGES + 1; i++)
	{
		mult4x4( transform, KCL::Vector3D(coneVerts[i].v), points[i]);
	}

	uint32 num_new_indices = 0;
	m_index_offsets.push_back(static_cast<KCL::uint32>(m_indices.size()));

	uint16 base_idx = static_cast<KCL::uint16>(m_vertices.size());

	Vector3D v;
	for (int i=0; i <= SLICE_CONE_EDGES; i++)
		m_vertices.push_back(points[i]);

	// sides of cone
	for (int i=1; i < SLICE_CONE_EDGES; i++)
	{
		m_indices.push_back( base_idx );
		m_indices.push_back( base_idx + i+1 );
		m_indices.push_back( base_idx + i );
		num_new_indices += 3;
	}
		m_indices.push_back( base_idx );
		m_indices.push_back( base_idx + 1 );
		m_indices.push_back( base_idx + SLICE_CONE_EDGES );
		num_new_indices += 3;

	// bottom of cone
	for (int i=2; i < SLICE_CONE_EDGES; i++)
	{
		m_indices.push_back( base_idx + 1 );
		m_indices.push_back( base_idx + i );
		m_indices.push_back( base_idx + i+1 );
		num_new_indices += 3;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Quick hack - replace code later! surface clipped by front plane with CPU generated geometry
	// [this is prone to visualization bugs as HW clipper and CPU generated geometry is bound to be different]

	// Find out if we intersect front plane (i.e. camera is inside light volume)
	// compute distances
	float min_z = FLT_MAX;
	float max_z = -FLT_MAX;
	for(int i = 0; i < SLICE_CONE_EDGES + 1; i++)
	{
		Vector4D v(points[i].x, points[i].y, points[i].z, 1.0f);
		//points[i].w = 0.0;
		d[i] = KCL::Vector4D::dot( plane, v);
		if (d[i] < min_z)
			min_z = d[i];
		if (d[i] > max_z)
			max_z = d[i];
	}

    if (min_z > 0)
	{
		m_num_indices.push_back( num_new_indices);
		return; // Exit if no intersection
	}

	// compute slice distances
	//float step_dist = 1.0f;
	//float count = 0; //(max_z - min_z) / step_dist;

	//count = 1; // we always do only single slice // (max_z - min_z);

	//step_dist = 1.0f;

	// slice
	float iter = 0;
	float dist = 0.001f; // set distance (from near clip plane) where we want to clip out geometry - should really be (ZERO+epsilon)
	{
		iter += 1.0f;
		//dist+= step_dist;

		base_idx = static_cast<KCL::uint16>(m_vertices.size());
		uint16 num_new_vertices = 0;

		for( uint32 i=0; i<2*SLICE_CONE_EDGES; i++)
		{
			uint32 i0 = edgeList[i*2 + 0];
			uint32 i1 = edgeList[i*2 + 1];
			float d0 = d[i0];
			float d1 = d[i1];

			if( (d0 > dist) != (d1 > dist))
			{
				d0 -= dist;
				d1 -= dist;
				float ii = (-d1) / (d0 - d1);

				//vec4f &v = vertices->Push_back();
				//LRP3( &v, ii, &points[i1], &points[i0]);

				Vector3D v;
				v = KCL::Vector3D::interpolate( KCL::Vector3D( points[i1].v), KCL::Vector3D( points[i0].v), ii);
				//v.col = colors[iter%4];
				//v = scrollStrength;

				m_vertices.push_back( v);

				if( num_new_vertices >= 2)
				{
					m_indices.push_back( base_idx);
                    m_indices.push_back(static_cast<KCL::uint16>(m_vertices.size() - 2));
                    m_indices.push_back(static_cast<KCL::uint16>(m_vertices.size() - 1u));
					num_new_indices += 3;
				}

				num_new_vertices++;
			}
		}
		if( num_new_vertices)
		{
			plane_normal.set( plane.v);
			origin = m_vertices[base_idx];

            //NOTE: since this would just reorder vertices, but leave indices untouched, would produce buggy geometry in some cases - not needed here
			//std::sort( &m_vertices[base_idx], &m_vertices[base_idx] + num_new_vertices, sorter);
		}
	}

	m_num_indices.push_back( num_new_indices);
}


void LightShaft::CreateSlices( Vector4D plane, bool isCameraShaft)
{
	float min_z = FLT_MAX;
	float max_z = -FLT_MAX;
	float d[8];
	uint32 edgelist[] = 
	{
		0, 1,
		1, 3,
		2, 3,
		0, 2,

		4, 5,
		5, 7,
		7, 6,
		4, 6,

		0, 4, 
		1, 5,
		2, 6,
		3, 7
	};

	for( uint32 i=0; i<8; i++)
	{
		m_corners[i].w = 0.0;

		d[i] = KCL::Vector4D::dot( plane, m_corners[i]);

		if( d[i] < min_z)
		{
			min_z = d[i];
		}
		if( d[i] > max_z)
		{
			max_z = d[i];
		}
	}

	//min_z = std::min( min_z, -plane.w);
	//max_z = std::min( max_z, -plane.w);

	//KCL::Vector3D colors[4] = 
	//{
	//	KCL::Vector3D(1,0,0),
	//	KCL::Vector3D(0,1,0),
	//	KCL::Vector3D(0,0,1),
	//	KCL::Vector3D(1,1,0)
	//};

	uint32 num_new_indices = 0;
	m_index_offsets.push_back(static_cast<KCL::uint32>(m_indices.size()));

	float count = 0; //(max_z - min_z) / step_dist;
	
    for (float dist = min_z; dist < max_z; )
	{
		if(isCameraShaft)
		{
			dist+= powf(count+0.5f,1.2f);
		}
		else
		{
			dist+= 1.0f;
		}

		count += 1.0f;
	}

	float iter = 0;
	for( float dist=min_z; dist<max_z; )
	{
		iter += 1.0f;

		if(isCameraShaft)
		{
			dist+= powf(iter+0.5f,1.2f);
		}
		else
		{
			dist+= 1.0f;
		}

		uint16 base_idx = static_cast<uint16>(m_vertices.size());
		uint16 num_new_vertices = 0;

		for( uint32 i=0; i<12; i++)
		{
			uint32 i0 = edgelist[i*2 + 0];
			uint32 i1 = edgelist[i*2 + 1];
			float d0 = d[i0];
			float d1 = d[i1];

			if( (d0 > dist) != (d1 > dist))
			{
				d0 -= dist;
				d1 -= dist;
				float ii = (-d1) / (d0 - d1);

				//vec4f &v = m_vertices->Push_back();
				//LRP3( &v, ii, &m_corners[i1], &m_corners[i0]);

				KCL::Vector3D v;
				v = KCL::Vector3D::interpolate( KCL::Vector3D( m_corners[i1].v), KCL::Vector3D( m_corners[i0].v), ii);
				//v.col = colors[iter%4];
				//v.scrollStrength = scrollStrength;

				m_vertices.push_back( v);

				if( num_new_vertices >= 2)
				{
					m_indices.push_back( base_idx);
                    m_indices.push_back(static_cast<KCL::uint16>(m_vertices.size() - 1));
                    m_indices.push_back(static_cast<KCL::uint16>(m_vertices.size() - 2));
					num_new_indices+=3;
				}

				num_new_vertices++;
			}
		}

		if( num_new_vertices)
		{
			plane_normal.set( plane.v);
			origin = m_vertices[base_idx];

			std::sort( &m_vertices[base_idx], &m_vertices.front() + m_vertices.size(), sorter);
		}
	}

	m_num_indices.push_back( num_new_indices);
	//std::reverse(m_indices.begin(), m_indices.end());
}
