/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_EMITTER
#define KCL_EMITTER


#include "kcl_particlesystem.h"

#define MAX_PARTICLES_PER_EMITTER 1024

#define MAX_PARTICLE_PER_MESH 50

namespace KCL
{
	struct _key_node;
	class Camera2;

	enum _emitter_type
	{
		ET_BILLBOARD = 0,
		ET_SPARK
	};


	struct ParticleRenderAttrib2
	{
		KCL::Vector3D m_pos;
		float m_life_normalized;
		KCL::Vector3D m_velocity;
		float m_size;
		KCL::Vector4D m_color;
	};
	
	struct _particle
	{
		KCL::Vector3D m_pos;
		KCL::Vector3D m_velocity;
		KCL::Vector3D m_t;
		KCL::Vector3D m_b;
		KCL::Vector3D m_n;
		float m_age;

		KCL::Vector3D m_phase;
		KCL::Vector3D m_frequency;
		KCL::Vector3D m_amplitude;
		float m_speed;
		float m_acceleration;

		_particle();

		void Init( KCL::Vector3D *pos, KCL::Vector3D *t, KCL::Vector3D *b, KCL::Vector3D *n, _particle &A, _particle &B);

		void Calculate( float delta_time, KCL::Vector3D *external_velocity, float gravity);
	};


	class _emitter : public AnimatedEmitter
	{
	public:
        static const int max_num_substeps = 5;

		friend class GLB_Scene_ES3;
		friend class SceneHandler;
	
		_particle particle_parameters[MAX_PARTICLES_PER_EMITTER];
		ParticleRenderAttrib2 attribs[MAX_PARTICLES_PER_EMITTER * 4];

		_emitter_type m_emitter_type2;

		_emitter( const std::string &name, ObjectType type, Node *parent, Object *owner);
		~_emitter();
		void InitAsSoot();
		void InitAsFire();
		void InitAsSmoke();
		void InitAsSpark();
		void Simulate( KCL::uint32 time_msec);

		virtual void Process() //platform specific processing after load
		{
		}

		void Init(KCL::uint32 max_particle_count)
		{
		}

		void DepthSort( const Camera2 &camera)
		{
		}

		void Set_Aperture_x ( float aperture_x )
		{
			m_aperture.x = aperture_x;
		}

		void Set_Aperture_y ( float aperture_y )
		{
			m_aperture.y = aperture_y;
		}

		void Set_Aperture_z ( float aperture_z )
		{
			m_aperture.z = aperture_z;
		}

		void SetRate( float r)
		{
			m_rate = r;
		}

		void SetFocusDistance( float r)
		{
			m_focus_distance = r;
		}

		void SetGravityFactor( float r)
		{
			m_gravity_factor = r;
		}

		void SetLifespan( float r)
		{
			m_lifespan = r;
		}

		void SetMinSpeed( float r)
		{
			m_min.m_speed = r;
		}

		void SetMaxSpeed( float r)
		{
			m_max.m_speed = r;
		}

		void SetMinAcceleration( float r)
		{
			m_min.m_acceleration = r;
		}

		void SetMaxAcceleration( float r)
		{
			m_max.m_acceleration = r;
		}

	protected:
		_particle m_min;
		_particle m_max;
		KCL::Vector3D m_external_velocity;
		KCL::Vector3D m_aperture;
		float m_focus_distance;
		float m_lifespan;
		float m_rate;
		float m_gravity_factor;
		float m_begin_size;
		float m_end_size;
		KCL::uint32 m_gen_type;

		KCL::uint32 m_last_particle_index;
		KCL::uint32 m_last_active_particle_index;
		KCL::uint32 m_num_active_particles;
		float m_emit_count;
		float m_accumulated_diff_time;
		KCL::uint32 m_time;
		KCL::uint32 m_prev_time2;

		KCL::uint32 CalculateNumSubsteps( float diff_time_sec);
		void InitParticle( _particle &pp);
		void ParticleIdxIncr( KCL::uint32 &i)
		{
			i++;
			i = i & (MAX_PARTICLES_PER_EMITTER - 1);
		}
		void CreateGeometry();
	};
}

#endif
