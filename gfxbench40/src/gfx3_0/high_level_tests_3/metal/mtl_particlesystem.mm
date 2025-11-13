/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_particlesystem.h"
#include "mtl_globals.h"
#include "mtl_factories.h"

#include "mtl_types.h"
#import "graphics/metalgraphicscontext.h"

//position and UV (4 for fan, 6 for 2 tris)
static const float geometry[] =
{
	-1.0f,	-1.0f,	0.0f,	1.0f,
	1.0f,	-1.0f,	1.0f,	1.0f,
	-1.0f,	1.0f,	0.0f,	0.0f,
	1.0f,	1.0f,	1.0f,	0.0f,
	
	//1.0f,	1.0f,	1.0f,	0.0f,
	
	//-1.0f,	-1.0f,	0.0f,	1.0f
};

id <MTLBuffer> MetalRender::TF_emitter::s_billboardDataBuffer = nil;

//factory
KCL::_emitter* MetalRender::EmitterFactory::New(const std::string &name, KCL::ObjectType type, KCL::Node* parent, KCL::Object* owner)
{
	return new MetalRender::TF_emitter(name, type, parent, owner);
}

MetalRender::TF_emitter::TF_emitter( const std::string &name, KCL::ObjectType type, Node *parent, Object *owner) : KCL::_emitter( name, type, parent, owner), m_startBirthIdx(0), m_endBirthIdx(0), m_noOverflow(1),
    m_Device(MetalRender::GetContext()->getDevice())
{
 	m_max_particle_count = MAX_BUFFER_SIZE;
	
	m_numSubsteps = 0;
	m_actual_rate = m_rate;
	
    m_InitializeData = (float*) malloc(m_max_particle_count*GetParticleDataStride());
}

MetalRender::TF_emitter::~TF_emitter()
{
	for(int i = 0; i < kBufferCount; i++)
	{
		releaseObj(m_particleDataBuffer[i]);
	}
    free(m_InitializeData);
}


void MetalRender::TF_emitter::InitBillboardDataBuffer()
{
	id <MTLDevice> device = MetalRender::GetContext()->getDevice();
#if !TARGET_OS_EMBEDDED
	s_billboardDataBuffer = [device newBufferWithBytes:geometry length:sizeof(geometry) options:MTLResourceStorageModeManaged];
#else
	s_billboardDataBuffer = [device newBufferWithBytes:geometry length:sizeof(geometry) options:MTLResourceOptionCPUCacheModeDefault];
#endif
}

void MetalRender::TF_emitter::Process()
{
	//sParticleDataFloatCount MUST be changed if this changes!!!!
	//shader.cpp / glTransformFeedbackVaryings MUST be changed if this changes!!!!
	
	//TODO m_InitializeData can be static
    memset(m_InitializeData, 0, m_max_particle_count*GetParticleDataStride());
	
	//TODO probably don't actually need to do this - just read from vid instead
    for (int i = 0; i < m_max_particle_count; i++)
	{
        m_InitializeData[(i*sParticleDataFloatCount) + (sParticleDataFloatCount -1)] = (float)i;
		ParticleData* d = reinterpret_cast<ParticleData*>(m_InitializeData);
		assert(d[i].in_Velocity.w == float(i));
		assert(m_InitializeData[(i*sParticleDataFloatCount) + (sParticleDataFloatCount -1)] == float(i));
	}
	
	// Setup Render VAOs
	for (int i = 0; i < kBufferCount; i++)
	{
#if !TARGET_OS_EMBEDDED
        m_particleDataBuffer[i] = [m_Device newBufferWithBytes:reinterpret_cast<void*>(m_InitializeData) length:m_max_particle_count*GetParticleDataStride() options:MTLResourceStorageModeManaged];
#else
		m_particleDataBuffer[i] = [m_Device newBufferWithBytes:reinterpret_cast<void*>(m_InitializeData) length:m_max_particle_count*GetParticleDataStride() options:MTLResourceOptionCPUCacheModeDefault];
#endif
	}
	
	m_readBuffer = m_particleDataBuffer[0];
	m_writeBuffer = m_particleDataBuffer[1];
}

KCL::uint32 MetalRender::TF_emitter::Emit( KCL::uint32 time)
{
	return 0;
}

void MetalRender::TF_emitter::SimulateSubStep()
{
	const float rate_divider = 40.0f;
	m_emit_count += m_actual_rate / rate_divider;
	
	m_emit_count = std::min<float>(m_emit_count, float(m_max_particle_count) / 4.0f); //cap max emission to limit overwriting existing
	
	m_startBirthIdx = m_endBirthIdx;
	
	m_endBirthIdx = (m_startBirthIdx + (KCL::uint32)m_emit_count) % m_max_particle_count;
	m_noOverflow = m_startBirthIdx <= m_endBirthIdx;
	
	m_visibleparticle_count = m_max_particle_count; //no CPU culling possible using transform feedback
	
	if( m_emit_count >= 1.0f)
	{
		m_emit_count = 0.0f;
	}
}

void MetalRender::TF_emitter::Simulate( KCL::uint32 time_msec)
{
	//if(IsPaused())
	//{
	//	return;
	//}
	
	m_prev_time2 = m_time;
	m_time = time_msec;
	
	float diff_time_sec = (m_time - m_prev_time2) / 1000.0f;
	
	m_actual_rate = m_rate;
	m_numSubsteps = CalculateNumSubsteps( diff_time_sec);
	
	//INFO("%d\n", m_num_active_particles);
	
	if( m_spawning_rate_animated)
	{
		float nothing=0;
		float t = time_msec / 1000.0f;
		
		KCL::Vector4D result;
		
		KCL::_key_node::Get(result, m_spawning_rate_animated, t, nothing);
		
		if( result.x > 1.0f)
		{
			result.x = 1.0f;
		}
		m_actual_rate *= result.x;
	}
}

KCL::Vector3D MetalRender::TF_emitter::GetTotalAccel()
{
	return Gravity() + get_additional_acceleration();
}

void MetalRender::TF_emitter::GetStartTransform(KCL::Matrix4x4& startTform)
{
	KCL::Vector3D startPos;
	
	if(m_is_local_emitting)
	{
		startTform.identity();
	}
	else
	{
		startTform = m_world_pom;
	}
}


#ifdef USE_METAL

KCL::AnimatedEmitter* MetalRender::MTLAnimatedEmitterFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
    return new MetalRender::TF_emitter( name, KCL::EMITTER2, parent, owner);
}

#endif


