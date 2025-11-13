/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#ifdef TYPE_compute

#define WORK_GROUP_HEIGHT 1
#define M_PI 3.141592654
#define WFT_SINE 0

#define m_age particle.Age_Speed_Accel_Active.x
#define m_speed particle.Age_Speed_Accel_Active.y
#define m_acceleration particle.Age_Speed_Accel_Active.z
#define m_is_active particle.Age_Speed_Accel_Active.w

#define m_freq particle.Frequency

#define m_amplitude particle.Amplitude.xyz
#define m_phase particle.Phase.xyz
#define m_frequency particle.Frequency.xyz
#define m_t particle.T.xyz
#define m_b particle.B.xyz
#define m_n particle.N.xyz
#define m_velocity particle.Velocity.xyz
#define m_pos particle.Pos.xyz

struct _particle
{
	hfloat4 Pos; // particle instance's position
	hfloat4 Velocity;
	hfloat4 T;
	hfloat4 B;
	hfloat4 N;
	hfloat4 Phase;
	hfloat4 Frequency;
	hfloat4 Amplitude;
	hfloat4 Age_Speed_Accel_Active;
};

struct _emitter
{
	int4 EmitCount_LastParticleIndex_NumSubSteps;
	hfloat4x4 pom;
	hfloat4 external_velocity;
	hfloat4 aperture;
	hfloat4 Focus_Life_Rate_Gravity;
	hfloat4 SpeedMin_SpeedMax_AccelMin_AccelMax;
	hfloat4 FreqMin;
	hfloat4 FreqMax;
	hfloat4 AmpMin;
	hfloat4 AmpMax;
};

_float emitter_rand(_float2 co)
{
	_float f = (fract(sin(dot(co.xy ,_float2(12.9898,78.233))) * 43758.5453));

	return f * 2.0 - 1.0;
}


_float WaveFunc( int type, _float time_, _float base, _float amplitude, _float phase, _float frequency)
{
	_float time = time_;

	time = (time * frequency + phase);

	time *= 2.0 * M_PI;

	_float f = sin( time) * amplitude + base;

	return f;
}

void CalculateParticle(device _emitter& emitter, thread _particle& particle, _float3 external_velocity, _float gravity, _float delta_time)
{
	_float t_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.x, m_phase.x, m_frequency.x);
	_float b_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.y, m_phase.y, m_frequency.y);
	_float n_turb = WaveFunc( WFT_SINE, m_age, 0.0, m_amplitude.z, m_phase.z, m_frequency.z);

	_float3 dir;

	dir.x = m_t.x * t_turb + m_b.x * b_turb + m_n.x * (n_turb + 1.0);
	dir.y = m_t.y * t_turb + m_b.y * b_turb + m_n.y * (n_turb + 1.0);
	dir.z = m_t.z * t_turb + m_b.z * b_turb + m_n.z * (n_turb + 1.0);

	dir = normalize( dir);
	//dir = m_n;

	m_velocity = hfloat3(dir * _float3(m_speed) + external_velocity + m_age * m_acceleration);
	m_velocity.y -= m_age * gravity;

	m_pos.x += m_velocity.x * delta_time;
	m_pos.y += m_velocity.y * delta_time;
	m_pos.z += m_velocity.z * delta_time;

	// m_pos.x = 0.5;
	// m_pos.y = 0.5;
	// m_pos.z = 0.5;

	m_age += delta_time;
}

void InitParticle(device _emitter& emitter, thread _particle& particle)
{
	m_age = 0.0;

	_float3 emitter_pos = _float3( emitter.pom[3][0], emitter.pom[3][1], emitter.pom[3][2]);
	_float3 emitter_focus_dir = _float3( emitter.pom[0][0], emitter.pom[0][1], emitter.pom[0][2]);
	_float3 emitter_aperture_dirx = _float3( emitter.pom[1][0], emitter.pom[1][1], emitter.pom[1][2]);
	_float3 emitter_aperture_diry = _float3( emitter.pom[2][0], emitter.pom[2][1], emitter.pom[2][2]);

	_float3 random = _float3(m_phase.xyz) * _float3( 2.0) - _float3( 1.0);
	_float3 aperture_offset;

	aperture_offset =
		emitter_aperture_dirx * random.y * emitter.aperture.x +
		emitter_aperture_diry * random.z * emitter.aperture.y +
		emitter_focus_dir * random.z * emitter.aperture.z;

	m_pos = hfloat3(emitter_pos + aperture_offset);

	_float4 side = _float4( emitter_aperture_dirx, 0.0);

	_float4 focus_offset;

	focus_offset.xyz = emitter_pos + emitter_focus_dir * emitter.Focus_Life_Rate_Gravity.x;

	if( emitter.Focus_Life_Rate_Gravity.x > 0.0)
	{
		m_n = hfloat3(focus_offset.xyz) - m_pos;
	}
	else
	{
		m_n = m_pos - hfloat3(focus_offset.xyz);
	}

	m_n = normalize( m_n);

	m_b = cross( m_n, hfloat3(side.xyz));
	m_b = normalize( m_b);

	m_t = cross( m_b, m_n);
	m_t = normalize( m_t);

	m_speed = mix( emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.x, emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.y,  m_phase.x);
	m_acceleration = mix( emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.z, emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.w,  m_phase.x);

	m_freq = mix(emitter.FreqMin, emitter.FreqMax, m_phase.x);
	m_amplitude = mix(emitter.AmpMin.xyz, emitter.AmpMax.xyz, m_phase.x);

	m_is_active = 1.0;
}

void particle_simulate(	uint thread_position_in_grid,
	device _emitter& emitter,
	thread _particle& particle
#ifdef SINGLE_FRAME_MODE
	, hfloat delta_time
#endif
	)
{
	if(m_is_active <= 0.5)
		return;

#ifndef SINGLE_FRAME_MODE
	_float delta_time = 1.0 / 40.0;
#endif

	_float predicted_age = m_age + delta_time * _float(  emitter.EmitCount_LastParticleIndex_NumSubSteps.z);

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
			CalculateParticle(emitter, particle, _float3(emitter.external_velocity.xyz), emitter.Focus_Life_Rate_Gravity.w, delta_time);
		}
	}
}

void particle_emit(
	uint thread_position_in_grid,
	device _emitter& emitter,
	thread _particle& particle
	)
{
	InitParticle(emitter, particle);
}

kernel void shader_main(
	uint thread_position_in_grid [[thread_position_in_grid]]
	,device _emitter* emitters [[buffer(1)]]
	,device _particle* particles [[buffer(0)]]
#if defined(SINGLE_FRAME_MODE) && defined(PARTICLE_SIMULATE)
	,constant hfloat* delta_time_buffer [[ buffer(2) ]]
#endif
	)
{
#ifdef PARTICLE_EMIT
	if(thread_position_in_grid >= emitters->EmitCount_LastParticleIndex_NumSubSteps.x)
		return;

	auto particleIdx = (emitters->EmitCount_LastParticleIndex_NumSubSteps.y + thread_position_in_grid) % MAX_PARTICLES;
#else
	auto particleIdx = thread_position_in_grid;
#endif

	auto localParticle = particles[particleIdx];

#ifdef PARTICLE_SIMULATE

#ifndef SINGLE_FRAME_MODE
	particle_simulate(thread_position_in_grid, *emitters, localParticle);
#else
	particle_simulate(thread_position_in_grid, *emitters, localParticle, delta_time_buffer[0]);
#endif
	
#else
	particle_emit(thread_position_in_grid, *emitters, localParticle);
#endif

	particles[particleIdx] = localParticle;
}

#endif