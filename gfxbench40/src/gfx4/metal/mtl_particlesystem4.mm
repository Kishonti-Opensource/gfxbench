/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_particlesystem4.h"
#include "kcl_node.h"
#include "mtl_pipeline.h"
#include "metal/mtl_pipeline_builder.h"
#include "metal/mtl_dynamic_data_buffer.h"
#include "mtl_scene_40.h"

#include "ng/log.h"


static int seed = 0;

const std::string EMIT_WARMUP_FRAME     = "car_chase/particle_buffers_32360ms_emitter0_emit_warmup";
const std::string SIMULATE_WARMUP_FRAME = "car_chase/particle_buffers_48560ms_emitter0_simulation_warmup";


static float rand_unsigned_clamped()
{
    return KCL::Math::randomf( &seed);
}
static float rand_signed_clamped()
{
    return KCL::Math::randomf_signed( &seed);
}

using namespace MetalRender;

ComputeEmitter::ComputeEmitter( KCL::uint32 num_max_particles, bool single_frame, id<MTLDevice> device)
:   KCL::Node("", KCL::EMITTER4, NULL, NULL)
    , m_device(device)
    , m_num_max_particles( num_max_particles)
    , m_emit_count( 0.0f)
    , m_simulation_count( 0.0f)
    , m_emit_wg_size( 32 )
    , m_simulate_wg_size( 32 )
    , m_single_frame( single_frame )
{
    _particle min_;
    _particle max_;
    KCL::Matrix4x4 pom;
    
    m_emitter.EmitCount_LastParticleIndex_NumSubSteps[1] = 0;
    min_.Frequency.set( 0.0f, 0.0f, 0.0f, 0.0f);
    min_.Amplitude.set( 0.0f, 0.0f, 0.0f, 0.0f);
    min_.Age_Speed_Accel_Active.set( 0.0f, 0.0f, 0.0f, 0.0f);
    max_ = min_;
    pom.identity();

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
		m_temp_particle_vertices_buffer = [m_device newBufferWithLength:sizeof(m_num_max_particles) * sizeof(_particle) options:MTLResourceStorageModePrivate];
    }
    
    m_last_particle_age = 0.0f;
}

ComputeEmitter::~ComputeEmitter()
{
    
}


KCL::KCL_Status ComputeEmitter::Init(WarmUpHelper *warm_up, DynamicDataBufferPool *pool)
{
    // Load the shaders
    KCL::KCL_Status status = KCL::KCL_TESTERROR_NOERROR;
    
    if (warm_up)
    {
        //
        // Save state before warmup
        //
        
        _emitter emitter_backup = m_emitter;
        _emitter single_frame_emitter_backup = m_single_frame_emitter;
        _particle* particle_data_backup = new _particle[m_num_max_particles];
        float last_particle_age_backup = m_last_particle_age;

		// Test single frame particle mode
		_particle* particles = (_particle*)[m_particle_vertices_buffer contents];
		memcpy(particle_data_backup, particles, m_num_max_particles * sizeof(_particle));

        //
        // Warmup
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
            
			_particle* particles = (_particle*)[m_particle_vertices_buffer contents];
			memcpy(particles, particle_data_backup, m_num_max_particles * sizeof(_particle));

#if !TARGET_OS_EMBEDDED
			[m_particle_vertices_buffer didModifyRange:NSMakeRange(0, m_num_max_particles * sizeof(_particle))];
#endif

            delete[] particle_data_backup;
        }
    }
    else
    {
        m_particle_dynamic_buffer = pool->GetNewBuffer(sizeof(_emitter));

        status = LoadEmitShader(m_emit_wg_size);
        if (status != KCL::KCL_TESTERROR_NOERROR)
        {
            return status;
        }
        
        status = LoadSimulateShader(m_simulate_wg_size);
    }
    
//    InitStat();
    
    return status;
    
}


void ComputeEmitter::SetAperture( float x, float y, float z)
{
    m_emitter.aperture.set( x, y, z, 0.0f);
}


void ComputeEmitter::SetExternalVelocity( float x, float y, float z)
{
    m_emitter.external_velocity.set( x, y, z, 0.0f);
}


void ComputeEmitter::SetFocusDistance( float f)
{
    m_emitter.Focus_Life_Rate_Gravity.x = f;
}


void ComputeEmitter::SetEmitRate( float f)
{
    m_emitter.Focus_Life_Rate_Gravity.z = f;
}


void ComputeEmitter::SetGravity( float f)
{
    m_emitter.Focus_Life_Rate_Gravity.w = f;
}


void ComputeEmitter::SetPom( KCL::Matrix4x4 &pom)
{
    m_emitter.pom = pom;
}

bool ComputeEmitter::NeedToEmit() const
{
    return m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] > 0;
}


bool ComputeEmitter::NeedToSimulate() const
{
    return m_emitter.EmitCount_LastParticleIndex_NumSubSteps[2] > 0;
}

void ComputeEmitter::AdjustToDisplace4( KCL::Matrix4x4 &pom, float emit_factor)
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

void ComputeEmitter::SetParticleLifetime( float f)
{
    m_emitter.Focus_Life_Rate_Gravity.y = f;
    
    m_last_particle_age = f;
}

void ComputeEmitter::SetParticleRange( _particle &min_, _particle &max_)
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

	m_particle_vertices_buffer = [m_device newBufferWithBytes:p length:m_num_max_particles * sizeof(_particle) options:STORAGE_MODE_MANAGED_OR_SHARED];
    
    delete [] p;

    for( int j=0; j<2; j++)
    {
        KCL::uint16 *indices = new KCL::uint16[m_num_max_particles];
        
        for( KCL::uint32 i=0; i<m_num_max_particles; i++)
        {
            indices[i] = (j == 0) ? i : m_num_max_particles - i - 1;
        }

        m_particle_indices_buffer[j] = [m_device newBufferWithBytes:indices length:sizeof(KCL::uint16) * m_num_max_particles options:STORAGE_MODE_MANAGED_OR_SHARED];

        delete [] indices;
    }
}



void ComputeEmitter::Animate(float timestep, KCL::uint32 anim_time)
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
    
//    CollectStat(anim_time);
    
    //TODO: Only update the changes (mostly EmitCount_LastParticleIndex_NumSubSteps + pom)
    m_particle_dynamic_buffer->WriteDataAndGetOffset(nil, &m_emitter, sizeof(_emitter));
}


bool ComputeEmitter::Emit(id<MTLCommandBuffer> command_buffer)
{
    if(NeedToEmit())
    {
        // emit to a temporary buffer in single frame mode
        auto computeEncoder = [command_buffer computeCommandEncoder];
        m_particle_emit_pipeline->SetAsCompute(computeEncoder);
        
		[computeEncoder setBuffer:m_single_frame ? m_temp_particle_vertices_buffer : m_particle_vertices_buffer offset:0 atIndex:0];
        [computeEncoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];
        
        auto workgroupSize = MTLSizeMake(m_emit_wg_size, 1, 1);
        auto threadGroups = MTLSizeMake((m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] + m_emit_wg_size - 1) / m_emit_wg_size, 1, 1);
        [computeEncoder dispatchThreadgroups:threadGroups threadsPerThreadgroup:workgroupSize];
        [computeEncoder endEncoding];

        if ( !m_single_frame)
        {
            m_last_particle_age = 0.0f;
        }
        
        return true;
    }
    
    return false;
}


bool ComputeEmitter::Simulate(id<MTLCommandBuffer> command_buffer)
{
    if (NeedToSimulate())
    {
        auto computeEncoder = [command_buffer computeCommandEncoder];
        m_particle_simulate_pipeline->SetAsCompute(computeEncoder);
        
		[computeEncoder setBuffer:(m_single_frame?m_temp_particle_vertices_buffer:m_particle_vertices_buffer) offset:0 atIndex:0];
        [computeEncoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];
        
		if (m_single_frame)
		{
			// set 0 delta time for the simulation
			float zero = 0.0f;
			[computeEncoder setBytes:&zero length:sizeof(zero) atIndex:2];
		}
		
        auto workgroupSize = MTLSizeMake(m_simulate_wg_size, 1, 1);
        auto threadGroups = MTLSizeMake((m_num_max_particles + m_simulate_wg_size - 1) / m_simulate_wg_size, 1, 1);
        [computeEncoder dispatchThreadgroups:threadGroups threadsPerThreadgroup:workgroupSize];
        [computeEncoder endEncoding];

        if ( !m_single_frame)
        {
            m_last_particle_age += m_emitter.EmitCount_LastParticleIndex_NumSubSteps[2] * 1.0f / 40.0f;
        }
        
        return true;
    }
    
    return false;
}


void ComputeEmitter::Render(id<MTLRenderCommandEncoder> encoder, Pipeline* billboard_point_gs, const KCL::Matrix4x4 &mvp, const KCL::Matrix4x4 &view_matrix, const KCL::Vector3D &eye, Texture *textures[2], id<MTLSamplerState> topdown_sampler, KCL::Vector3D &eye_forward)
{
    if ( m_last_particle_age >= m_emitter.Focus_Life_Rate_Gravity.y)
    {
        // Every particles are inactive
        return;
    }

    if (m_single_frame)
    {
        m_movement = m_single_frame_movement;
    }
    
    bool o = KCL::Vector3D::dot( m_movement, eye_forward) > 0.0f;

    billboard_point_gs->Set(encoder);
    
    [encoder setVertexBuffer:m_particle_vertices_buffer offset:0 atIndex:0];
    [encoder setVertexBuffer:m_particle_indices_buffer[o] offset:0 atIndex:1];
    [encoder setVertexBytes:&mvp length:sizeof(mvp) atIndex:2];
    [encoder setVertexBytes:&view_matrix length:sizeof(view_matrix) atIndex:3];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4 instanceCount:m_num_max_particles];
}


KCL::KCL_Status ComputeEmitter::LoadEmitShader(KCL::uint32 wg_size)
{
    m_emit_wg_size = wg_size;
    
    MTLPipeLineBuilder sb;
    KCL::KCL_Status error;
    
    // Load emitter shaders
    sb.AddDefine("PARTICLE_EMIT");
    sb.AddDefineInt("MAX_PARTICLES", m_num_max_particles);
    sb.AddDefineInt("WORK_GROUP_WIDTH", wg_size);
    
    m_particle_emit_pipeline = sb.ShaderFile("emitter_simulate4.shader").Build(error);
    
    return error;
}


KCL::KCL_Status ComputeEmitter::LoadSimulateShader(KCL::uint32 wg_size)
{
    m_simulate_wg_size = wg_size;
    
    MTLPipeLineBuilder sb;
    KCL::KCL_Status error;
    
    sb.AddDefine("PARTICLE_SIMULATE");
    sb.AddDefineInt("WORK_GROUP_WIDTH", wg_size);
    sb.AddDefineInt("MAX_PARTICLES", m_num_max_particles);
    
    if (m_single_frame)
    {
        sb.AddDefine("SINGLE_FRAME_MODE");
    }
    
    m_particle_simulate_pipeline = sb.ShaderFile("emitter_simulate4.shader").Build(error);
    
    return error;
}


KCL::KCL_Status ComputeEmitter::InitEmitWarmUp()
{
	KCL::KCL_Status status = LoadStateByFilename(EMIT_WARMUP_FRAME.c_str());
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		NGLOG_INFO("WARNING! Unable to load buffer state to warmup particle system emit shader!");
		return status;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status ComputeEmitter::InitSimulateWarmUp()
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

	return KCL::KCL_TESTERROR_NOERROR;
}


KCL::KCL_Status ComputeEmitter::WarmupEmitShader(WarmUpHelper *warm_up)
{
	NGLOG_INFO("Warm up ParticleSystem Emit shader...");
	KCL::KCL_Status status;

	MTL_Scene_40 * scene = warm_up->GetScene();

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

		if (wg_size > m_particle_emit_pipeline->GetMaxThreadCount())
		{
			continue;
		}

		id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
		id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

		m_particle_emit_pipeline->SetAsCompute(encoder);
		[encoder setBuffer:m_particle_vertices_buffer offset:0 atIndex:0];
		[encoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];

		NGLOG_INFO("Workgroup size: %s", wg_size);
		KCL::uint32 wg_count = (m_emitter.EmitCount_LastParticleIndex_NumSubSteps[0] + wg_size - 1) / wg_size;

		// First try with 5 iterations
		KCL::uint32 iterations = 5;
		KCL::uint64 dt = 0;
		double avg_time = 0.0;
		warm_up->BeginTimer();
		for (KCL::uint32 j = 0; j < iterations; j++)
		{
			const MTLSize threadsPerGroup = { m_simulate_wg_size, 1, 1 };
			const MTLSize numThreadgroups = { wg_count, 1, 1};

			[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
		}
		[encoder endEncoding];
		[command_buffer commit];
		[command_buffer waitUntilCompleted];

		dt = warm_up->EndTimer();
		avg_time = double(dt) / double(iterations);

		NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

		if (dt < 50)
		{
			id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
			id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

			m_particle_emit_pipeline->SetAsCompute(encoder);
			[encoder setBuffer:m_particle_vertices_buffer offset:0 atIndex:0];
			[encoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];

			// Warm up until 500ms but maximalize the max iteration count
			iterations = avg_time > 0.01 ? KCL::uint32(500.0 / avg_time) : 200;

			iterations = KCL::Max(iterations, 5u);
			iterations = KCL::Min(iterations, 200u);

			NGLOG_INFO("  warmup %s iterations...", iterations);
			warm_up->BeginTimer();
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				const MTLSize threadsPerGroup = { m_simulate_wg_size, 1, 1 };
				const MTLSize numThreadgroups = { wg_count, 1, 1};

				[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
			}
			[encoder endEncoding];
			[command_buffer commit];
			[command_buffer waitUntilCompleted];

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


KCL::KCL_Status ComputeEmitter::WarmupSimulateShader(WarmUpHelper *warm_up)
{
	NGLOG_INFO("Warm up ParticleSystem Simulate shader...");
	KCL::KCL_Status status;

	status = InitSimulateWarmUp();
	if (status != KCL::KCL_TESTERROR_NOERROR)
	{
		return status;
	}

	MTL_Scene_40 * scene = warm_up->GetScene();

	_particle* warmup_data = new _particle[m_num_max_particles];

	//
	//	Save the loaded warmup data
	//
	_particle* particles = (_particle*)[m_particle_vertices_buffer contents];
	memcpy(warmup_data, particles, m_num_max_particles * sizeof(_particle));


	double best_time = INT_MAX;
	KCL::uint32 best_wg_size = 0;
	KCL::uint32 sizes[] = { 8, 16, 32, 64, 128, 256 };
	for (KCL::uint32 i = 0; i < COUNT_OF(sizes); i++)
	{
		// restore the data used for warmup
		_particle* particles = (_particle*)[m_particle_vertices_buffer contents];
		memcpy(particles, warmup_data, m_num_max_particles * sizeof(_particle));
		
#if !TARGET_OS_EMBEDDED
		[m_particle_vertices_buffer didModifyRange:NSMakeRange(0, m_num_max_particles * sizeof(_particle))];
#endif

		KCL::uint32 wg_size = sizes[i];
		//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particles_vbo);

		if (!warm_up->GetValidator()->Validate(wg_size))
		{
			continue;
		}

		status = LoadSimulateShader(wg_size);
		if (status != KCL::KCL_TESTERROR_NOERROR)
		{
			continue;
		}

		if (wg_size > m_particle_simulate_pipeline->GetMaxThreadCount())
		{
			continue;
		}

		id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
		id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

		m_particle_simulate_pipeline->SetAsCompute(encoder);
		[encoder setBuffer:m_particle_vertices_buffer offset:0 atIndex:0];
		[encoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];

		NGLOG_INFO("Workgroup size: %s", wg_size);
		KCL::uint32 wg_count = (m_num_max_particles + wg_size - 1) /wg_size;

		// First try with 5 iterations
		KCL::uint32 iterations = 5;
		KCL::uint64 dt = 0;
		double avg_time = 0.0;
		warm_up->BeginTimer();
		for (KCL::uint32 j = 0; j < iterations; j++)
		{
			const MTLSize threadsPerGroup = { m_simulate_wg_size, 1, 1 };
			const MTLSize numThreadgroups = { wg_count, 1, 1};
			
			if (m_single_frame)
			{
				// set 0 delta time for the simulation
				float zero = 0.0f;
				[encoder setBytes:&zero length:sizeof(zero) atIndex:2];
			}

			[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
		}
		[encoder endEncoding];
		[command_buffer commit];
		[command_buffer waitUntilCompleted];

		dt = warm_up->EndTimer();
		avg_time = double(dt) / double(iterations);

		NGLOG_INFO("  result after %s interations: sum: %sms, avg time: %sms", iterations, float(dt), float(avg_time));

		if (dt < 50)
		{
			id<MTLCommandBuffer> command_buffer = [scene->GetCommandQueue() commandBuffer];
			id<MTLComputeCommandEncoder> encoder = [command_buffer computeCommandEncoder];

			m_particle_simulate_pipeline->SetAsCompute(encoder);
			[encoder setBuffer:m_particle_vertices_buffer offset:0 atIndex:0];
			[encoder setBuffer:m_particle_dynamic_buffer->GetCurrentBuffer() offset:0 atIndex:1];

			// Warm up until 500ms but maximalize the max iteration count
			iterations = avg_time > 0.01 ? KCL::uint32(500.0 / avg_time) : 200;

			iterations = KCL::Max(iterations, 5u);
			iterations = KCL::Min(iterations, 200u);

			NGLOG_INFO("  warmup %s iterations...", iterations);
			warm_up->BeginTimer();
			for (KCL::uint32 j = 0; j < iterations; j++)
			{
				const MTLSize threadsPerGroup = { m_simulate_wg_size, 1, 1 };
				const MTLSize numThreadgroups = { wg_count, 1, 1};
				
				if (m_single_frame)
				{
					// set 0 delta time for the simulation
					float zero = 0.0f;
					[encoder setBytes:&zero length:sizeof(zero) atIndex:2];
				}

				[encoder dispatchThreadgroups:numThreadgroups threadsPerThreadgroup:threadsPerGroup];
			}
			[encoder endEncoding];
			[command_buffer commit];
			[command_buffer waitUntilCompleted];

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


void ComputeEmitter::ResetParticleSystemSeed()
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
void ComputeEmitter::EmitterStreamOperation(T & sstream)
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
void ComputeEmitter::ParticleStreamOperation(_particle &particle, T & sstream)
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
void ComputeEmitter::StateOperation(T & sstream)
{
	EmitterStreamOperation<write>(sstream);
	BufferStreamOperation<write>(m_movement,sstream);
	StreamNewLine(sstream);

	_particle* particles = (_particle*)[m_particle_vertices_buffer contents];

	for (unsigned int i = 0; i < m_num_max_particles; i++)
	{
		ParticleStreamOperation<write>(particles[i], sstream);
	}

	if (!write)
	{
#if !TARGET_OS_EMBEDDED
		[m_particle_vertices_buffer didModifyRange:NSMakeRange(0, m_num_max_particles*sizeof(_particle))];
#endif
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
}


class ParticleFile : public KCL::File
{
public:
	ParticleFile(const std::string& filename, const KCL::OPENMODE& mode, const KCL::WORKING_DIRERCTORY& path, const bool no_throw = true);
	virtual ~ParticleFile()
	{
	}
};


KCL::KCL_Status ComputeEmitter::LoadStateByFilename(const char * filename)
{
	KCL::File* particle_file = NULL;
	
	// try read and write directories
	ParticleFile particle_file_datar(filename, KCL::Read, KCL::RDir);
	ParticleFile particle_file_datarw(filename, KCL::Read, KCL::RWDir);
	
	if (particle_file_datar.GetLastError() == KCL::KCL_IO_NO_ERROR)
	{
		particle_file = &particle_file_datar;
	}
	else
	{
		particle_file = &particle_file_datarw;
	}
	
	if (particle_file->GetLastError() == KCL::KCL_IO_NO_ERROR)
	{
#if BINARY_DUMP
		StateOperation<false>(*particle_file);
#else
		std::stringstream sstream(*particle_file.GetBuffer());
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


KCL::KCL_Status ComputeEmitter::LoadState(KCL::uint32 anim_time)
{
	std::string filename = PARTICLE_BUFFERS_FILENAME(anim_time, m_name);

	return LoadStateByFilename(filename.c_str());
}


void ComputeEmitter::SaveState(KCL::uint32 anim_time)
{
	ParticleFile particle_file(PARTICLE_BUFFERS_FILENAME(anim_time,m_name), KCL::Write, KCL::RWDir);

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


void ComputeEmitter::SynchronizeParticleBuffer(id <MTLCommandQueue> command_queue)
{
	id <MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
#if !TARGET_OS_IPHONE
	id <MTLBlitCommandEncoder> encoder = [command_buffer blitCommandEncoder];
	[encoder synchronizeResource:m_particle_vertices_buffer];
	[encoder endEncoding];
#endif
	[command_buffer commit];
	[command_buffer waitUntilCompleted];
}


ParticleFile::ParticleFile(const std::string& filename, const KCL::OPENMODE& mode, const KCL::WORKING_DIRERCTORY& path, const bool no_throw)
{
	m_original_filename = filename;
	m_mode = mode;
	m_error = KCL::KCL_IO_NO_ERROR;
	m_working_dir = path;
	m_buffer = 0;
	m_file_pointer = 0;
	m_filesize = -1;
	m_is_filemapped = false;
	
	if (path == KCL::RDir)
	{
		m_filename = GetDataPath() + filename;
	}
	else if (path == KCL::RWDir)
	{
		m_filename = GetDataRWPath() + filename;
	}
	
	open_file_(no_throw);
}

