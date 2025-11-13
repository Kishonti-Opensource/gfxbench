/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_PARTICLESYSTEM_H
#define GLB_PARTICLESYSTEM_H

#include <kcl_particlesystem.h>
#include <kcl_particlesystem2.h>

namespace GLB
{
	//NOTE: simulated on the GPU, does not support sorting!!!

	class TF_emitter : public KCL::_emitter
	{
		friend class KCL::AnimatedEmitter;


	public:
		/*override*/ void Process();
		/*override*/ void Simulate( KCL::uint32 time);
		/*override*/ KCL::uint32 Emit( KCL::uint32 time);

		void GetBufferData(KCL::int32& startBirthIdx, KCL::int32& endBirthIdx, KCL::int32& noOverflow, float& diffTime)
		{
			startBirthIdx = m_startBirthIdx;
			endBirthIdx = m_endBirthIdx;
			noOverflow = m_noOverflow;
			const float animate_diff_time = 0.025f;
			diffTime = animate_diff_time;
		}

		void GetEmitterParams(	KCL::Vector4D& emitter_apertureXYZ_focusdist,
								KCL::Matrix4x4& emitter_worldmat,
								KCL::Vector4D& emitter_min_freqXYZ_speed,
								KCL::Vector4D& emitter_max_freqXYZ_speed,
								KCL::Vector4D&emitter_min_ampXYZ_accel,
								KCL::Vector4D& emitter_max_ampXYZ_accel,
								KCL::Vector4D& emitter_color,
								KCL::Vector4D& emitter_externalVel_gravityFactor,
								KCL::Vector3D& emitter_maxlife_sizeXY
							)
		{
			emitter_apertureXYZ_focusdist = KCL::Vector4D(m_aperture.x, m_aperture.y, m_aperture.z, m_focus_distance);
			emitter_worldmat = m_world_pom;
			emitter_min_freqXYZ_speed = KCL::Vector4D(m_min.m_frequency.x, m_min.m_frequency.y, m_min.m_frequency.z, m_min.m_speed);
			emitter_max_freqXYZ_speed = KCL::Vector4D(m_max.m_frequency.x, m_max.m_frequency.y, m_max.m_frequency.z, m_max.m_speed);
			emitter_min_ampXYZ_accel = KCL::Vector4D(m_min.m_amplitude.x, m_min.m_amplitude.y, m_min.m_amplitude.z, m_min.m_acceleration);
			emitter_max_ampXYZ_accel = KCL::Vector4D(m_max.m_amplitude.x, m_max.m_amplitude.y, m_max.m_amplitude.z, m_max.m_acceleration);
			emitter_color = m_color;
			emitter_externalVel_gravityFactor = KCL::Vector4D(m_external_velocity.x, m_external_velocity.y, m_external_velocity.z, m_gravity_factor);
			emitter_maxlife_sizeXY = KCL::Vector3D(m_lifespan, m_begin_size, m_end_size);
		}

		void GetBuffers(KCL::uint32& advectVAO, KCL::uint32& advectBuffer, KCL::uint32& renderVAO)
		{
 			advectVAO = m_advectVAOs[0];		//source
			advectBuffer = m_instanceBufs[1];	//transform target
 			renderVAO = m_renderVAOs[1];		//render source = transform target
		}

		void SwitchBuffers()
		{
			std::swap(m_instanceBufs[0], m_instanceBufs[1]);
 			std::swap(m_advectVAOs[0], m_advectVAOs[1]);
			std::swap(m_renderVAOs[0], m_renderVAOs[1]);
		}

		KCL::uint32 GetParticleDataStride()
		{
			return sParticleDataFloatCount * sizeof(float);
		}

		void SimulateSubStep();

		float GetRandomSeed() { return float(m_random_seed % 128); } //will index a rnd 1D texture in the advect shader
		KCL::uint32 GetMaxParticleCount() { return m_max_particle_count; }
		void GetStartTransform(KCL::Matrix4x4& startTform);
		KCL::uint32 GetNumSubsteps() {return m_numSubsteps;}
		KCL::Vector3D GetTotalAccel();

		~TF_emitter();

		KCL::uint32 m_instanceBufs[2];

		KCL::int32 m_ubo_handle;

	public:
		TF_emitter( const std::string &name, KCL::ObjectType type, Node *parent, Object *owner);
		virtual AnimatedEmitter* Create(const std::string &name, Node *parent, Object *owner)
		{
			return new TF_emitter(name, KCL::EMITTER2, parent, owner);
		}



	private:

		static const KCL::uint32 sParticleDataFloatCount = 28; //Must be in sync with the shaders!

		KCL::uint32 m_advectVAOs[2];
		KCL::uint32 m_renderVAOs[2];


        float* m_InitializeData;
		KCL::uint32 m_geometryBuf; //TODO: this could be shared between all emitters...

		KCL::int32 m_startBirthIdx;
		KCL::int32 m_endBirthIdx;
		KCL::int32 m_noOverflow;

		KCL::uint32 m_numSubsteps;
		float m_actual_rate;
	};


	class TF_emitterFactory : public KCL::AnimatedEmitterFactory
	{
	public:

		virtual KCL::AnimatedEmitter *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
	};

}//namespace GLB

#endif

