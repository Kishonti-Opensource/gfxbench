/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_PARTICLESYSTEM4_H
#define GLB_PARTICLESYSTEM4_H

#include "platform.h"
#include "kcl_math3d.h"
#include "memory"
#include "kcl_node.h"
#include "kcl_particlesystem2.h"
#include "jsonserializer.h"
#include "glb_scene_opengl4.h"

#include <sstream>

#define ENABLE_PARTICLE_SYSTEM_STAT 0

namespace GLB
{
	class GLBShader2;

	//aperture			 meter 0..2?
	//external_velocity  meter/sec 0..10?
	//focus distance	 meter -inf.. +inf 
	//emit rate			 unit/sec 0..+inf
	//gravity			 meter/sec 0..10
	//freq				 vec3 min/max 0.1 .. +inf
	//amp			     vec3 min/max 0..100?
	//accel				 min/max m/sec 0..10?
	//particle Lifetime  sec 0.1 .. 10
	class ComputeEmitter : public KCL::Node
	{
public:
	struct _particle
	{
		KCL::Vector4D Pos; // particle instance's position
		KCL::Vector4D Velocity;
		KCL::Vector4D T;
		KCL::Vector4D B;
		KCL::Vector4D N;
		KCL::Vector4D Phase;
		KCL::Vector4D Frequency;
		KCL::Vector4D Amplitude;
		KCL::Vector4D Age_Speed_Accel_Active;

		_particle()
		{
			Age_Speed_Accel_Active.w = 0.0f;
		}
	};

	ComputeEmitter( KCL::uint32 num_max_particles, bool single_frame);
	~ComputeEmitter();
	void Animate( float timestep, KCL::uint32 anim_time);
	bool Emit();
	bool Simulate();
	KCL::KCL_Status Init(WarmUpHelper *warm_up);
	void Render( GLBShader2 *billboard_point_gs, const KCL::Matrix4x4  &mvp, const KCL::Matrix4x4 &view_matrix, const KCL::Vector3D &eye, KCL::Texture *textures[2], KCL::uint32 topdown_sampler, KCL::Vector3D &eye_forward);

	bool NeedToEmit() const;
	bool NeedToSimulate() const;

	void SetAperture( float x, float y, float z);
	void SetExternalVelocity( float x, float y, float z);
	void SetFocusDistance( float f);
	void SetEmitRate( float f);
	void SetGravity( float f);
	void SetPom( KCL::Matrix4x4 &pom);

	void AdjustToDisplace4( KCL::Matrix4x4 &pom, float emit_factor);

	void SetParticleLifetime( float f);
	void SetParticleRange( _particle &min_, _particle &max_);


	virtual void Serialize(JsonSerializer& s);

	static void ResetParticleSystemSeed() ;

	void SaveState(KCL::uint32 anim_time);
	KCL::KCL_Status LoadState(KCL::uint32 anim_time);

	private:
		KCL::KCL_Status LoadEmitShader(KCL::uint32 wg_size);
		KCL::KCL_Status LoadSimulateShader(KCL::uint32 wg_size);

		KCL::KCL_Status WarmupEmitShader(WarmUpHelper *warm_up);
		KCL::KCL_Status WarmupSimulateShader(WarmUpHelper *warm_up);

		KCL::KCL_Status InitEmitWarmUp();
		KCL::KCL_Status InitSimulateWarmUp();

		KCL::KCL_Status LoadStateByFilename(const char * filename);

		struct _emitter
		{
			KCL::int32 EmitCount_LastParticleIndex_NumSubSteps[4];
			KCL::Matrix4x4 pom;
			KCL::Vector4D external_velocity;//world
			KCL::Vector4D aperture;
			KCL::Vector4D Focus_Life_Rate_Gravity;
			KCL::Vector4D SpeedMin_SpeedMax_AccelMin_AccelMax;
			KCL::Vector4D FreqMin;
			KCL::Vector4D FreqMax;
			KCL::Vector4D AmpMin;
			KCL::Vector4D AmpMax;

			_emitter()
			{
				EmitCount_LastParticleIndex_NumSubSteps[0] = 0;
				EmitCount_LastParticleIndex_NumSubSteps[1] = 0;
				EmitCount_LastParticleIndex_NumSubSteps[2] = 0;
				EmitCount_LastParticleIndex_NumSubSteps[3] = 0;
			}
		};

		KCL::uint32 m_num_max_particles;
		KCL::uint32 m_particles_vao[2];
		KCL::uint32 m_particles_vbo;
		KCL::uint32 m_particles_ebo[2];
		KCL::uint32 m_emitter_ubo;
		KCL::uint32 m_temp_particles_vbo;
		float m_emit_count;
		float m_simulation_count;
		float m_last_particle_age;
		_emitter m_emitter;

		GLBShader2 *m_particle_emit_shader;
		GLBShader2 *m_particle_simulate_shader;
		KCL::uint32 m_emit_wg_size;
		KCL::uint32 m_simulate_wg_size;

		bool m_single_frame;
		_emitter m_single_frame_emitter;
		KCL::Vector3D m_single_frame_movement;

		KCL::Matrix4x4 m_current_pom;
		KCL::Matrix4x4 m_prev_pom;
		KCL::Vector3D m_movement;
		friend class GFXGui;

		//	particle system state load save
		template <bool write, typename T > void EmitterStreamOperation(T & sstream);
		template <bool write, typename T > void ParticleStreamOperation(_particle & particle, T & sstream);
		template <bool write, typename T > void StateOperation(T & sstream);

#if ENABLE_PARTICLE_SYSTEM_STAT
		KCL::int32  m_min_emit_count, m_max_emit_count;
		KCL::uint32 m_min_emit_frame, m_max_emit_frame;
		KCL::uint64 m_sum_emit_count ;

		KCL::int32  m_min_simulation_count, m_max_simulation_count;
		KCL::uint32 m_min_simulation_frame, m_max_simulation_frame;
		KCL::uint64 m_sum_simulation_count;

		KCL::uint32 m_stat_collect_count;

		std::map<KCL::uint32, KCL::int32> m_emit_per_frame;
		std::map<KCL::uint32, KCL::int32> m_simulation_per_frame;

		void InitStat();
		void CollectStat(KCL::uint32 anim_time);
		void DumpStat();
#else
		void InitStat() {}
		void CollectStat(KCL::uint32 anim_time) {}
		void DumpStat() {}
#endif

	};
}

#endif
