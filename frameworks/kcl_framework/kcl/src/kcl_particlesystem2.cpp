/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_particlesystem2.h"
#include "kcl_os.h"
#include <float.h>

//#include "emitter.h"
//#include "0/math/sakura_math.h"
//#include "6/sakuraSDK/camera/camera.h"
//#include "6/sakuraSDK/globals.h"

using namespace KCL;

int seed = 0;

#ifndef M_PI
#	define M_PI 3.141592654f
#endif
#define DEG2RAD(X) ((X)*M_PI/180.0f)
inline float lrp (float i, float a, float b)
{
	return ( a * (1.0f - i) + b * i);
}
float rand_unsigned_clamped()
{
	return KCL::Math::randomf( &seed);
}
float rand_signed_clamped()
{
	return KCL::Math::randomf_signed( &seed);
}
enum _waveFuncType
{
	WFT_SINE=0,
	WFT_TRIANGLE,
	WFT_SQUARE,
	WFT_SAW,
	WFT_INVSAW,
	WFT_ABS_SINE,
	WFT_CUSTOM,
	WFT_MAX
};
#define SAKURA_WAVETABLE_SIZE 2048
#define SAKURA_WAVETABLE_MAX SAKURA_WAVETABLE_SIZE-1
static float s_wave_tables[WFT_MAX][SAKURA_WAVETABLE_SIZE];
void InitWaveTables()
{
	for(KCL::uint32 i=0;i<SAKURA_WAVETABLE_SIZE;i++)
	{
		s_wave_tables[WFT_SINE][i]=sin(DEG2RAD(i*360.0f/((float)SAKURA_WAVETABLE_MAX)));
		s_wave_tables[WFT_ABS_SINE][i]=sin(DEG2RAD(i*180.0f/((float)SAKURA_WAVETABLE_MAX)));
		s_wave_tables[WFT_SQUARE][i]=(i<SAKURA_WAVETABLE_SIZE/2) ? 1.0f : -1.0f;
		s_wave_tables[WFT_SAW][i]=(float)i/ SAKURA_WAVETABLE_SIZE;
		s_wave_tables[WFT_INVSAW][i]=1.0f-s_wave_tables[WFT_SAW][i];

		if(i<SAKURA_WAVETABLE_SIZE/2)
		{
			if(i<SAKURA_WAVETABLE_SIZE/4)
			{
				s_wave_tables[WFT_TRIANGLE][i]=(float)i/(SAKURA_WAVETABLE_SIZE/4);
			}
			else
			{
				s_wave_tables[WFT_TRIANGLE][i]=1.0f-s_wave_tables[WFT_TRIANGLE][i-SAKURA_WAVETABLE_SIZE/4];
			}
		}
		else
		{
			s_wave_tables[WFT_TRIANGLE][i]=-s_wave_tables[WFT_TRIANGLE][i-SAKURA_WAVETABLE_SIZE/2];
		}
	}
}
float WaveFunc(_waveFuncType type, float time,float base,float amplitude,float phase,float frequency)
{
	return s_wave_tables[type][ (KCL::int32)((time * frequency + phase)*SAKURA_WAVETABLE_SIZE) & SAKURA_WAVETABLE_MAX]*amplitude + base;
}














_particle::_particle()
{
	m_age = FLT_MAX;
}


void _particle::Init( KCL::Vector3D *pos, KCL::Vector3D *t, KCL::Vector3D *b, KCL::Vector3D *n, _particle &A, _particle &B)
{
	m_speed = lrp( rand_unsigned_clamped(), A.m_speed, B.m_speed);
	m_acceleration = lrp( rand_unsigned_clamped(), A.m_acceleration, B.m_acceleration);

	m_phase.x = rand_unsigned_clamped();
	m_phase.y = rand_unsigned_clamped();
	m_phase.z = rand_unsigned_clamped();

	m_frequency.x = lrp( rand_unsigned_clamped(), A.m_frequency.x, B.m_frequency.x);
	m_frequency.y = lrp( rand_unsigned_clamped(), A.m_frequency.y, B.m_frequency.y);
	m_frequency.z = lrp( rand_unsigned_clamped(), A.m_frequency.z, B.m_frequency.z);

	m_amplitude.x = lrp( rand_unsigned_clamped(), A.m_amplitude.x, B.m_amplitude.x);
	m_amplitude.y = lrp( rand_unsigned_clamped(), A.m_amplitude.y, B.m_amplitude.y);
	m_amplitude.z = lrp( rand_unsigned_clamped(), A.m_amplitude.z, B.m_amplitude.z);

	m_pos = *pos;
	m_t = *t;
	m_b = *b;
	m_n = *n;
	m_age = 0;

}


void _particle::Calculate( float delta_time, KCL::Vector3D *external_velocity, float gravity)
{
	const int gravity_axis = 1;
	float t_turb = WaveFunc( WFT_SINE, m_age, 0, m_amplitude.x, m_phase.x, m_frequency.x);
	float b_turb = WaveFunc( WFT_SINE, m_age, 0, m_amplitude.y, m_phase.y, m_frequency.y);
	float n_turb = WaveFunc( WFT_SINE, m_age, 0, m_amplitude.z, m_phase.z, m_frequency.z);

	m_velocity.x = m_n.x * (m_speed + m_age * m_acceleration);
	m_velocity.y = m_n.y * (m_speed + m_age * m_acceleration);
	m_velocity.z = m_n.z * (m_speed + m_age * m_acceleration);

	m_velocity.x += m_t.x * t_turb + m_b.x * b_turb + m_n.x * n_turb;
	m_velocity.y += m_t.y * t_turb + m_b.y * b_turb + m_n.y * n_turb;
	m_velocity.z += m_t.z * t_turb + m_b.z * b_turb + m_n.z * n_turb;

	m_velocity.x += external_velocity->x;
	m_velocity.y += external_velocity->y;
	m_velocity.z += external_velocity->z;

	m_velocity.v[gravity_axis] -= m_age * gravity;

	m_pos.x += m_velocity.x;
	m_pos.y += m_velocity.y;
	m_pos.z += m_velocity.z;

	m_age += delta_time;
}


_emitter::_emitter( const std::string &name, ObjectType type, Node *parent, Object *owner) : AnimatedEmitter( name, type, parent, owner)
{
	InitWaveTables();

	m_emitter_type2 = ET_BILLBOARD;

	m_focus_distance = 1;

	m_lifespan = 1.0f;

	m_min.m_frequency.x = 0.0f;
	m_min.m_frequency.y = 0.0f;
	m_min.m_frequency.z = 0.0f;

	m_max.m_frequency.x = 0.0f;
	m_max.m_frequency.y = 0.0f;
	m_max.m_frequency.z = 0.0f;

	m_min.m_amplitude.x = 0.0f;
	m_min.m_amplitude.y = 0.0f;
	m_min.m_amplitude.z = 0.0f;

	m_max.m_amplitude.x = 0.0f;
	m_max.m_amplitude.y = 0.0f;
	m_max.m_amplitude.z = 0.0f;

	m_min.m_speed = 0.1f;
	m_max.m_speed = 0.2f;

	m_min.m_acceleration = 0.0f;
	m_max.m_acceleration = 0.0f;

	m_gravity_factor = 0.0f;

	m_external_velocity.x = 0.08f;
	m_external_velocity.y = 0.0f;
	m_external_velocity.z = 0.0f;

	m_last_particle_index = 0;
	m_last_active_particle_index = 0;
	m_num_active_particles = 0;
	m_emit_count = 0;
	m_accumulated_diff_time = 0.0f;

	m_rate = 1.0f;

	m_prev_time2 = 0;
	m_time = 0;

	m_begin_size = 1.0f;
	m_end_size = 1.0f;
}


_emitter::~_emitter()
{
}


void _emitter::InitAsSoot()
{
	m_emitter_type2 = ET_SPARK;

	m_focus_distance = 100;

	m_aperture.x = 2.1f;
	m_aperture.y = 2.1f;
	m_aperture.z = 0.0f;

	m_lifespan = 3.0f;

	m_min.m_frequency.x = 0.1f;
	m_min.m_frequency.y = 0.1f;
	m_min.m_frequency.z = 0.1f;

	m_max.m_frequency.x = 2.5f;
	m_max.m_frequency.y = 2.5f;
	m_max.m_frequency.z = 2.0f;

	m_min.m_amplitude.x = 0.1f;
	m_min.m_amplitude.y = 0.1f;
	m_min.m_amplitude.z = 0.1f;

	m_max.m_amplitude.x = 0.2f;
	m_max.m_amplitude.y = 0.2f;
	m_max.m_amplitude.z = 0.2f;

	m_min.m_speed = 0.05f;
	m_max.m_speed = 0.1f;

	m_min.m_acceleration = 0.f;
	m_max.m_acceleration = 0.1f;

	m_color.x = 2.0f;
	m_color.y = 0.8f;
	m_color.z = 0.2f;

	m_rate = 30.0f;

	m_gen_type = 1;
}


void _emitter::InitAsFire()
{
	m_focus_distance = 8;

	m_aperture.x = 2.0f;
	m_aperture.y = 2.0f;
	m_aperture.z = 0.0f;

	m_lifespan = 1.0f;

	m_min.m_frequency.x = 0.1f;
	m_min.m_frequency.y = 0.1f;
	m_min.m_frequency.z = 0.0f;

	m_max.m_frequency.x = 0.2f;
	m_max.m_frequency.y = 0.2f;
	m_max.m_frequency.z = 0.0f;

	m_min.m_amplitude.x = 0.0f;
	m_min.m_amplitude.y = 0.0f;
	m_min.m_amplitude.z = 0.0f;

	m_max.m_amplitude.x = 0.0f;
	m_max.m_amplitude.y = 0.0f;
	m_max.m_amplitude.z = 0.0f;

	m_min.m_speed = 0.1f;
	m_max.m_speed = 0.2f;

	m_min.m_acceleration = 0.1f;
	m_max.m_acceleration = 0.3f;

	m_color.x = 2.0f;
	m_color.y = 0.8f;
	m_color.z = 0.2f;

	m_begin_size = 2.5f;
	m_end_size = 1.0f;

	m_rate = 100.0f;

	m_gen_type = 0;
}


void _emitter::InitAsSmoke()
{
	m_focus_distance = -19.0f;

	m_aperture.x = 2.4f;
	m_aperture.y = 2.4f;
	m_aperture.z = 2.0f;

	m_lifespan = 4.0f;

	m_min.m_frequency.x = 0.05f;
	m_min.m_frequency.y = 0.05f;
	m_min.m_frequency.z = 0.0f;

	m_max.m_frequency.x = 1.1f;
	m_max.m_frequency.y = 1.1f;
	m_max.m_frequency.z = 0.0f;

	m_min.m_amplitude.x = 0.02f;
	m_min.m_amplitude.y = 0.02f;
	m_min.m_amplitude.z = 0.000f;

	m_max.m_amplitude.x = 0.04f;
	m_max.m_amplitude.y = 0.04f;
	m_max.m_amplitude.z = 0.001f;

	m_min.m_speed = 0.01f;
	m_max.m_speed = 0.2f;

	m_min.m_acceleration = -0.001f;
	m_max.m_acceleration = -0.002f;

	m_color.x = 0.4f;
	m_color.y = 0.4f;
	m_color.z = 0.4f;

	m_begin_size = 2.1f;
	m_end_size = 20.0f;

	m_rate = 40.0f;

	m_gen_type = 0;
}


void _emitter::InitAsSpark()
{
	m_focus_distance = -0.01f;

	m_aperture.x = 0.02f;
	m_aperture.y = 0.02f;
	m_aperture.z = 0.0f;

	m_lifespan = 1.0f;

	m_min.m_frequency.x = 0.1f;
	m_min.m_frequency.y = 0.1f;
	m_min.m_frequency.z = 0.1f;

	m_max.m_frequency.x = 1.5f;
	m_max.m_frequency.y = 1.5f;
	m_max.m_frequency.z = 1.0f;

	m_min.m_amplitude.x = 0.0f;
	m_min.m_amplitude.y = 0.0f;
	m_min.m_amplitude.z = 0.0f;

	m_max.m_amplitude.x = 0.0f;
	m_max.m_amplitude.y = 0.0f;
	m_max.m_amplitude.z = 0.0f;

	m_min.m_speed = 0.01f;
	m_max.m_speed = 0.05f;

	m_min.m_acceleration = 0.0f;
	m_max.m_acceleration = 0.0f;

	m_gravity_factor = 0.1f;

	m_color.x = 1.0f;
	m_color.y = 0.4f;
	m_color.z = 0.1f;

	m_rate = 40.0f;

	m_gen_type = 1;
}


void _emitter::InitParticle( _particle &pp)
{
	KCL::Vector3D pos;
	KCL::Vector3D t;
	KCL::Vector3D b;
	KCL::Vector3D n;
	KCL::Vector3D delta;
	KCL::Vector3D pos_offset;
	KCL::Vector3D focus_offset;

	//lokalis pozicio offset az aperturaban
	pos_offset.x = m_aperture.x * rand_signed_clamped();
	pos_offset.y = m_aperture.z * rand_signed_clamped();
	pos_offset.z = m_aperture.y * rand_signed_clamped();

	//globalis fokusz offszet
	focus_offset.x = m_world_pom.v[4] * m_focus_distance;
	focus_offset.y = m_world_pom.v[5] * m_focus_distance;
	focus_offset.z = m_world_pom.v[6] * m_focus_distance;

	//globalis pozicio offszet
	delta.x = pos_offset.x * m_world_pom.v[0] + pos_offset.y * m_world_pom.v[4] +  pos_offset.z * m_world_pom.v[8];
	delta.y = pos_offset.x * m_world_pom.v[1] + pos_offset.y * m_world_pom.v[5] +  pos_offset.z * m_world_pom.v[9];
	delta.z = pos_offset.x * m_world_pom.v[2] + pos_offset.y * m_world_pom.v[6] +  pos_offset.z * m_world_pom.v[10];

	//globalis pozicio
	pos.x = delta.x + m_world_pom.v[12];
	pos.y = delta.y + m_world_pom.v[13];
	pos.z = delta.z + m_world_pom.v[14];

	//globalis elore vektor
	if( m_focus_distance > 0)
	{
		n.x = focus_offset.x - delta.x;
		n.y = focus_offset.y - delta.y;
		n.z = focus_offset.z - delta.z;
	}
	else
	{
		n.x = delta.x - focus_offset.x;
		n.y = delta.y - focus_offset.y;
		n.z = delta.z - focus_offset.z;
	}
	n.normalize();

	b = KCL::Vector3D::cross( n, KCL::Vector3D( &m_world_pom.v[0]));
	b.normalize();

	t = KCL::Vector3D::cross( b, n);
	t.normalize();

	pp.Init( &pos, &t, &b, &n, m_min, m_max);
}


KCL::uint32 _emitter::CalculateNumSubsteps( float diff_time_sec)
{
	const float diff_time_threshold = 0.025f;
	int num_substeps = 0;

	m_accumulated_diff_time += diff_time_sec;

	if( m_accumulated_diff_time < diff_time_threshold)
	{
		num_substeps = 0;
	}
	else
	{
		num_substeps = KCL::uint32( m_accumulated_diff_time / diff_time_threshold) + 1;

		if( num_substeps > max_num_substeps)
		{
			num_substeps = max_num_substeps;
		}

		m_accumulated_diff_time = 0;
	}

	return num_substeps;
}


void _emitter::Simulate( uint32 time_msec)
{	
	const float animate_diff_time = 0.025f;
	const float rate_divider = 40.0f;

	m_prev_time2 = m_time;
	m_time = time_msec;

	float diff_time_sec = (m_time - m_prev_time2) / 1000.0f;

	float rate = m_rate;
	int num_substeps = CalculateNumSubsteps( diff_time_sec);

	//INFO("%d\n", m_num_active_particles);

	if( m_spawning_rate_animated)
	{
		float nothing=0;
		float t = time_msec / 1000.0f;

		KCL::Vector4D result;
		
		_key_node::Get(result, m_spawning_rate_animated, t, nothing);

		if( result.x > 1.0f)
		{
			result.x = 1.0f;
		}
		rate *= result.x;
	}

	for( int substep=0; substep<num_substeps; substep++)
	{
		m_emit_count += rate / rate_divider;

		for( KCL::uint32 i=0; i<(KCL::uint32)m_emit_count; i++)
		{
			_particle &pp = particle_parameters[m_last_particle_index];

			if( m_num_active_particles < MAX_PARTICLES_PER_EMITTER - 1)
			{
				m_num_active_particles++;
			}

			InitParticle( pp);

			ParticleIdxIncr( m_last_particle_index);
		}

		if( m_emit_count >= 1.0f)
		{
			m_emit_count = 0.0f;
		}

		KCL::uint32 num_active_particles = m_num_active_particles;
		KCL::uint32 idx = m_last_active_particle_index;

		for( KCL::uint32 i=0; i<num_active_particles; i++)
		{
			_particle &pp = particle_parameters[idx];

			if( pp.m_age > m_lifespan)
			{
				m_num_active_particles--;

				ParticleIdxIncr( m_last_active_particle_index);
			}
			else
			{
				pp.Calculate( animate_diff_time, &m_external_velocity, m_gravity_factor);
			}

			ParticleIdxIncr( idx);
		}
	}

	if( num_substeps > 0)
	{
		CreateGeometry();
	}
}


void _emitter::CreateGeometry()
{
	KCL::uint32 idx = m_last_active_particle_index;
	m_visibleparticle_count = 0;

	for( KCL::uint32 i=0; i<m_num_active_particles; i++)
	{
		_particle &pp = particle_parameters[idx];

		if( pp.m_age < m_lifespan)
		{
			float inte = pp.m_age / m_lifespan;

			attribs[m_visibleparticle_count].m_pos = pp.m_pos;
			attribs[m_visibleparticle_count].m_life_normalized = 1.0f - inte;
			attribs[m_visibleparticle_count].m_velocity = pp.m_velocity;
			attribs[m_visibleparticle_count].m_size = lrp( inte, m_begin_size, m_end_size);

			m_visibleparticle_count++;
		}

		ParticleIdxIncr( idx);
	}
}
