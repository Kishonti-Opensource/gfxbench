/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_particlesystem.h>
#include <kcl_actor.h>
#include "kcl_scene_handler.h"
#include "ng/stringutil.h"
#include <sstream>

using namespace KCL;

Emitter::Emitter( const std::string &name, ObjectType type, Node *parent, Object *owner) : Node( name, type, parent, owner)
{
	static int randomSeed = 0;
	m_random_seed = randomSeed++;

	m_visibleparticle_count = 0;
	m_prev_time = -1.0f;
	m_prev_time_div_1000 = m_prev_time / 1000.0f;
	m_diff_time = 0.0f;
	m_diff_time_div_1000 = 0.0f;
	m_emit_count_accumulator = 0.0f;
	m_emitter_type = 0;
	m_is_local_emitting = false;
	m_is_focusing = false;
	m_is_active = false;
}


Emitter::~Emitter()
{
	for(size_t i=0; i<m_particles.size(); ++i)
	{
		delete m_particles[i];
		m_particles[i] = 0;
	}
}


void Emitter::Init(KCL::uint32 max_particle_count)
{
	m_max_particle_count = max_particle_count;

	if(max_particle_count)
	{
		m_particles.reserve( max_particle_count);
		m_particle_render_attribs.reserve( max_particle_count);
	}

	for( KCL::uint32 i=0; i<max_particle_count; i++)
	{
		Particle *p = new Particle;
		m_particles.push_back( p);
		
		ParticleRenderAttrib dummy;
		m_particle_render_attribs.push_back(dummy);
	}
}


KCL::uint32 Emitter::Emit( KCL::uint32 time)
{
	if( !IsActive() )
	{
		return 0;
	}

	m_diff_time = m_prev_time < 0 || time-m_prev_time < 0 ? 0 : time-m_prev_time;
	m_diff_time_div_1000 = m_diff_time / 1000.0f;

	m_emit_count_accumulator += m_diff_time_div_1000 * get_spawning_rate() / 200.0;
	KCL::uint32 emitCount = m_emit_count_accumulator;
	m_emit_count_accumulator -= emitCount;

	m_prev_time = time;
	m_prev_time_div_1000 = m_prev_time / 1000.0f;

	//INFO("Emitcount %d\n", emitCount);

	return emitCount;
}


void Emitter::ResurrectParticle( Particle *particle)
{
	particle->Reset();

	particle->m_birth_time = m_prev_time;
	particle->m_random_offset = KCL::Math::randomf_from_range(&m_random_seed, 0.0f, 1.0f);
	particle->m_current_lifetime = 0;
	particle->m_max_lifetime = KCL::Math::randomf_from_range(&m_random_seed, get_min_initial_lifetime(), get_max_initial_lifetime());
	
	InitialParticleSpeedAndPosition( particle->m_speed, particle->m_world_position);

	//TODO more : additional acceleration, scale, fade, rotation_angle
}


void Emitter::InitialParticleSpeedAndPosition( KCL::Vector3D &speed, KCL::Vector3D &position)
{
	{
		KCL::Vector3D _speed;

		{
			float Length = KCL::Math::randomf_from_range(&m_random_seed, get_min_initial_velocity(), get_max_initial_velocity());
			float rad = KCL::Math::Rad ( get_max_angle() );
			//float F = KCL::Math::randomf_from_range(&m_random_seed, -rad, rad);
		
			_speed.x = -sin (rad) * Length;
			_speed.y =  cos (rad) * Length; 
		}

		{
			float deg = KCL::Math::randomf_from_range(&m_random_seed, 0, 360);
			float rad = KCL::Math::Rad (deg);

			_speed.z = -sin (rad) * _speed.x;
			_speed.x =  cos (rad) * _speed.x;
		}

		if(m_is_local_emitting)
		{
			speed.x = _speed.x;
			speed.y = _speed.y;
			speed.z = _speed.z;
		}
		else
		{
			speed.x = m_world_pom.v11 * _speed.x + m_world_pom.v21 * _speed.y + m_world_pom.v31 * _speed.z;
			speed.y = m_world_pom.v12 * _speed.x + m_world_pom.v22 * _speed.y + m_world_pom.v32 * _speed.z;
			speed.z = m_world_pom.v13 * _speed.x + m_world_pom.v23 * _speed.y + m_world_pom.v33 * _speed.z;
		}
	}

	float RX = KCL::Math::randomf_signed(&m_random_seed) * get_aperture_x() / 2;
	float RZ = KCL::Math::randomf_signed(&m_random_seed) * get_aperture_z() / 2;

	if(IsFocusing())
	{
		if(speed.x * RX > 0)
		{
			speed.x *= -1;
		}
		
		if(speed.z * RZ > 0)
		{
			speed.z *= -1;
		}
	}

	if(m_is_local_emitting)
	{
		position.x = RX;
		position.y = 0.0f;
		position.z = RZ;
	}
	else
	{
		position.x = m_world_pom.v41 + m_world_pom.v11 * RX + m_world_pom.v31 * RZ;
		position.y = m_world_pom.v42 + m_world_pom.v12 * RX + m_world_pom.v32 * RZ;
		position.z = m_world_pom.v43 + m_world_pom.v13 * RX + m_world_pom.v33 * RZ;
	}
}


void Emitter::UpdateParticle( Particle *particle)
{
	particle->m_current_lifetime += m_diff_time;

	particle->UpdateSpeed(m_diff_time_div_1000, Gravity() + get_additional_acceleration() );
	particle->UpdatePosition(m_diff_time_div_1000);
	particle->SetColor( get_color());

	//TODO: update other stuff
}


void Emitter::Simulate( KCL::uint32 time)
{
	if(IsPaused())
	{
		return;
	}

	KCL::uint32 emitCount = Emit( time);
	
	m_visibleparticle_count = 0;
	
	for(size_t i=0; i<m_particles.size(); ++i)
	{
		if( m_particles[i]->IsAlive())
		{
			UpdateParticle( m_particles[i]);
		}

		if( m_particles[i]->IsDead() )
		{
			if( emitCount )
			{
				ResurrectParticle( m_particles[i]);
				--emitCount;
			}
		}

		if( m_particles[i]->IsAlive())
		{
			m_particle_render_attribs[m_visibleparticle_count].SetPosition( m_particles[i]->m_world_position);
			m_particle_render_attribs[m_visibleparticle_count].SetLife( m_particles[i]->LifeNormalized(), m_particles[i]->FractionalLifetime());
			m_particle_render_attribs[m_visibleparticle_count].SetSpeed( m_particles[i]->m_speed);
			m_particle_render_attribs[m_visibleparticle_count].SetColor( m_particles[i]->m_color);

			++m_visibleparticle_count;
		}
	}
}


template< typename _DATA> struct _distance_sorter
{
	_DATA m_data;
	float m_distance;

	static int compare(const void* a,const  void* b)
	{
		_distance_sorter<_DATA> *A = (_distance_sorter<_DATA>*)a;
		_distance_sorter<_DATA> *B = (_distance_sorter<_DATA>*)b;

		if (A->m_distance > B->m_distance)
			return -1;
		else if (A->m_distance == B->m_distance)
			return 0;
		else 
			return 1;
	}


	static int inv_compare(const void* a,const  void* b)
	{
		_distance_sorter<_DATA> *A = (_distance_sorter<_DATA>*)a;
		_distance_sorter<_DATA> *B = (_distance_sorter<_DATA>*)b;

		if (A->m_distance < B->m_distance)
			return -1;
		else if (A->m_distance == B->m_distance)
			return 0;
		else 
			return 1;
	}

};



void Emitter::DepthSort( const Camera2 &camera)
{
	if( !m_visibleparticle_count)
	{
		return;
	}
	std::vector<_distance_sorter<ParticleRenderAttrib> > s;

	for( KCL::uint32 i=0; i<m_visibleparticle_count; i++)
	{
		_distance_sorter<ParticleRenderAttrib> ds;

		ds.m_data = m_particle_render_attribs[i];
		ds.m_distance = 
			camera.GetCullPlane( 5).x * m_particle_render_attribs[i].m_position_x + 
			camera.GetCullPlane( 5).y * m_particle_render_attribs[i].m_position_y + 
			camera.GetCullPlane( 5).z * m_particle_render_attribs[i].m_position_z + 
			camera.GetCullPlane( 5).w;
		s.push_back( ds);
	}

	qsort( &s[0], s.size(), sizeof( _distance_sorter<ParticleRenderAttrib>), &_distance_sorter<ParticleRenderAttrib>::compare);

	for( KCL::uint32 i=0; i<m_visibleparticle_count; i++)
	{
		m_particle_render_attribs[i] = s[i].m_data;
	}
}



//////////////////////////////////////

void DummyEmitter::Init(KCL::uint32 max_particle_count)
{
	max_particle_count = 4000;
	Emitter::Init(max_particle_count);

	m_gravity.y = -0.00000005f;

	m_is_pause = false;
	m_is_active = false;
	m_prev_time = -1;

	m_is_active = true;
	m_is_local_emitting = false;

	m_max_angle = 20.0f;
	m_spawning_rate = 1;
	
	
	m_aperture_x = 0.05f;
	m_aperture_z = 0.05f;
	m_max_initial_velocity = 0.00003f;
	m_min_initial_velocity = 0.00007f;

	m_lifetime_range[0] = 5*1000;
	m_lifetime_range[0] = 5*3000;


	KCL::Matrix4x4::Identity(m_world_pom);
	m_world_pom.v41 = 5;
}


float DummyEmitter::get_spawning_rate()
{
	return m_spawning_rate;
}


float DummyEmitter::get_max_angle()
{
	return m_max_angle;
}


float DummyEmitter::get_aperture_x()
{
	return m_aperture_x;
}


float DummyEmitter::get_aperture_z()
{
	return m_aperture_z;
}


float DummyEmitter::get_min_initial_velocity()
{
	return m_min_initial_velocity;
}


float DummyEmitter::get_max_initial_velocity()
{
	return m_max_initial_velocity;
}


float DummyEmitter::get_min_initial_lifetime()
{
	return m_lifetime_range[0];
}


float DummyEmitter::get_max_initial_lifetime()
{
	return m_lifetime_range[1];
}


const KCL::Vector3D DummyEmitter::get_additional_acceleration()
{
	return KCL::Vector3D();
}

//////////////////////////////////////


KCL::AnimatedEmitter::AnimatedEmitter( const std::string &name, ObjectType type, Node *parent, Object *owner) : Emitter( name, type, parent, owner)
{
	m_spawning_rate_animated = 0;
	m_max_angle_animated = 0;
	m_aperture_x_animated = 0;
	m_aperture_z_animated = 0;
	m_min_initial_velocity_animated = 0;
	m_max_initial_velocity_animated = 0;
	m_min_initial_lifetime_animated = 0;
	m_max_initial_lifetime_animated = 0;
	m_additional_acceleration_animated = 0;


	m_spawning_rate = 0;
	m_max_angle = 0;
	m_aperture_x = 0;
	m_aperture_z = 0;
	m_min_initial_velocity = 0;
	m_max_initial_velocity = 0;
	m_min_initial_lifetime = 0;
	m_max_initial_lifetime = 0;
}


KCL::AnimatedEmitter::~AnimatedEmitter()
{
	delete m_spawning_rate_animated;
	delete m_max_angle_animated;
	delete m_aperture_x_animated;
	delete m_aperture_z_animated;
	delete m_min_initial_velocity_animated;
	delete m_max_initial_velocity_animated;
	delete m_min_initial_lifetime_animated;
	delete m_max_initial_lifetime_animated;
	delete m_additional_acceleration_animated;
}

void KCL::AnimatedEmitter::Init(KCL::uint32 max_particle_count)
{
	Emitter::Init(max_particle_count);
	//TODO ?
}

void KCL::AnimatedEmitter::Set_Spawning_rate_animated ( _key_node* spawning_rate )
{
	if( m_spawning_rate_animated )
	{
		delete m_spawning_rate_animated;
	}
	m_spawning_rate_animated = spawning_rate;
}


void KCL::AnimatedEmitter::Set_Max_angle_animated ( _key_node* max_angle )
{
	if( m_max_angle_animated )
	{
		delete m_max_angle_animated;
	}
	m_max_angle_animated = max_angle;
}


void KCL::AnimatedEmitter::Set_Aperture_x_animated ( _key_node* aperture_x )
{
	if( m_aperture_x_animated )
	{
		delete m_aperture_x_animated;
	}
	m_aperture_x_animated = aperture_x;
}


void KCL::AnimatedEmitter::Set_Aperture_z_animated ( _key_node* aperture_z )
{
	if( m_aperture_z_animated )
	{
		delete m_aperture_z_animated; 
	}m_aperture_z_animated = aperture_z;
}


void KCL::AnimatedEmitter::Set_Min_initial_velocity_animated ( _key_node* min_initial_velocity )
{
	if( m_min_initial_velocity_animated )
	{
		delete m_min_initial_velocity_animated;
	}
	m_min_initial_velocity_animated = min_initial_velocity;
}


void KCL::AnimatedEmitter::Set_Max_initial_velocity_animated ( _key_node* max_initial_velocity )
{
	if( m_max_initial_velocity_animated )
	{
		delete m_max_initial_velocity_animated;
	}
	m_max_initial_velocity_animated = max_initial_velocity;
}


void KCL::AnimatedEmitter::Set_Min_initial_lifetime_animated ( _key_node* min_initial_lifetime )
{
	if( m_min_initial_lifetime_animated )
	{
		delete m_min_initial_lifetime_animated;
	}
	m_min_initial_lifetime_animated = min_initial_lifetime;
}


void KCL::AnimatedEmitter::Set_Max_initial_lifetime_animated ( _key_node* max_initial_lifetime )
{
	if( m_max_initial_lifetime_animated )
	{
		delete m_max_initial_lifetime_animated;
	}
	m_max_initial_lifetime_animated = max_initial_lifetime;
}


void KCL::AnimatedEmitter::Set_Additional_acceleration_animated ( _key_node* additional_acceleration)
{
	if( m_additional_acceleration_animated )
	{
		delete m_additional_acceleration_animated;
	}
	m_additional_acceleration_animated = additional_acceleration;
}


float KCL::AnimatedEmitter::get_spawning_rate()
{
	if(m_spawning_rate_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		
		_key_node::Get(result, m_spawning_rate_animated, m_prev_time_div_1000, nothing);

		return result.x;
	}
	else
	{
		return m_spawning_rate;
	}
}


float KCL::AnimatedEmitter::get_max_angle()
{
	if(m_max_angle_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_max_angle_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_max_angle;
	}
}


float KCL::AnimatedEmitter::get_aperture_x()
{
	if(m_aperture_x_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_aperture_x_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_aperture_x;
	}
}


float KCL::AnimatedEmitter::get_aperture_z()
{
	if(m_aperture_z_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_aperture_z_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_aperture_z;
	}
}


float KCL::AnimatedEmitter::get_min_initial_velocity()
{
	if(m_min_initial_velocity_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_min_initial_velocity_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_min_initial_velocity;
	}
}


float KCL::AnimatedEmitter::get_max_initial_velocity()
{
	if(m_max_initial_velocity_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_max_initial_velocity_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_max_initial_velocity;
	}
}


float KCL::AnimatedEmitter::get_min_initial_lifetime()
{
	if(m_min_initial_lifetime_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_min_initial_lifetime_animated, m_prev_time_div_1000, nothing);
		return result.x;
	}
	else
	{
		return m_min_initial_lifetime;
	}
}


float KCL::AnimatedEmitter::get_max_initial_lifetime()
{
	if(m_max_initial_lifetime_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_max_initial_lifetime_animated, m_prev_time_div_1000, nothing);
		return result.x;}
	else
	{
		return m_max_initial_lifetime;
	}
}


const KCL::Vector3D KCL::AnimatedEmitter::get_additional_acceleration()
{
	if(m_additional_acceleration_animated)
	{
		float nothing=0;
		KCL::Vector4D result;
		_key_node::Get(result, m_additional_acceleration_animated, m_prev_time_div_1000, nothing);
		return KCL::Vector3D(result.x, result.y, result.z);}
	else
	{
		return m_additional_acceleration;
	}
}


const KCL::Vector4D KCL::AnimatedEmitter::get_color()
{
	if(m_alpha_track)
	{
		return KCL::Vector4D( m_color.x * m_alpha, m_color.y * m_alpha, m_color.z * m_alpha, 1.0);
	}
	else
	{
		return m_color;
	}
}


AnimatedEmitterFactory::~AnimatedEmitterFactory()
{
}


void AnimatedEmitter::LoadParameters(KCL::SceneHandler *scene)
{
	AssetFile emitter_file(GetParameterFilename());
	if (!emitter_file.Opened())
	{
		//printf("!!!error: no emitter found - %s\n", name.c_str());
		return ;
	}

	Vector3D accel;
	while (!emitter_file.eof())
	{
		char buff[4096];
		emitter_file.Gets(buff, 4096);
		std::stringstream ss;
		char string0[512];
		char string1[512];

		ss << buff;
		ss >> string0;

		if (strcmp(string0, "rate_track") == 0)
		{
			ss >> string1;
			m_rate_track_name = string1;
			{
				_key_node* spawning_rate_animated = 0;

				scene->ReadAnimation(spawning_rate_animated, string1);

				if (spawning_rate_animated)
				{
					Set_Spawning_rate_animated(spawning_rate_animated);
				}
			}
		}
		else if (strcmp(string0, "type") == 0)
		{
			ss >> string1;
			if (strcmp(string1, "spark") == 0)
			{
				InitAsSpark();
			}
			else if (strcmp(string1, "fire") == 0)
			{
				InitAsFire();
			}
			else if (strcmp(string1, "smoke") == 0)
			{
				InitAsSmoke();
			}
			else if (strcmp(string1, "steam") == 0)
			{
				InitAsSteam();
			}
			else
			{
				InitAsSmoke();
			}
		}
		else if (strcmp(string0, "accelerationX") == 0)
		{
			ss >> string1;
			accel.x = ng::atof(string1);
		}
		else if (strcmp(string0, "accelerationY") == 0)
		{
			ss >> string1;
			accel.y = ng::atof(string1);
		}
		else if (strcmp(string0, "accelerationZ") == 0)
		{
			ss >> string1;
			accel.z = ng::atof(string1);
		}
		else if (strcmp(string0, "aperture_x") == 0)
		{
			ss >> string1;
			Set_Aperture_x(ng::atof(string1));
		}
		else if (strcmp(string0, "aperture_y") == 0)
		{
			ss >> string1;
			Set_Aperture_z(ng::atof(string1));
		}
		else if (strcmp(string0, "max_angle") == 0)
		{
			ss >> string1;
			Set_Max_angle(ng::atof(string1));
		}
		else if (strcmp(string0, "max_initial_lifetime") == 0)
		{
			ss >> string1;
			Set_Max_initial_lifetime(ng::atof(string1));
		}
		else if (strcmp(string0, "max_initial_velocity") == 0)
		{
			ss >> string1;
			Set_Max_initial_velocity(ng::atof(string1));
		}
		else if (strcmp(string0, "min_initial_lifetime") == 0)
		{
			ss >> string1;
			Set_Min_initial_lifetime(ng::atof(string1));
		}
		else if (strcmp(string0, "min_initial_velocity") == 0)
		{
			ss >> string1;
			Set_Min_initial_velocity(ng::atof(string1));
		}
		else if (strcmp(string0, "rate") == 0)
		{
			ss >> string1;
			Set_Spawning_rate(ng::atof(string1));
		}
		else if (strcmp(string0, "color") == 0)
		{
			KCL::Vector4D color;

			ss >> color.x;
			ss >> color.y;
			ss >> color.z;

			SetColor(color);
		}
	}

	Set_Additional_acceleration(accel);
	Init(MAX_PARTICLE_PER_EMITTER);
}


void AnimatedEmitter::SaveParameters(KCL::SceneHandler *scene)
{
	INFO("not implemented!");
}


const KCL::Vector4D KCL::DummyEmitter::get_color()
{
	return m_color;
}
