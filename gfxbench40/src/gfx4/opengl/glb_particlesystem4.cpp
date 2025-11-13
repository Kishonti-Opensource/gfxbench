/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_particlesystem4.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_state_manager.h"
#include <cstddef>

#include "kcl_scene_handler.h"
#include "ng/stringutil.h"
#include "ng/log.h"

static int seed = 0;

const std::string EMIT_WARMUP_FRAME     = "particle_buffers_32360ms_emitter0_emit_warmup";
const std::string SIMULATE_WARMUP_FRAME = "particle_buffers_48560ms_emitter0_simulation_warmup";


static float rand_unsigned_clamped()
{
	return KCL::Math::randomf( &seed);
}
static float rand_signed_clamped()
{
	return KCL::Math::randomf_signed( &seed);
}




GLB::ComputeEmitter::ComputeEmitter( KCL::uint32 num_max_particles, bool single_frame)  : KCL::Node("", KCL::EMITTER4, NULL, NULL)
, m_num_max_particles( num_max_particles)
, m_particles_vbo( 0)
, m_emitter_ubo( 0)
, m_emit_count( 0.0f)
, m_simulation_count( 0.0f)
, m_single_frame( single_frame )
, m_emit_wg_size( 128 )
, m_simulate_wg_size( 128 )
, m_temp_particles_vbo( 0 )
{
	_particle min_;
	_particle max_;
	KCL::Matrix4x4 pom;

	m_particles_vao[0] = 0;
	m_particles_vao[1] = 0;

	m_particles_ebo[0] = 0;
	m_particles_ebo[1] = 0;

	m_emitter.EmitCount_LastParticleIndex_NumSubSteps[1] = 0;
	min_.Frequency.set( 0.0f, 0.0f, 0.0f, 0.0f);
	min_.Amplitude.set( 0.0f, 0.0f, 0.0f, 0.0f);
	min_.Age_Speed_Accel_Active.set( 0.0f, 0.0f, 0.0f, 0.0f);
	max_ = min_;
	pom.identity();

	glGenBuffers( 1, &m_emitter_ubo);

	glBindBuffer( GL_ARRAY_BUFFER, m_emitter_ubo);
	glBufferData( GL_ARRAY_BUFFER, sizeof(_emitter), &m_emitter, GL_STATIC_DRAW);
	glBindBuffer( GL_ARRAY_BUFFER, 0);

	//default parameters
	SetAperture( 0.0f, 0.0f, 0.0f);
	SetExternalVelocity( 0.0f, 0.0f, 0.0f);
	SetFocusDistance( 1024.0f);
	SetEmitRate( 1.0f);
	SetGravity( 0.0f);
	SetPom( pom);
	SetParticleLifetime( 1.0f);
	SetParticleRange( min_, max_);
	m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.x = 1.0;
	m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.y = 1.0;
	m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.z = 0.0;
	m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax.w = 0.0;

	m_emitter.FreqMin.set(1.0, 1.0, 1.0, 0.0);
	m_emitter.FreqMax.set(1.0, 1.0, 1.0, 0.0);
	m_emitter.AmpMin.set(0.0, 0.0, 0.0, 0.0);
	m_emitter.AmpMax.set(0.0, 0.0, 0.0, 0.0);

	if (m_single_frame)
	{
		glGenBuffers(1, &m_temp_particles_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_temp_particles_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_num_max_particles * sizeof(_particle), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_last_particle_age = 0.0f;
}

KCL::KCL_Status GLB::ComputeEmitter::Init(WarmUpHelper *warm_up)
{
	// Load the shaders
	KCL::KCL_Status status = KCL::KCL_TESTERROR_NOERROR;

	if (warm_up)
	{
		//
		//	Save state before warmup
		//

		_emitter emitter_backup = m_emitter;
		_emitter single_frame_emitter_backup = m_single_frame_emitter;
		_particle* particle_data_backup = new _particle[m_num_max_particles];
		float last_particle_age_backup = m_last_particle_age;

		{
			glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
			_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), GL_MAP_READ_BIT);
			memcpy(particle_data_backup, particles, m_num_max_particles*sizeof(_particle));
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		//
		//	Warmup
		//
		WarmUpHelper::WarmUpConfig *cfg = warm_up->GetConfig(WarmUpHelper::PARTICLE_SYSTEM_EMIT);
		if (cfg)
		{
			NGLOG_INFO("ParticleSystem4 %s: using pre-defined workgroup size: %s for emit shader", m_name.c_str(), cfg->m_wg_config.size_x);
			status = LoadEmitShader(cfg->m_wg_config.size_x);
		}
		else
		{
			status = WarmupEmitShader(warm_up);
		}
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			return status;
		}

		cfg = warm_up->GetConfig(WarmUpHelper::PARTICLE_SYSTEM_SIMULATE);
		if (cfg)
		{
			NGLOG_INFO("ParticleSystem4 %s: using pre-defined workgroup size: %s for simulate shader", m_name.c_str(), cfg->m_wg_config.size_x);
			status = LoadSimulateShader(cfg->m_wg_config.size_x);
		}
		else
		{
			status = WarmupSimulateShader(warm_up);
		}

		//
		// Restore internal state
		//
		{
			m_emitter = emitter_backup ;
			m_single_frame_emitter = single_frame_emitter_backup;
			m_last_particle_age = last_particle_age_backup;

			glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
			_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), GL_MAP_WRITE_BIT);
			memcpy(particles, particle_data_backup, m_num_max_particles*sizeof(_particle));
			glUnmapBuffer(GL_ARRAY_BUFFER);

			delete[] particle_data_backup;
		}
	}
	else
	{
		status = LoadEmitShader(m_emit_wg_size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			return status;
		}

		status = LoadSimulateShader(m_simulate_wg_size);
	}

	InitStat();

	return status;

}


GLB::ComputeEmitter::~ComputeEmitter()
{
	if( m_particles_vbo)
	{
		glDeleteBuffers( 1, &m_particles_vbo);
	}
	if( m_emitter_ubo)
	{
		glDeleteBuffers( 1, &m_emitter_ubo);
	}
	if( m_particles_ebo[0])
	{
		glDeleteBuffers( 2, m_particles_ebo);
	}
	if( m_particles_vao[0])
	{
		glDeleteVertexArrays( 2, m_particles_vao);
	}
	if (m_temp_particles_vbo)
	{
		glDeleteBuffers(1, &m_temp_particles_vbo);
}

	DumpStat();
}


void GLB::ComputeEmitter::SetAperture( float x, float y, float z)
{
	m_emitter.aperture.set( x, y, z, 0.0f);
}


void GLB::ComputeEmitter::SetExternalVelocity( float x, float y, float z)
{
	m_emitter.external_velocity.set( x, y, z, 0.0f);
}


void GLB::ComputeEmitter::SetFocusDistance( float f)
{
	m_emitter.Focus_Life_Rate_Gravity.x = f;
}


void GLB::ComputeEmitter::SetEmitRate( float f)
{
	m_emitter.Focus_Life_Rate_Gravity.z = f;
}


void GLB::ComputeEmitter::SetGravity( float f)
{
	m_emitter.Focus_Life_Rate_Gravity.w = f;
}


void GLB::ComputeEmitter::SetPom( KCL::Matrix4x4 &pom)
{
	m_emitter.pom = pom;
}


void GLB::ComputeEmitter::AdjustToDisplace4( KCL::Matrix4x4 &pom, float emit_factor)
{
	KCL::Vector3D interpolated_pos;

	m_prev_pom = m_current_pom;
	m_current_pom = pom;

	m_movement.x = m_current_pom.v[12] - m_prev_pom.v[12];
	m_movement.y = m_current_pom.v[13] - m_prev_pom.v[13];
	m_movement.z = m_current_pom.v[14] - m_prev_pom.v[14];

	interpolated_pos.x = (m_current_pom.v[12] + m_prev_pom.v[12]) * 0.5f;
	interpolated_pos.y = (m_current_pom.v[13] + m_prev_pom.v[13]) * 0.5f;
	interpolated_pos.z = (m_current_pom.v[14] + m_prev_pom.v[14]) * 0.5f;

	KCL::Matrix4x4 interpolated_pom = m_current_pom;

	interpolated_pom.v[12] = interpolated_pos.x;
	interpolated_pom.v[13] = interpolated_pos.y;
	interpolated_pom.v[14] = interpolated_pos.z;

	float d = m_movement.length();

	SetAperture( 0, d * 0.5f, 0.5);

	SetPom( interpolated_pom);

	float emit_rate = 50 * emit_factor * (1.0f + d);

	if( emit_rate > 200.0f)
	{
		emit_rate = 200.0f;
	}

	SetEmitRate( emit_rate);
}


void GLB::ComputeEmitter::SetParticleLifetime( float f)
{
	m_emitter.Focus_Life_Rate_Gravity.y = f;

	m_last_particle_age = f;
}


void GLB::ComputeEmitter::SetParticleRange( _particle &min_, _particle &max_)
{
	//set new values + vbo overwriting

	_particle *p = new _particle[m_num_max_particles];

	for( KCL::uint32 i=0; i<m_num_max_particles; i++)
	{
		// the rand_unsigned_clamped method has side effect! increase a static seed.
		float rand_z = rand_unsigned_clamped();
		float rand_y = rand_unsigned_clamped();
		float rand_x = rand_unsigned_clamped();

		p[i].Phase.set(rand_x, rand_y, rand_z, 0.0f);

		p[i].Pos.set( 0, -10000, 0, 1);
		p[i].Age_Speed_Accel_Active.w = 0.0f;
	}

	if( m_particles_ebo[0])
	{
		glDeleteBuffers( 2, m_particles_ebo);
	}

	if( m_particles_vbo)
	{
		glDeleteBuffers( 1, &m_particles_vbo);
	}

	if( m_particles_vao[0])
	{
		glDeleteVertexArrays( 2, m_particles_vao);
	}

	glGenBuffers( 1, &m_particles_vbo);

	glBindBuffer( GL_ARRAY_BUFFER, m_particles_vbo);
	glBufferData( GL_ARRAY_BUFFER, m_num_max_particles * sizeof(_particle), p, GL_STATIC_DRAW);
	glBindBuffer( GL_ARRAY_BUFFER, 0);

	delete [] p;

	glGenVertexArrays( 2, m_particles_vao);

	glGenBuffers( 2, m_particles_ebo);

	for( int j=0; j<2; j++)
	{
		KCL::uint16 *indices = new KCL::uint16[m_num_max_particles];

		for( KCL::uint32 i=0; i<m_num_max_particles; i++)
		{
			indices[i] = (j == 0) ? i : m_num_max_particles - i - 1;
		}

		glBindVertexArray( m_particles_vao[j]);

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_particles_ebo[j]);
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_num_max_particles * sizeof(KCL::uint16), indices, GL_STATIC_DRAW);

		delete [] indices;

		glBindBuffer( GL_ARRAY_BUFFER, m_particles_vbo);
		glEnableVertexAttribArray( 0);
		glEnableVertexAttribArray( 1);
		glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof( _particle), (void*)offsetof( _particle, Pos));
		glVertexAttribPointer( 1, 2, GL_FLOAT, 0, sizeof( _particle), (void*)offsetof( _particle, Age_Speed_Accel_Active));

		glBindVertexArray( 0);
		glBindBuffer( GL_ARRAY_BUFFER, 0);
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}


void GLB::ComputeEmitter::Animate(float timestep, KCL::uint32 anim_time)
{
	//simulation fps, has to be same with the shader
	const float m_simulation_rate = 40.0f;

	//we can't simulate less than one unit
	//simulate while unit, residue carry on
	m_emit_count += m_emitter.Focus_Life_Rate_Gravity.z * timestep;
	m_simulation_count += m_simulation_rate * timestep;

	m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] = (KCL::int32)m_emit_count;
	m_emitter.EmitCount_LastParticleIndex_NumSubSteps[1] += m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0];
	m_emitter.EmitCount_LastParticleIndex_NumSubSteps[1] = m_emitter.EmitCount_LastParticleIndex_NumSubSteps[1] % m_num_max_particles;
	m_emitter.EmitCount_LastParticleIndex_NumSubSteps[2] = (KCL::int32)m_simulation_count;

	if( m_emit_count >= 1.0f)
	{
		m_emit_count -= int( m_emit_count);
	}

	if( m_simulation_count >= 1.0f)
	{
		m_simulation_count -= int( m_simulation_count);
	}

	if (m_single_frame)
	{
		// restore the single frame emitter
		m_emitter = m_single_frame_emitter;
	}

	CollectStat(anim_time);

	//TODO: Only update the changes (mostly EmitCount_LastParticleIndex_NumSubSteps + pom)
	glBindBuffer( GL_UNIFORM_BUFFER, m_emitter_ubo);
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(_emitter), &m_emitter);
	glBindBuffer( GL_UNIFORM_BUFFER, 0);
}


bool GLB::ComputeEmitter::Emit()
{
	if( NeedToEmit())
	{
		// emit to a temporary buffer in single frame mode
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, (m_single_frame)?m_temp_particles_vbo:m_particles_vbo);
		glBindBufferBase( GL_UNIFORM_BUFFER, 1, m_emitter_ubo);

		OpenGLStateManager::GlUseProgram( m_particle_emit_shader->m_p);
		glDispatchComputeProc( (m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] + m_emit_wg_size - 1) / m_emit_wg_size, 1, 1);

		if ( !m_single_frame)
		{
			m_last_particle_age = 0.0f;
		}

		return true;
	}

	return false;
}


bool GLB::ComputeEmitter::Simulate()
{
	if ( NeedToSimulate())
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particles_vbo); // simulation with the original buffer with 0 delta time in single frame
		glBindBufferBase( GL_UNIFORM_BUFFER, 1, m_emitter_ubo);

		OpenGLStateManager::GlUseProgram( m_particle_simulate_shader->m_p);
		glDispatchComputeProc( (m_num_max_particles + m_simulate_wg_size -1)/m_simulate_wg_size, 1, 1);

		if ( !m_single_frame)
		{
			m_last_particle_age += m_emitter.EmitCount_LastParticleIndex_NumSubSteps[2] * 1.0f / 40.0f;
		}

		return true;
	}

	return false;
}


void GLB::ComputeEmitter::Render(GLBShader2 *billboard_point_gs, const KCL::Matrix4x4 &mvp, const KCL::Matrix4x4 &view_matrix, const KCL::Vector3D &eye, KCL::Texture *textures[2], KCL::uint32 topdown_sampler, KCL::Vector3D &eye_forward)
{
	if ( m_last_particle_age >= m_emitter.Focus_Life_Rate_Gravity.y)
	{
		// Every particles are inactive
		return;
	}

	OpenGLStateManager::GlEnable( GL_BLEND);
	OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
	OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
	GLBShader2 *s;

	s = billboard_point_gs;

	if (m_single_frame)
	{
		m_movement = m_single_frame_movement;
	}

	bool o = KCL::Vector3D::dot( m_movement, eye_forward) > 0.0f;

	OpenGLStateManager::GlDepthMask( 0);
	OpenGLStateManager::GlUseProgram( s->m_p);

	glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::mvp], 1, 0, mvp.v);
	glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::view], 1, 0, view_matrix.v);

	textures[0]->bind( 0);
	glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
	textures[1]->bind( 1);
	glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_unit7], 1);
	glBindSampler(1, topdown_sampler);

	glBindVertexArray( m_particles_vao[o]);

	OpenGLStateManager::Commit();

	glDrawElements( GL_POINTS, m_num_max_particles, GL_UNSIGNED_SHORT, 0);

	glBindSampler(1, 0);

	OpenGLStateManager::GlDepthMask( 1);
	OpenGLStateManager::GlDisable( GL_BLEND);
	OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
}


void GLB::ComputeEmitter::Serialize(JsonSerializer& s)
{
	s.SerializeNVP(m_emitter.Focus_Life_Rate_Gravity);
	s.SerializeNVP(m_emitter.aperture);
	s.SerializeNVP(m_emitter.external_velocity);
	s.SerializeNVP(m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax);
}


KCL::KCL_Status GLB::ComputeEmitter::LoadEmitShader(KCL::uint32 wg_size)
{
	m_emit_wg_size = wg_size;

	GLB::GLBShaderBuilder sb;
	KCL::KCL_Status error;

	// Load emitter shaders
	sb.AddDefine("PARTICLE_EMIT");
	sb.AddDefineInt("MAX_PARTICLES", m_num_max_particles);
	sb.AddDefineInt("WORK_GROUP_WIDTH", wg_size);
	m_particle_emit_shader = sb.ShaderFile("emitter_simulate4.shader").Build(error);

	return error;
}


KCL::KCL_Status GLB::ComputeEmitter::LoadSimulateShader(KCL::uint32 wg_size)
{
	m_simulate_wg_size = wg_size;

	GLB::GLBShaderBuilder sb;
	KCL::KCL_Status error;

	sb.AddDefine("PARTICLE_SIMULATE");
	sb.AddDefineInt("WORK_GROUP_WIDTH", wg_size);
	sb.AddDefineInt("MAX_PARTICLES", m_num_max_particles);

	if (m_single_frame)
	{
		sb.AddDefine("SINGLE_FRAME_MODE");
	}

	m_particle_simulate_shader = sb.ShaderFile("emitter_simulate4.shader").Build(error);

	if (m_single_frame)
	{
		// set 0 delta time for the simulation
		OpenGLStateManager::GlUseProgram(m_particle_simulate_shader->m_p);
		GLint delta_time_location = glGetUniformLocation(m_particle_simulate_shader->m_p, "delta_time");
		assert(delta_time_location != -1);
		glUniform1f(delta_time_location, 0.0f);
		OpenGLStateManager::GlUseProgram(0);
	}

	return error;
}


bool GLB::ComputeEmitter::NeedToEmit() const
{
	return m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] > 0;
}


bool GLB::ComputeEmitter::NeedToSimulate() const
{
	return m_emitter.EmitCount_LastParticleIndex_NumSubSteps[2] > 0; 
}


KCL::KCL_Status GLB::ComputeEmitter::InitEmitWarmUp()
{
	KCL::KCL_Status status = LoadStateByFilename(EMIT_WARMUP_FRAME.c_str());
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		NGLOG_INFO("WARNING! Unable to load buffer state to warmup particle system emit shader!");
		return status;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_emitter_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(_emitter), &m_emitter);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particles_vbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_emitter_ubo);

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status GLB::ComputeEmitter::InitSimulateWarmUp()
{
	KCL::KCL_Status status = LoadStateByFilename(SIMULATE_WARMUP_FRAME.c_str());
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		NGLOG_INFO("WARNING! Unable to load buffer state to warmup particle system simulate shader!");
		return status;
	}

	//
	//	Increase the emitter life attribute thus the particles doesn't die during the warmup
	//
	_emitter warm_up_emitter = m_emitter;
	warm_up_emitter.Focus_Life_Rate_Gravity.y = 10000000.0f;

	glBindBuffer(GL_UNIFORM_BUFFER, m_emitter_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(_emitter), &warm_up_emitter);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_emitter_ubo);

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status GLB::ComputeEmitter::WarmupEmitShader(WarmUpHelper *warm_up)
{
	NGLOG_INFO("Warm up ParticleSystem Emit shader...");
	KCL::KCL_Status status;

	status = InitEmitWarmUp();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256 };
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		KCL::uint32 wg_size = sizes[i];

		if (!warm_up->GetValidator()->Validate(wg_size))
		{
			continue;
		}

		status = LoadEmitShader(wg_size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			continue;
		}

		GLB::OpenGLStateManager::GlUseProgram(m_particle_emit_shader->m_p);

		NGLOG_INFO("Workgroup size: %s", wg_size);
		KCL::uint32 wg_count = (m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] + wg_size - 1) / wg_size;

		// First try with 5 iterations
		KCL::uint32 iterations = 5;
		KCL::uint64 dt = 0;
		double avg_time = 0.0;
		warm_up->BeginTimer();
		for (KCL::uint32 j = 0; j < iterations; j++)
		{
			glDispatchComputeProc(wg_count, 1, 1);
			glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		dt = warm_up->EndTimer();
		avg_time = double(dt) / double(iterations);

		NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

		if (dt < 50)
		{
			// Warm up until 500ms but maximalize the max iteration count
			iterations = avg_time > 0.01 ? KCL::uint32(500.0 / avg_time) : 200;

			iterations = KCL::Max(iterations, 5u);
			iterations = KCL::Min(iterations, 200u);

			NGLOG_INFO("  warmup %s iterations...", iterations);
			warm_up->BeginTimer();
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				glDispatchComputeProc(wg_count, 1, 1);
				glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);
			}
			dt = warm_up->EndTimer();
			avg_time = double(dt) / double(iterations);

			NGLOG_INFO("  result: sum: %sms, avg time: %sms", float(dt), float(avg_time));
		}

		if (avg_time < best_time)
		{
			best_time = avg_time;
			best_wg_size = wg_size;
		}
	}


	NGLOG_INFO("Best result: %s -> %sms (avg)", best_wg_size, float(best_time));
	if (best_wg_size == 0)
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	WarmUpHelper::WarmUpConfig *cfg = new WarmUpHelper::WarmUpConfig();
	cfg->m_wg_config.size_x = best_wg_size;
	warm_up->SetConfig(WarmUpHelper::PARTICLE_SYSTEM_EMIT, cfg);

	return LoadEmitShader(best_wg_size);
}


KCL::KCL_Status GLB::ComputeEmitter::WarmupSimulateShader(WarmUpHelper *warm_up)
{
	NGLOG_INFO("Warm up ParticleSystem Simulate shader...");
	KCL::KCL_Status status;

	status = InitSimulateWarmUp();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	_particle* warmup_data = new _particle[m_num_max_particles];

	//
	//	Save the loaded warmup data
	//
	glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
	_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), GL_MAP_READ_BIT);
	memcpy(warmup_data, particles, m_num_max_particles*sizeof(_particle));
	glUnmapBuffer(GL_ARRAY_BUFFER);


	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256 };
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		// restore the data used for warmup
		glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
		_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), GL_MAP_WRITE_BIT);
		memcpy(particles, warmup_data, m_num_max_particles*sizeof(_particle));
		glUnmapBuffer(GL_ARRAY_BUFFER);

		KCL::uint32 wg_size = sizes[i];
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particles_vbo);

		if (!warm_up->GetValidator()->Validate(wg_size))
		{
			continue;
		}

		status = LoadSimulateShader(wg_size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			continue;
		}

		GLB::OpenGLStateManager::GlUseProgram(m_particle_simulate_shader->m_p);

		NGLOG_INFO("Workgroup size: %s", wg_size);
		KCL::uint32 wg_count = (m_num_max_particles + wg_size - 1) /wg_size;

		// First try with 5 iterations
		KCL::uint32 iterations = 5;
		KCL::uint64 dt = 0;
		double avg_time = 0.0;
		warm_up->BeginTimer();
		for (KCL::uint32 j = 0; j < iterations; j++)
		{
			glDispatchComputeProc(wg_count, 1, 1);
			glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		dt = warm_up->EndTimer();
		avg_time = double(dt) / double(iterations);

		NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

		if (dt < 50)
		{
			// Warm up until 500ms but maximalize the max iteration count
			iterations = avg_time > 0.01 ? KCL::uint32(500.0 / avg_time) : 200;

			iterations = KCL::Max(iterations, 5u);
			iterations = KCL::Min(iterations, 200u);

			NGLOG_INFO("  warmup %s iterations...", iterations);
			warm_up->BeginTimer();
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				glDispatchComputeProc(wg_count, 1, 1);
				glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);
			}
			dt = warm_up->EndTimer();
			avg_time = double(dt) / double(iterations);

			NGLOG_INFO("  result: sum: %sms, avg time: %sms", float(dt), float(avg_time));
		}

		if (avg_time < best_time)
		{
			best_time = avg_time;
			best_wg_size = wg_size;
		}
	}

	delete[] warmup_data;

	NGLOG_INFO("Best result: %s -> %sms (avg)", best_wg_size, float(best_time));
	if (best_wg_size == 0)
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	WarmUpHelper::WarmUpConfig *cfg = new WarmUpHelper::WarmUpConfig();
	cfg->m_wg_config.size_x = best_wg_size;
	warm_up->SetConfig(WarmUpHelper::PARTICLE_SYSTEM_SIMULATE, cfg);

	return LoadSimulateShader(best_wg_size);
}


void GLB::ComputeEmitter::ResetParticleSystemSeed()
{
	seed = 0;
}


//
//	particle system state load save
//

#define BINARY_DUMP 1
#define TEXT_DUMP_FOR_DEBUG 0

#if BINARY_DUMP
template <bool write, typename T>
void PrimitiveStreamOperation(T &data, KCL::File & file)
{
	if (write)
	{
		file.Write(&data, sizeof(T), 1);
	}
	else
	{
		file.Read(&data, sizeof(T), 1);
	}
}

void StreamNewLine(KCL::File & file)
{

}
#endif

#if !BINARY_DUMP || TEXT_DUMP_FOR_DEBUG
template <bool write, typename T>
void PrimitiveStreamOperation(T &data, std::stringstream & sstream)
{
	if (write)
	{
		sstream << data << " ";
	}
	else
	{
		sstream >> data;
	}
}

void StreamNewLine(std::stringstream & sstream)
{
	sstream << std::endl;
}
#endif


template <bool write, typename T, int count, typename W>
void BufferStreamOperation(void* data, W & sstream)
{
	for (unsigned int i = 0; i < count; i++)
	{
		PrimitiveStreamOperation<write>(reinterpret_cast<T*>(data)[i], sstream);
	}
	if (write) StreamNewLine(sstream);
}


template <bool write, typename T>
void BufferStreamOperation(KCL::Matrix4x4 &m, T & sstream)
{
	BufferStreamOperation<write, float, 16>(m.v, sstream);
}


template <bool write, typename T>
void BufferStreamOperation(KCL::Vector4D &v, T & sstream)
{
	BufferStreamOperation<write, float, 4>(v.v, sstream);
}



template <bool write, typename T>
void BufferStreamOperation(KCL::Vector3D &v, T & sstream)
{
	BufferStreamOperation<write, float, 3>(v.v, sstream);
}


template <bool write, typename T>
void GLB::ComputeEmitter::EmitterStreamOperation(T & sstream)
{
	BufferStreamOperation<write, KCL::int32, 4 >(m_emitter.EmitCount_LastParticleIndex_NumSubSteps, sstream);
	BufferStreamOperation<write>(m_emitter.pom, sstream);
	BufferStreamOperation<write>(m_emitter.external_velocity, sstream);
	BufferStreamOperation<write>(m_emitter.aperture, sstream);
	BufferStreamOperation<write>(m_emitter.Focus_Life_Rate_Gravity, sstream);

	BufferStreamOperation<write>(m_emitter.SpeedMin_SpeedMax_AccelMin_AccelMax, sstream);
	BufferStreamOperation<write>(m_emitter.FreqMin, sstream);
	BufferStreamOperation<write>(m_emitter.FreqMax, sstream);
	BufferStreamOperation<write>(m_emitter.AmpMin, sstream);
	BufferStreamOperation<write>(m_emitter.AmpMax, sstream);
	if (write) StreamNewLine(sstream);
}


template <bool write, typename T>
void GLB::ComputeEmitter::ParticleStreamOperation(_particle &particle, T & sstream)
{
	BufferStreamOperation<write>(particle.Pos, sstream);
	BufferStreamOperation<write>(particle.Velocity, sstream);
	BufferStreamOperation<write>(particle.T, sstream);
	BufferStreamOperation<write>(particle.B, sstream);
	BufferStreamOperation<write>(particle.N, sstream);

	BufferStreamOperation<write>(particle.Phase, sstream);
	BufferStreamOperation<write>(particle.Frequency, sstream);
	BufferStreamOperation<write>(particle.Amplitude, sstream);
	BufferStreamOperation<write>(particle.Age_Speed_Accel_Active, sstream);
	if (write) StreamNewLine(sstream);
}


template <bool write, typename T>
void GLB::ComputeEmitter::StateOperation(T & sstream)
{
	EmitterStreamOperation<write>(sstream);
	BufferStreamOperation<write>(m_movement,sstream);
	StreamNewLine(sstream);

	glMemoryBarrierProc(GL_BUFFER_UPDATE_BARRIER_BIT);

	glFinish();
	glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
	_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), (write) ? GL_MAP_READ_BIT : GL_MAP_WRITE_BIT);

	for (unsigned int i = 0; i < m_num_max_particles; i++)
	{
		ParticleStreamOperation<write>(particles[i], sstream);
	}

	if (!write)
	{
		// Select the youngest particle
		m_last_particle_age = m_emitter.Focus_Life_Rate_Gravity.y;

		for (KCL::uint32 i = 0; i < m_num_max_particles; i++)
		{
			if (particles[i].Age_Speed_Accel_Active.w > 0.5f && particles[i].Age_Speed_Accel_Active.x < m_last_particle_age)
			{
				m_last_particle_age = particles[i].Age_Speed_Accel_Active.x;
			}
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
}

KCL::KCL_Status GLB::ComputeEmitter::LoadStateByFilename(const char * filename)
{
	KCL::AssetFile particle_file(filename);

	if (particle_file.GetLastError() == KCL::KCL_IO_NO_ERROR)
	{
#if BINARY_DUMP
		StateOperation<false>(particle_file);
#else
		std::stringstream sstream(particle_file.GetBuffer());
		StateOperation<false>(sstream);
#endif
	}
	else
	{
		NGLOG_INFO("WARNING! Particle system buffer state not found: %s!!", filename);
		return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
	}

	m_single_frame_emitter = m_emitter;
	m_single_frame_movement = m_movement;
	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status GLB::ComputeEmitter::LoadState(KCL::uint32 anim_time)
{
	std::string filename = PARTICLE_BUFFERS_FILENAME(anim_time, m_name);

	return LoadStateByFilename(filename.c_str());
}


void GLB::ComputeEmitter::SaveState(KCL::uint32 anim_time)
{
	KCL::File particle_file(PARTICLE_BUFFERS_FILENAME(anim_time,m_name), KCL::Write, KCL::RWDir);

#if BINARY_DUMP
	StateOperation<true>(particle_file);
#else
	std::stringstream sstream;
	StateOperation<true>(sstream);
	sstream.precision(20);
	particle_file.Write(sstream.str());
#endif

	particle_file.Close();

#if TEXT_DUMP_FOR_DEBUG
	KCL::File particle_file_debug(PARTICLE_BUFFERS_FILENAME(anim_time, m_name) + ".txt", KCL::Write, KCL::RWDir);

	std::stringstream sstream_debug;
	sstream_debug.precision(7);
	sstream_debug << std::fixed;
	StateOperation<true>(sstream_debug);
	particle_file_debug.Write(sstream_debug.str());

	particle_file_debug.Close();
#endif
}


#if ENABLE_PARTICLE_SYSTEM_STAT

#include <stdint.h>
#include <float.h>

void GLB::ComputeEmitter::InitStat()
{
	m_min_emit_count = INT32_MAX;
	m_max_emit_count = INT32_MIN;
	m_sum_emit_count = 0;

	m_min_simulation_count = INT32_MAX;
	m_max_simulation_count = INT32_MIN;
	m_sum_simulation_count = 0;

	m_stat_collect_count = 0;
}

template <typename T>
void minmaxsum(KCL::int32 &v, KCL::int32 &min, KCL::int32 &max, KCL::uint64 &sum, T &data, T &min_data, T &max_data)
{
	if (v < min)
	{
		min = v;
		min_data = data;
	}
	if (v > max)
	{
		max = v;
		max_data = data;
	}
	sum += v;
}

void GLB::ComputeEmitter::CollectStat(KCL::uint32 anim_time)
{
	KCL::int32 actual_emit_count = m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0];

	m_emit_per_frame[anim_time] = actual_emit_count;
	minmaxsum(actual_emit_count, m_min_emit_count, m_max_emit_count, m_sum_emit_count, anim_time, m_min_emit_frame, m_max_emit_frame);

	KCL::int32 actual_simulation_count = 0;

	glFinish();
	glBindBuffer(GL_ARRAY_BUFFER, m_particles_vbo);
	_particle* particles = (_particle*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_num_max_particles*sizeof(_particle), GL_MAP_READ_BIT);

	for (unsigned int i = 0; i < m_num_max_particles; i++)
	{
		if (particles[i].Age_Speed_Accel_Active.w > 0.5f)
		{
			actual_simulation_count++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	m_simulation_per_frame[anim_time] = actual_simulation_count;
	minmaxsum(actual_simulation_count, m_min_simulation_count, m_max_simulation_count, m_sum_simulation_count,
		anim_time, m_min_simulation_frame, m_max_simulation_frame);

	m_stat_collect_count++;
}


KCL::uint32 GetMinDifValue(std::map<KCL::uint32, KCL::int32> data, double v)
{
	KCL::uint32 min_id = 0;
	double min_diff = DBL_MAX;

	for (std::map<KCL::uint32, KCL::int32>::iterator it = data.begin(); it != data.end(); it++)
	{
		double d = static_cast<double>(it->second);
		double diff = abs(d - v);
		if (diff < min_diff)
		{
			min_diff = diff;
			min_id = it->first;
		}
	}
	return min_id;
}


void GLB::ComputeEmitter::DumpStat()
{
	double avg_emit_count = static_cast<double>(m_sum_emit_count) / m_stat_collect_count;
	double avg_simulation_count = static_cast<double>(m_sum_simulation_count) / m_stat_collect_count;

	KCL::uint32 avg_emit_frame = GetMinDifValue(m_emit_per_frame, avg_emit_count);
	KCL::uint32 avg_simulation_frame = GetMinDifValue(m_simulation_per_frame, avg_simulation_count);

	INFO("");
	INFO("Emitter:        %s", m_name.c_str());
	//INFO("min emit count: %d", m_min_emit_count);
	INFO("max emit count: %d", m_max_emit_count);
	INFO("max emit frame: %d", m_max_emit_frame);
	INFO("avg emit count: %f", avg_emit_count);
	INFO("avg emit frame: %d", avg_emit_frame);
	//INFO("min simulation count: %d", m_min_simulation_count);
	INFO("max simulation count: %d", m_max_simulation_count);
	INFO("max simulation frame: %d", m_max_simulation_frame);
	INFO("avg simulation count: %f", avg_simulation_count);
	INFO("avg simulation frame: %d", avg_simulation_frame);
	INFO("");
}

#endif

