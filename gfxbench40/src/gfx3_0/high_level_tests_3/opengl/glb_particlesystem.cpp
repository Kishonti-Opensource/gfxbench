/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_particlesystem.h"
#include "platform.h"

//position and UV (4 for fan, 6 for 2 tris)
static const GLfloat geometry[] =
{
	-1.0f,	-1.0f,	0.0f,	1.0f,
	1.0f,	-1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	0.0f,

	//1.0f,	1.0f,	1.0f,	0.0f,
	-1.0f,	1.0f,	0.0f,	0.0f
	//-1.0f,	-1.0f,	0.0f,	1.0f
};

GLB::TF_emitter::TF_emitter( const std::string &name, KCL::ObjectType type, Node *parent, Object *owner) : KCL::_emitter( name, type, parent, owner), m_startBirthIdx(0), m_endBirthIdx(0), m_noOverflow(1)
{
	const int MAX_BUFFER_SIZE = 1000; //this shall be based on rate and lifetime
 	m_max_particle_count = MAX_BUFFER_SIZE;

 	for (int i = 0; i < 2 ; ++i)
 	{
 		m_instanceBufs[i] = 0;
 		m_advectVAOs[i] = 0;
		m_renderVAOs[i] = 0;
 	}
	m_geometryBuf = 0;

	m_numSubsteps = 0;
	m_actual_rate = m_rate;

    m_InitializeData = (float*) malloc(m_max_particle_count*GetParticleDataStride());

	m_ubo_handle = -1;
}

GLB::TF_emitter::~TF_emitter()
{
#if defined HAVE_GLES3 || defined __glew_h__

	glDeleteBuffers(1, &m_geometryBuf);
	glDeleteBuffers(2, m_instanceBufs);
	glDeleteVertexArrays(2, m_advectVAOs);
	glDeleteVertexArrays(2, m_renderVAOs);
#endif
	for (int i = 0; i < 2 ; ++i)
	{
		m_instanceBufs[i] = 0;
		m_advectVAOs[i] = 0;
		m_renderVAOs[i] = 0;
	}

    free(m_InitializeData);
}

void GLB::TF_emitter::Process()
{
#if defined HAVE_GLES3 || defined __glew_h__
	glGenBuffers(2, m_instanceBufs);
	glGenVertexArrays(2, m_advectVAOs);
	glGenVertexArrays(2, m_renderVAOs);
	glGenBuffers(1, &m_geometryBuf);
	//sParticleDataFloatCount MUST be changed if this changes!!!!
	//shader.cpp / glTransformFeedbackVaryings MUST be changed if this changes!!!!

    memset(m_InitializeData, 0, m_max_particle_count*GetParticleDataStride());

    for (unsigned int i = 0; i < m_max_particle_count; i++)
	{
        m_InitializeData[(i*sParticleDataFloatCount) + (sParticleDataFloatCount -1)] = (float)i;
	}

	// Setup Render VAOs
	for (int i = 0; i < 2; i++)
	{
		glBindVertexArray(m_advectVAOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufs[i]);
            glBufferData(GL_ARRAY_BUFFER, m_max_particle_count * GetParticleDataStride(), (void*)m_InitializeData, GL_DYNAMIC_COPY);

			glEnableVertexAttribArray(0); //position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), 0);

			glEnableVertexAttribArray(1); //age01, speed, accel
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(3*sizeof(float)));

			glEnableVertexAttribArray(2); //amplitude
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(6*sizeof(float)));

			glEnableVertexAttribArray(3); //phase
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(9*sizeof(float)));

			glEnableVertexAttribArray(4); //frequency
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(12*sizeof(float)));

			glEnableVertexAttribArray(5); //T
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(15*sizeof(float)));

			glEnableVertexAttribArray(6); //B
			glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(18*sizeof(float)));

			glEnableVertexAttribArray(7); //N
			glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(21*sizeof(float)));

			glEnableVertexAttribArray(8); //Velocity
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(24*sizeof(float)));

			//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		glBindVertexArray(m_renderVAOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufs[i]);
            glBufferData(GL_ARRAY_BUFFER, m_max_particle_count * GetParticleDataStride(), (void*)m_InitializeData, GL_DYNAMIC_COPY);

			glEnableVertexAttribArray(0); //position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), 0);

			glEnableVertexAttribArray(1); //age01, maxage, speed, accel
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(3*sizeof(float)));

			glEnableVertexAttribArray(2); //amplitude
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(6*sizeof(float)));

			glEnableVertexAttribArray(3); //phase
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(9*sizeof(float)));

			glEnableVertexAttribArray(4); //frequency
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(12*sizeof(float)));

			glEnableVertexAttribArray(5); //T
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(15*sizeof(float)));

			glEnableVertexAttribArray(6); //B
			glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(18*sizeof(float)));

			glEnableVertexAttribArray(7); //N
			glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(21*sizeof(float)));

			glEnableVertexAttribArray(8); //Velocity
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, GetParticleDataStride(), (const void*)(24*sizeof(float)));


			//glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindBuffer(GL_ARRAY_BUFFER, m_geometryBuf);
			if (i == 0) //only allocate the static geometry buffer once
			{
				glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
			}
			glEnableVertexAttribArray(9); //billboard corner position
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 0, 0);
			//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);
	}

	glBindVertexArray(0);
#endif
}

KCL::uint32 GLB::TF_emitter::Emit( KCL::uint32 time)
{
	return 0;
}

void GLB::TF_emitter::SimulateSubStep()
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

void GLB::TF_emitter::Simulate( KCL::uint32 time_msec)
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

KCL::Vector3D GLB::TF_emitter::GetTotalAccel()
{
	return Gravity() + get_additional_acceleration();
}

void GLB::TF_emitter::GetStartTransform(KCL::Matrix4x4& startTform)
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


KCL::AnimatedEmitter *GLB::TF_emitterFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{

		return new GLB::TF_emitter( name, KCL::EMITTER2, parent, owner);

}
