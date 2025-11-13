/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __GFXBench30__mtl_particlesystem__
#define __GFXBench30__mtl_particlesystem__

#include <kcl_particlesystem2.h>

#include <Metal/Metal.h>

#include "mtl_shader_constant_layouts_30.h"

namespace MetalRender
{
	//NOTE: simulated on the GPU, does not support sorting!!!
	
	class TF_emitter : public KCL::_emitter
	{
	public:
		friend class Factory;
		friend class EmitterFactory;
		
		static const KCL::uint32 sParticleDataFloatCount = 36; //Must be in sync with the shaders!
		static const KCL::uint32 s_particle_stride = sParticleDataFloatCount * sizeof(float);
		static const int MAX_BUFFER_SIZE = 1000; //this shall be based on rate and lifetime
		
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
		
		void GetBufferData(VectorI4D& packed)
		{
			packed.x = m_startBirthIdx;
			packed.y = m_endBirthIdx;
			packed.z = m_noOverflow;
			const float animate_diff_time = 0.025f;
			packed.w = animate_diff_time;
		}
		
		//TODO pack this into a buffer
		void GetEmitterParams(	KCL::Vector4D& emitter_apertureXYZ_focusdist,
							  KCL::Matrix4x4& emitter_worldmat,
							  KCL::Vector4D& emitter_min_freqXYZ_speed,
							  KCL::Vector4D& emitter_max_freqXYZ_speed,
							  KCL::Vector4D&emitter_min_ampXYZ_accel,
							  KCL::Vector4D& emitter_max_ampXYZ_accel,
							  KCL::Vector4D& emitter_color,
							  KCL::Vector4D& emitter_externalVel_gravityFactor,
							  KCL::Vector3D& emitter_maxlife_sizeXY)
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
		
		void GetEmitterParams(EmitterAdvectConsts* packedConsts)
		{
			packedConsts->emitter_apertureXYZ_focusdist = KCL::Vector4D(m_aperture.x, m_aperture.y, m_aperture.z, m_focus_distance);
			packedConsts->emitter_worldmat = m_world_pom;
			packedConsts->emitter_min_freqXYZ_speed = KCL::Vector4D(m_min.m_frequency.x, m_min.m_frequency.y, m_min.m_frequency.z, m_min.m_speed);
			packedConsts->emitter_max_freqXYZ_speed = KCL::Vector4D(m_max.m_frequency.x, m_max.m_frequency.y, m_max.m_frequency.z, m_max.m_speed);
			packedConsts->emitter_min_ampXYZ_accel = KCL::Vector4D(m_min.m_amplitude.x, m_min.m_amplitude.y, m_min.m_amplitude.z, m_min.m_acceleration);
			packedConsts->emitter_max_ampXYZ_accel = KCL::Vector4D(m_max.m_amplitude.x, m_max.m_amplitude.y, m_max.m_amplitude.z, m_max.m_acceleration);
			packedConsts->emitter_color = m_color;
			packedConsts->emitter_externalVel_gravityFactor = KCL::Vector4D(m_external_velocity.x, m_external_velocity.y, m_external_velocity.z, m_gravity_factor);
			packedConsts->emitter_maxlifeX_sizeYZ_pad = KCL::Vector4D(m_lifespan, m_begin_size, m_end_size, 0.0);
		}
		
		//TODO: need a way to double buffer this intelligently
		//just need to worry about overwriting stuff being read from
		id <MTLBuffer> GetReadBuffer()
		{
 			return m_readBuffer;
		}
		
		id <MTLBuffer> GetWriteBuffer()
		{
			return m_writeBuffer;
		}
		
		/*
			no need to fence because the flow is like this:
		 
			Frame	Read Buffer	Action
			(n/a)	A			Initial values in A and B
			0		A			TFX Into B
		 	0		B			Render with values in B
		 	0					swap read/write buffers
		 	1		B			TFX Into A
		 	1		A			Render
		 	1					swap buffers
		 
		 	So for every set of new values you will always:
		 	Read and render with those values
		 	Read and TFX into the other buffer
		*/
		void SwapBuffers()
		{
			id temp = m_readBuffer;
			m_readBuffer = m_writeBuffer;
			m_writeBuffer = temp;
		}
		
		inline KCL::uint32 GetParticleDataStride()
		{
			//NOTE: this value should just be the sizeof the input struct (ParticleData), but this is what kishonti does so i'm duplicating it for now
			static_assert(sizeof(ParticleData) == s_particle_stride, "Particle data must match struct size!");
			return s_particle_stride;
		}
		
		void SimulateSubStep();
		
		float GetRandomSeed() { return float(m_random_seed % 128); } //will index a rnd 1D texture in the advect shader
		KCL::uint32 GetMaxParticleCount() { return m_max_particle_count; }
		void GetStartTransform(KCL::Matrix4x4& startTform);
		KCL::uint32 GetNumSubsteps() {return m_numSubsteps;}
		KCL::Vector3D GetTotalAccel();
		
		~TF_emitter();
		
		inline static const id <MTLBuffer> GetBillboardGeometryBuffer()
		{
			return s_billboardDataBuffer;
		}
		
	//protected:
		TF_emitter( const std::string &name, KCL::ObjectType type, Node *parent, Object *owner);
		
		static void InitBillboardDataBuffer();
		static void ReleaseBillboardDataBuffer()
		{
			releaseObj(s_billboardDataBuffer);
		}
		
	private:
		
		//this is the number of floating point values in a row per (instanced input)
		//THIS MUST MUST MUST be the same as the shader
		//MUST match this struct in mtl_instanced_particle_layout.h
		/*looks like this:
		 float4 in_Pos;
		 float4 in_Age01_Speed_Accel;
		 float4 in_Amplitude;
		 float4 in_Phase;
		 float4 in_Frequency;
		 float4 in_T;
		 float4 in_B;
		 float4 in_N;
		 float4 in_Velocity;
		 
		 = 36 floats
		 
		 //The original GFXBench layout was this:
			 float3 in_Pos;
			 float3 in_Age01_Speed_Accel;
			 float3 in_Amplitude;
			 float3 in_Phase;
			 float3 in_Frequency;
			 float3 in_T;
			 float3 in_B;
			 float3 in_N;
			 float4 in_Velocity;
		 
			== 28 floats
		 */
		static const int kBufferCount = 2;
		
		//double buffered
		id <MTLBuffer> m_particleDataBuffer[kBufferCount];
		
		id <MTLBuffer> m_readBuffer;
		id <MTLBuffer> m_writeBuffer;
		
		static id <MTLBuffer> s_billboardDataBuffer; //must be the same across all emitters!
		
        float* m_InitializeData;
		
		KCL::int32 m_startBirthIdx;
		KCL::int32 m_endBirthIdx;
		KCL::int32 m_noOverflow;
		
		KCL::uint32 m_numSubsteps;
		float m_actual_rate;
        
        id <MTLDevice> m_Device ;
	};
    
    class MTLAnimatedEmitterFactory : public KCL::AnimatedEmitterFactory
    {
    public:
        virtual KCL::AnimatedEmitter *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
    };
}

#endif /* defined(__GFXBench30__mtl_particlesystem__) */
