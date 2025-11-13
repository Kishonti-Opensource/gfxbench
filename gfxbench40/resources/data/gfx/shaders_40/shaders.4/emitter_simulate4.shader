/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#define WORK_GROUP_HEIGHT 1
#define M_PI 3.141592654
#define WFT_SINE 0

#define m_age g_particle.Age_Speed_Accel_Active.x
#define m_speed g_particle.Age_Speed_Accel_Active.y
#define m_acceleration g_particle.Age_Speed_Accel_Active.z
#define m_is_active g_particle.Age_Speed_Accel_Active.w

#define m_freq g_particle.Frequency

#define m_amplitude g_particle.Amplitude.xyz
#define m_phase g_particle.Phase.xyz
#define m_frequency g_particle.Frequency.xyz
#define m_t g_particle.T.xyz
#define m_b g_particle.B.xyz
#define m_n g_particle.N.xyz
#define m_velocity g_particle.Velocity.xyz
#define m_pos g_particle.Pos.xyz

#ifdef SINGLE_FRAME_MODE
uniform float delta_time ;
#endif
	
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
	vec4 Age_Speed_Accel_Active;
};


struct _emitter
{
	ivec4 EmitCount_LastParticleIndex_NumSubSteps;
	mat4 pom;
	vec4 external_velocity;
	vec4 aperture;
	vec4 Focus_Life_Rate_Gravity;
	vec4 SpeedMin_SpeedMax_AccelMin_AccelMax;
	vec4 FreqMin;
	vec4 FreqMax;
	vec4 AmpMin;
	vec4 AmpMax;
};

_particle g_particle ;

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


void CalculateParticle(vec3 external_velocity, float gravity, float delta_time)
{
	float t_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.x, m_phase.x, m_frequency.x);
	float b_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.y, m_phase.y, m_frequency.y);
	float n_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.z, m_phase.z, m_frequency.z);

	vec3 dir;

	dir.x = m_t.x * t_turb + m_b.x * b_turb + m_n.x * (n_turb + 1.0);
	dir.y = m_t.y * t_turb + m_b.y * b_turb + m_n.y * (n_turb + 1.0);
	dir.z = m_t.z * t_turb + m_b.z * b_turb + m_n.z * (n_turb + 1.0);

	dir = normalize( dir);
	//dir = m_n;

	m_velocity = (dir * m_speed + external_velocity + m_age * m_acceleration);
	m_velocity.y -= m_age * gravity;

	m_pos.x += m_velocity.x * delta_time;
	m_pos.y += m_velocity.y * delta_time;
	m_pos.z += m_velocity.z * delta_time;

	// m_pos.x = 0.5;
	// m_pos.y = 0.5;
	// m_pos.z = 0.5;

	m_age += delta_time;
}

void InitParticle()
{
	m_age = 0.0;

	vec3 emitter_pos = vec3( emitter.pom[3][0], emitter.pom[3][1], emitter.pom[3][2]);
	vec3 emitter_focus_dir = vec3( emitter.pom[0][0], emitter.pom[0][1], emitter.pom[0][2]);
	vec3 emitter_aperture_dirx = vec3( emitter.pom[1][0], emitter.pom[1][1], emitter.pom[1][2]);
	vec3 emitter_aperture_diry = vec3( emitter.pom[2][0], emitter.pom[2][1], emitter.pom[2][2]);

	vec3 random = m_phase.xyz * vec3( 2.0) - vec3( 1.0);
	vec3 aperture_offset;

	aperture_offset =
		emitter_aperture_dirx * random.y * emitter.aperture.x +
		emitter_aperture_diry * random.z * emitter.aperture.y +
		emitter_focus_dir * random.z * emitter.aperture.z;

	m_pos = emitter_pos + aperture_offset;

	vec4 side = vec4( emitter_aperture_dirx, 0.0);

	vec4 focus_offset;

	focus_offset.xyz = emitter_pos + emitter_focus_dir * emitter.Focus_Life_Rate_Gravity.x;

	if( emitter.Focus_Life_Rate_Gravity.x > 0.0)
	{
		m_n = focus_offset.xyz - m_pos;
	}
	else
	{
		m_n = m_pos - focus_offset.xyz;
	}

	m_n = normalize( m_n);

	m_b = cross( m_n, side.xyz);
	m_b = normalize( m_b);

	m_t = cross( m_b, m_n);
	m_t = normalize( m_t);

	m_speed = mix( emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.x, emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.y,  m_phase.x);
	m_acceleration = mix( emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.z, emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.w,  m_phase.x);

	m_freq = mix(emitter.FreqMin, emitter.FreqMax, m_phase.x);
	m_amplitude = mix(emitter.AmpMin.xyz, emitter.AmpMax.xyz, m_phase.x);

	m_is_active = 1.0;
}


#ifdef PARTICLE_EMIT
void main()
{
	if( int(gl_GlobalInvocationID.x) < emitter.EmitCount_LastParticleIndex_NumSubSteps.x)
	{
		int particle_idx = (emitter.EmitCount_LastParticleIndex_NumSubSteps.y + int(gl_GlobalInvocationID.x)) % MAX_PARTICLES;
		g_particle = particles[particle_idx];
		
		InitParticle();
		
		particles[particle_idx] = g_particle;
	}
}
#endif


#ifdef PARTICLE_SIMULATE
void main()
{
#ifndef SINGLE_FRAME_MODE
	float delta_time = 1.0 / 40.0;
#endif

	int particle_idx = int(gl_GlobalInvocationID.x);

	g_particle = particles[particle_idx];
	
	if( m_is_active > 0.5)
	{
		float predicted_age = m_age + delta_time * float(  emitter.EmitCount_LastParticleIndex_NumSubSteps.z);

		if( predicted_age >= emitter.Focus_Life_Rate_Gravity.y)
		{
			m_is_active = 0.0;

			m_pos.x = 0.0;
			m_pos.y = 100000.0;
			m_pos.z = 0.0;
		}
		else
		{
			for( int i=0; i<emitter.EmitCount_LastParticleIndex_NumSubSteps.z; i++)
			{
				CalculateParticle(emitter.external_velocity.xyz, emitter.Focus_Life_Rate_Gravity.w, delta_time);
			}
		}
		particles[particle_idx] = g_particle;
	}
}
#endif

#endif
