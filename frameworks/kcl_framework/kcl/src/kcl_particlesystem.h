/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_PARTICLESYSTEM_H
#define KCL_PARTICLESYSTEM_H

#include <kcl_base.h>
#include <kcl_mesh.h>
#include <kcl_camera2.h>
#include <kcl_factory_base.h>
#include <map>


#ifndef MAX_PARTICLE_PER_EMITTER
#define MAX_PARTICLE_PER_EMITTER 1000
#endif

#define MAX_PARTICLE_PER_MESH 50
#define MAX_MESH_PER_EMITTER (MAX_PARTICLE_PER_EMITTER / MAX_PARTICLE_PER_MESH)

/*

Aperture of emitter in XZ plane, flame emitted in Y direction:

            Y|
             |
             ------------------------> X
            /
           /        f
          /          f
         /        f f
        /          ff f
       /          f fff
      /          fffffff
     /            fffff
  Z /

*/

namespace KCL
{
	struct ParticleRenderAttrib
	{
		ParticleRenderAttrib()
			:
			m_position_x(0),
			m_position_y(0),
			m_position_z(0),
			m_life_normalized(0),
			m_speed_x(0),
			m_speed_y(0),
			m_speed_z(0)
		{
		}

		float m_position_x;
		float m_position_y;
		float m_position_z;
		float m_life_normalized;
		float m_speed_x;
		float m_speed_y;
		float m_speed_z;
		float m_fractional_lifetime;
		float m_color_x;
		float m_color_y;
		float m_color_z;
		float m_color_w;

		void SetPosition( const KCL::Vector3D & position)
		{
			m_position_x = position.x;
			m_position_y = position.y;
			m_position_z = position.z;
		}


		void SetLife( float lifeNormalized, float fractional_lifetime)
		{
			m_life_normalized = lifeNormalized;
			m_fractional_lifetime = fractional_lifetime;
		}


		void SetSpeed( const KCL::Vector3D &speed)
		{
			m_speed_x = speed.x;
			m_speed_y = speed.y;
			m_speed_z = speed.z;
		}


		void SetColor( const KCL::Vector4D &c)
		{
			m_color_x = c.x;
			m_color_y = c.y;
			m_color_z = c.z;
			m_color_w = c.w;
		}
	};


	struct Particle
	{
		float m_random_offset;
		KCL::uint32 m_birth_time;
		float m_max_lifetime;
		float m_current_lifetime;

		float m_scale; //??
		float m_fade; //?? [0..1], 0 == invisible
		float m_rotation_angle; //??
		float m_angular_velocity; //??

		KCL::Vector3D m_speed;
		KCL::Vector3D m_world_position;
		KCL::Vector4D m_color;


		Particle()
		{
			Reset();
		}

		virtual ~Particle()
		{
		}

		virtual void Reset()
		{
			m_birth_time = 0;
			m_max_lifetime = -1.0f;
			m_current_lifetime = 0.0f;
			m_scale = 1.0f;
			m_fade = 1.0f;
			m_rotation_angle = 0.0f;
			m_angular_velocity = 0.0f;
			m_speed.x = 0.0f;
			m_speed.y = 0.0f;
			m_speed.z = 0.0f;
			m_world_position.x = 0.0f;
			m_world_position.y = 0.0f;
			m_world_position.z = 0.0f;
		}

		virtual void UpdateSpeed(float diff_time, const KCL::Vector3D &acceleration)
		{
			m_speed += acceleration * diff_time;
		}

		virtual void UpdatePosition(float diff_time)
		{
			m_world_position += m_speed * diff_time;
		}

		virtual bool IsAlive() const
		{
			return m_current_lifetime <= m_max_lifetime;
		}

		virtual bool IsDead() const
		{
			return m_current_lifetime > m_max_lifetime;
		}

		virtual float FractionalLifetime() const
		{
			float fracp, intp;
			fracp = std::modf( m_random_offset + m_current_lifetime * 0.00005f, &intp);
			return fracp;
		}

		float LifeNormalized() const
		{
			if(m_max_lifetime == 0)
			{
				return 0.0f;
			}
			float result = 1.0f - m_current_lifetime/m_max_lifetime;
			if(result < 0.0f)
			{
				return 0.0f;
			}
			if(result > 1.0f)
			{
				return 1.0f;
			}
			return result;
		}

		void SetColor( const KCL::Vector4D &c)
		{
			m_color = c;
		}
	};


	class Emitter : public Node
	{
	public:
		std::string m_emitter_name;

		KCL::Mesh m_meshes[MAX_MESH_PER_EMITTER];

		virtual ~Emitter();
		virtual void Init(KCL::uint32 max_particle_count);

		virtual void Simulate( KCL::uint32 time);

		void SetName(std::string emitter_name)
		{
			m_emitter_name = emitter_name;
		}

		void Pause()
		{
			if(!m_is_active)
			{
				return;
			}
			m_is_pause = true;
		}

		void Continue()
		{
			if(!m_is_active)
			{
				return;
			}
			m_is_pause = false;
		}

		void Stop()
		{
			if(!m_is_active)
			{
				return;
			}

			m_is_pause = false;
			m_is_active = false;

			m_visibleparticle_count = 0;

			for(size_t i=0; i<m_particles.size(); ++i)
			{
				m_particles[i]->Reset();
			}
		}

		void Start()
		{
			if(m_is_active)
			{
				return;
			}

			m_is_active = true;

			m_visibleparticle_count = 0;

			for(size_t i=0; i<m_particles.size(); ++i)
			{
				m_particles[i]->Reset();
			}

			m_prev_time = -1.0f;
			m_emit_count_accumulator = 0.0f;
			//TODO: reset other stuff
		}


		bool IsActive() const
		{
			return m_is_active;
		}

		bool IsPaused() const
		{
			return m_is_pause;
		}

		void EmitLocal(bool b)
		{
			m_is_local_emitting = b;
		}

		bool IsLocalEmitting() const
		{
			return m_is_local_emitting;
		}

		void Focusing(bool b)
		{
			m_is_focusing = b;
		}

		bool IsFocusing() const
		{
			return m_is_focusing;
		}

		const KCL::Vector3D& Gravity() const
		{
			return m_gravity;
		}

		void SetGravity(const KCL::Vector3D gravity)
		{
			m_gravity = gravity;
		}

		void SetColor(const KCL::Vector4D c)
		{
			m_color = c;
		}

		KCL::uint32 VisibleParticleCount() const
		{
			return m_visibleparticle_count;
		}

		size_t MaxParticleCount() const
		{
			return m_particles.size();
		}

		const std::vector<Particle*>& Particles() const
		{
			return m_particles;
		}

		const std::vector<ParticleRenderAttrib>& ParticleRenderAttribs() const
		{
			return m_particle_render_attribs;
		}

		int m_emitter_type;

		virtual void DepthSort( const Camera2 &camera);

		const KCL::Vector4D GetColor()
		{
			return m_color;
		}

		KCL::uint32 m_visibleparticle_count;

	protected:

		virtual KCL::uint32 Emit( KCL::uint32 time);

		virtual void ResurrectParticle( Particle *particle);
		virtual void InitialParticleSpeedAndPosition( KCL::Vector3D &speed, KCL::Vector3D &position);

		virtual void UpdateParticle( Particle *particle);

		virtual float get_spawning_rate() = 0; //how many particles are generated per unit of time
		virtual float get_max_angle() = 0;
		virtual float get_aperture_x() = 0;
		virtual float get_aperture_z() = 0;
		virtual float get_min_initial_velocity() = 0;
		virtual float get_max_initial_velocity() = 0;
		virtual float get_min_initial_lifetime() = 0;
		virtual float get_max_initial_lifetime() = 0;
		virtual const KCL::Vector3D get_additional_acceleration() = 0;
		virtual const KCL::Vector4D get_color() = 0;

		//Emitter
		int m_random_seed;
		bool m_is_active;
		bool m_is_pause;
		bool m_is_local_emitting;
		bool m_is_focusing;
		float m_prev_time;
		float m_prev_time_div_1000;
		float m_diff_time;
		float m_diff_time_div_1000;
		float m_emit_count_accumulator;
		KCL::Vector3D m_gravity;
		KCL::Vector4D m_color;

		KCL::uint32 m_max_particle_count;

		std::vector<Particle*> m_particles;
		std::vector<ParticleRenderAttrib> m_particle_render_attribs;


	protected:
		Emitter( const std::string &name, ObjectType type, Node *parent, Object *owner);
	private:
		Emitter(const Emitter&);
		Emitter& operator=(const Emitter&);
	};


	class DummyEmitter : public Emitter
	{
	public:

		void Init(KCL::uint32 max_particle_count);

	private:
		float get_spawning_rate();
		float get_max_angle();
		float get_aperture_x();
		float get_aperture_z();
		float get_min_initial_velocity();
		float get_max_initial_velocity();
		float get_min_initial_lifetime();
		float get_max_initial_lifetime();
		const KCL::Vector3D get_additional_acceleration();
		const KCL::Vector4D get_color();

		float m_spawning_rate; //how many particles are generated per unit of time
		float m_max_angle;
		float m_aperture_x;
		float m_aperture_z;
		float m_min_initial_velocity;
		float m_max_initial_velocity;
		//float m_additional_acceleration_when_to_change_range[2];
		KCL::Vector3D m_additional_acceleration_range[2];
		float m_lifetime_range[2];
	};


	class AnimatedEmitter : public Emitter
	{
	public:

		~AnimatedEmitter();

		void Init(KCL::uint32 max_particle_count);

		void Set_Spawning_rate_animated ( _key_node* spawning_rate );
		void Set_Max_angle_animated ( _key_node* max_angle );
		void Set_Aperture_x_animated ( _key_node* aperture_x );
		void Set_Aperture_z_animated ( _key_node* aperture_z );
		void Set_Min_initial_velocity_animated ( _key_node* min_initial_velocity );
		void Set_Max_initial_velocity_animated ( _key_node* max_initial_velocity );
		void Set_Min_initial_lifetime_animated ( _key_node* min_initial_lifetime );
		void Set_Max_initial_lifetime_animated ( _key_node* max_initial_lifetime );
		void Set_Additional_acceleration_animated ( _key_node* additional_acceleration);

		void Set_Spawning_rate ( float spawning_rate ){ m_spawning_rate = spawning_rate;}
		void Set_Max_angle ( float max_angle ){ m_max_angle = max_angle;}
		virtual void Set_Aperture_x ( float aperture_x ){ m_aperture_x	= aperture_x;}
		virtual void Set_Aperture_z ( float aperture_z ){ m_aperture_z	= aperture_z;}
		void Set_Min_initial_velocity ( float min_initial_velocity ){ m_min_initial_velocity = min_initial_velocity;}
		void Set_Max_initial_velocity ( float max_initial_velocity ){ m_max_initial_velocity = max_initial_velocity;}
		void Set_Min_initial_lifetime ( float min_initial_lifetime ){ m_min_initial_lifetime = min_initial_lifetime;}
		void Set_Max_initial_lifetime ( float max_initial_lifetime ){ m_max_initial_lifetime = max_initial_lifetime;}
		void Set_Additional_acceleration ( const KCL::Vector3D &additional_acceleration) { m_additional_acceleration = additional_acceleration;}

		bool HasRateAnimation()
		{
			return m_spawning_rate_animated != 0;
		}

		virtual void InitAsSpark()
		{
			SetGravity(KCL::Vector3D(0, -10.0, 0));
			m_emitter_type = 0;
		}
		virtual void InitAsFire()
		{
			SetGravity(KCL::Vector3D(0, 1.0, 0));
			Focusing(true);
			m_emitter_type = 1;
		}
		virtual void InitAsSmoke()
		{
			SetGravity(KCL::Vector3D(0, 1.0 ,0));
			Focusing(false);
			m_emitter_type = 2;
		}
		virtual void InitAsSteam()
		{
			Focusing(false);
			m_emitter_type = 3;
		}

		std::string m_rate_track_name;

		virtual void LoadParameters(KCL::SceneHandler *scene);
		virtual void SaveParameters(KCL::SceneHandler *scene);

		AnimatedEmitter(const std::string &name, ObjectType type, Node *parent, Object *owner);
	protected:
		float get_spawning_rate();
		float get_max_angle();
		float get_aperture_x();
		float get_aperture_z();
		float get_min_initial_velocity();
		float get_max_initial_velocity();
		float get_min_initial_lifetime();
		float get_max_initial_lifetime();
		const KCL::Vector3D get_additional_acceleration();
		const KCL::Vector4D get_color();

		_key_node* m_spawning_rate_animated;
		_key_node* m_max_angle_animated;
		_key_node* m_aperture_x_animated;
		_key_node* m_aperture_z_animated;
		_key_node* m_min_initial_velocity_animated;
		_key_node* m_max_initial_velocity_animated;
		_key_node* m_min_initial_lifetime_animated;
		_key_node* m_max_initial_lifetime_animated;
		_key_node* m_additional_acceleration_animated;

		float m_spawning_rate;
		float m_max_angle;
		float m_aperture_x;
		float m_aperture_z;
		float m_min_initial_velocity;
		float m_max_initial_velocity;
		float m_min_initial_lifetime;
		float m_max_initial_lifetime;
		KCL::Vector3D m_additional_acceleration;
	};



	class AnimatedEmitterFactory : public KCL::FactoryBase
	{
	public:
		virtual ~AnimatedEmitterFactory();
		virtual KCL::AnimatedEmitter *Create(const std::string &name, Node *parent, Object *owner)
		{
			return new KCL::AnimatedEmitter(name, KCL::EMITTER1, parent, owner);
		}
	};


	class KCLFactories
	{
	public:
		void RegisterFactory(KCL::FactoryBase* factory, const ObjectType type)
		{
			std::map<ObjectType, FactoryBase*>::iterator it;
			if ( (it = factories.find(type)) != factories.end())
			{
				factories.erase(it);
			}
			factories[type] = factory;
		}

		FactoryBase* GetFactory(ObjectType type)
		{
			FactoryBase *factory = NULL;
			if (factories.find(type) != factories.end())
			{
				return factories[type];
			}

			return factory;
		}

		std::map<ObjectType, FactoryBase*> factories;

	};
}

#endif //KCL_PARTICLESYSTEM_H
