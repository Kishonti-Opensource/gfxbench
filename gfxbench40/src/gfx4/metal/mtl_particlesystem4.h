/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once
#include "platform.h"
#include "kcl_math3d.h"
#include "memory"
#include "kcl_node.h"
#include "kcl_particlesystem2.h"
#include "jsonserializer.h"
#include "graphics/metalgraphicscontext.h"

class WarmUpHelper;

namespace MetalRender
{
    class Pipeline;
    class DynamicDataBuffer;
    class DynamicDataBufferPool;
    class Texture;
};


namespace MetalRender
{
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
        
        ComputeEmitter( KCL::uint32 num_max_particles, bool single_frame, id<MTLDevice> device);
        ~ComputeEmitter();
        
        void Animate( float timestep, KCL::uint32 anim_time);
        bool Emit(id<MTLCommandBuffer> command_buffer);
        bool Simulate(id<MTLCommandBuffer> command_buffer);
        KCL::KCL_Status Init(WarmUpHelper *warm_up, DynamicDataBufferPool* bufferPool);
        void Render(id<MTLRenderCommandEncoder> encoder, Pipeline *billboard_point_gs, const KCL::Matrix4x4  &mvp, const KCL::Matrix4x4 &view_matrix, const KCL::Vector3D &eye, MetalRender::Texture* textures[2], id<MTLSamplerState> topdown_sampler, KCL::Vector3D &eye_forward);
        
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

		static void ResetParticleSystemSeed();

		void SaveState(KCL::uint32 anim_time);
		KCL::KCL_Status LoadState(KCL::uint32 anim_time);
		void SynchronizeParticleBuffer(id <MTLCommandQueue> command_queue);

        static MTLVertexDescriptor* GetVertexDescriptor()
        {
            static MTLVertexDescriptor* m_particle_vertex_desc;
            if(m_particle_vertex_desc == nil)
            {
                m_particle_vertex_desc = [[MTLVertexDescriptor alloc] init];
                
                m_particle_vertex_desc.attributes[0].format = MTLVertexFormatFloat3;
                m_particle_vertex_desc.attributes[0].offset = offsetof( _particle, Pos);
                m_particle_vertex_desc.attributes[0].bufferIndex = 0;
                
                m_particle_vertex_desc.attributes[1].format = MTLVertexFormatFloat2;
                m_particle_vertex_desc.attributes[1].offset = offsetof( _particle, Age_Speed_Accel_Active);
                m_particle_vertex_desc.attributes[1].bufferIndex = 0;
                
                m_particle_vertex_desc.layouts[0].stepRate = 1;
                m_particle_vertex_desc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
                m_particle_vertex_desc.layouts[0].stride = sizeof(_particle);
            }
            return m_particle_vertex_desc;
        };
        
    private:
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
        
        id<MTLDevice> m_device;
        
        KCL::uint32 m_num_max_particles;

        DynamicDataBuffer * m_particle_dynamic_buffer;
        id<MTLBuffer> m_particle_indices_buffer[2];
        id<MTLBuffer> m_particle_vertices_buffer;
        id<MTLBuffer> m_temp_particle_vertices_buffer;

        float m_emit_count;
        float m_simulation_count;
        float m_last_particle_age;
        _emitter m_emitter;
        
        MetalRender::Pipeline* m_particle_emit_pipeline;
        MetalRender::Pipeline* m_particle_simulate_pipeline;
        
        KCL::KCL_Status LoadEmitShader(KCL::uint32 wg_size);
        KCL::KCL_Status LoadSimulateShader(KCL::uint32 wg_size);

		KCL::KCL_Status WarmupEmitShader(WarmUpHelper *warm_up);
		KCL::KCL_Status WarmupSimulateShader(WarmUpHelper *warm_up);

		KCL::KCL_Status InitEmitWarmUp();
		KCL::KCL_Status InitSimulateWarmUp();

		KCL::KCL_Status LoadStateByFilename(const char * filename);
        
        KCL::uint32 m_emit_wg_size;
        KCL::uint32 m_simulate_wg_size;
        
        
        bool m_single_frame;
        _emitter m_single_frame_emitter;
        KCL::Vector3D m_single_frame_movement;
        
        KCL::Matrix4x4 m_current_pom;
        KCL::Matrix4x4 m_prev_pom;
        KCL::Vector3D m_movement;
        friend class GFXGui;
        
        // particle system state load save
        template <bool write, typename T > void EmitterStreamOperation(T & sstream);
        template <bool write, typename T > void ParticleStreamOperation(_particle & particle, T & sstream);
        template <bool write, typename T > void StateOperation(T & sstream);
    };
};
