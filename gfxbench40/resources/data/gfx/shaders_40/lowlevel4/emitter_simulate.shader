/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


#define MAX_PARTICLES 128
#define WORK_GROUP_WIDTH 128
#define WORK_GROUP_HEIGHT 1
#define M_PI 3.141592654
#define WFT_SINE 0

#define m_age particles[particle_idx].Age_Speed_Accel.x
#define m_speed particles[particle_idx].Age_Speed_Accel.y
#define m_acceleration particles[particle_idx].Age_Speed_Accel.z

#define m_amplitude particles[particle_idx].Amplitude.xyz
#define m_phase particles[particle_idx].Phase.xyz
#define m_frequency particles[particle_idx].Frequency.xyz
#define m_t particles[particle_idx].T.xyz
#define m_b particles[particle_idx].B.xyz
#define m_n particles[particle_idx].N.xyz
#define m_velocity particles[particle_idx].Velocity.xyz
#define m_pos particles[particle_idx].Pos.xyz

struct _particle
{
	vec4 Pos; // particle instance's position
	vec4 Velocity;
	vec4 T;
	vec4 B;
	vec4 N;
	vec4 Phase;
	vec4 Frequency;
	vec4 Amplitude;
	vec4 Age_Speed_Accel;
};


struct _emitter
{
	mat4 pom;
	_particle minp;
	_particle maxp;
	vec4 external_velocity;
	vec4 aperture;
	vec4 Focus_Life_Rate_Gravity;
};


layout (local_size_x = WORK_GROUP_WIDTH, local_size_y = WORK_GROUP_HEIGHT) in;

layout (std430, binding = 0) buffer BufferObject
{
    _particle particles[MAX_PARTICLES];
};

layout( std140, binding = 1) uniform UniformObject
{
	_emitter emitter;
};



float rand(vec2 co)
{
	float f = (fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453));

	return f * 2.0 - 1.0;
}


float WaveFunc( int type, float time_, float base, float amplitude, float phase, float frequency)
{
	float time = time_;

	time = (time * frequency + phase);

	time *= 2.0 * M_PI;

	float f = sin( time) * amplitude + base;
	
	return f;
}


bool CalculateParticle( uint particle_idx, vec3 external_velocity, float gravity, float delta_time, float lifespan)
{
	float t_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.x, m_phase.x, m_frequency.x);
	float b_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.y, m_phase.y, m_frequency.y);
	float n_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.z, m_phase.z, m_frequency.z);

	m_velocity.x = m_n.x * (m_speed + m_age * m_acceleration);
	m_velocity.y = m_n.y * (m_speed + m_age * m_acceleration);
	m_velocity.z = m_n.z * (m_speed + m_age * m_acceleration);

	m_velocity.x += m_t.x * t_turb + m_b.x * b_turb + m_n.x * n_turb;
	m_velocity.y += m_t.y * t_turb + m_b.y * b_turb + m_n.y * n_turb;
	m_velocity.z += m_t.z * t_turb + m_b.z * b_turb + m_n.z * n_turb;

	m_velocity.x += external_velocity.x;
	m_velocity.y += external_velocity.y;
	m_velocity.z += external_velocity.z;

	m_velocity.y -= m_age * gravity;

	m_pos.x += m_velocity.x;
	m_pos.y += m_velocity.y;
	m_pos.z += m_velocity.z;

	// m_pos.x = 0.5;
	// m_pos.y = 0.5;
	// m_pos.z = 0.5;

	m_age += delta_time;
	
	return m_age >= lifespan;
}

void InitParticle( uint particle_idx)
{
	m_age = 0.0;

	m_pos.x = 0.0 +  rand( particles[particle_idx].Velocity.yx) * emitter.aperture.x;
	m_pos.z = 0.5 +  rand( particles[particle_idx].Velocity.xy) *emitter.aperture.y;
	m_pos.y = 0.0;

	vec4 oldalra = vec4( 1.0, 0.0, 0.0, 0.0);
	vec4 focus_offset = vec4( 0.0, 1.0 + emitter.Focus_Life_Rate_Gravity.x, 0.5, 0.0);

	if( emitter.Focus_Life_Rate_Gravity.x > 0.0)
	{
		m_n = focus_offset.xyz - m_pos;
	}
	else
	{
		m_n = m_pos - focus_offset.xyz;
	}
	
	m_n = normalize( m_n);

	m_b = cross( m_n, oldalra.xyz);
	m_b = normalize( m_b);

	m_t = cross( m_b, m_n);
	m_t = normalize( m_t);
}

#ifdef TYPE_compute
void main() 
{
	uint particle_idx = gl_GlobalInvocationID.x;
	
	const float delta_time = 1.0 / 60.0;
	
	bool reinit = CalculateParticle( particle_idx, emitter.external_velocity.xyz, emitter.Focus_Life_Rate_Gravity.w, delta_time, emitter.Focus_Life_Rate_Gravity.y);
	
	if( reinit)
	{
		InitParticle( particle_idx);
	}
}
#endif
