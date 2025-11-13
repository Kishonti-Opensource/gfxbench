/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_30.h"
#include "mtl_particlesystem.h"
#include "mtl_pipeline.h"
#include "mtl_texture.h"
#include "mtl_dynamic_data_buffer.h"
#include "mtl_material.h"

//#define DEBUG_DUMP 1
#define PARTICLE_DEBUG

void dumpParticleBuffer(id <MTLBuffer> buffer)
{
	float* data = reinterpret_cast<float*>([buffer contents]);
	for(int i = 0; i < /*emitter->VisibleParticleCount()*/ 1000; i++)
	{
		if(data[i*MetalRender::TF_emitter::sParticleDataFloatCount+0] > 0.0 &&
			data[i*MetalRender::TF_emitter::sParticleDataFloatCount+0] < 10000.0)
		{
			__builtin_printf("p %u ", i);
			for(int j = 0; j < MetalRender::TF_emitter::sParticleDataFloatCount; j++)
			{
				__builtin_printf("%f ", data[i*MetalRender::TF_emitter::sParticleDataFloatCount+j]);
			}
			
			__builtin_printf("\n");
		}
	}
}

void MTL_Scene_30::MoveParticles(id <MTLCommandBuffer> commandBuffer)
{
	assert(m_pointSampler != nil);
	assert(m_noiseTexture != nil);
	assert(m_TransformFeedbackFramebuffer);
	
	id <MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:m_TransformFeedbackFramebuffer];
	[encoder setDepthStencilState:m_DepthTestOffDepthWritesOff];
	
	//this data will not change no matter what's being emitted
	m_particleAdvect_pipeline->Set(encoder);
	
	MetalRender::Texture* noiseTex = static_cast<MetalRender::Texture*>(m_noiseTexture);
	
	noiseTex->Set<true, false>(encoder, 0);
	[encoder setVertexSamplerState:m_pointSampler atIndex:0];
	
	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		KCL::Actor* actor = m_actors[i];
		
		
		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			//TODO: possible optimization: don't do any of this if no particles are visible!
			
			MetalRender::TF_emitter *emitter = static_cast<MetalRender::TF_emitter*>(actor->m_emitters[i]);
			
			EmitterAdvectConsts consts;
			emitter->GetEmitterParams(&consts);
			
			const unsigned int numSubsteps = emitter->GetNumSubsteps();
			
			for( unsigned int substep = 0; substep < numSubsteps; substep++ )
			{
				emitter->SimulateSubStep();
				
				//emitter->GetBufferData( subStepData[ substep * 4 ], subStepData[ substep * 4 + 1 ], subStepData[ substep * 4 + 2 ], dummy );
				emitter->GetBufferData(consts.particleBufferParamsXYZ_pad[substep]);
				//TODO: necessary to reset this?
				consts.particleBufferParamsXYZ_pad[substep].w = 0;
			}
			
			consts.emitter_numSubstepsX_pad_pad_pad.x = numSubsteps;
			
			//the emitters keep two buffers for data
			//Frame 0:
			//A is read from
			//B is written (while reading from A, using transform feedback/stream out)
			//B is rendered
			//Next frame:
			//B is read from
			//A is written
			//A is rendered
			//and so on
			id <MTLBuffer> source = emitter->GetReadBuffer();
			id <MTLBuffer> dest = emitter->GetWriteBuffer();
			
			//only worried about the vertex shader
			[encoder setVertexBuffer:source offset:0 atIndex:XFB_PARTICLE_SOURCE_DATA_INDEX];
			[encoder setVertexBuffer:dest offset:0 atIndex:XFB_PARTICLE_DEST_DATA_INDEX];
			
			m_dynamicDataBuffer->WriteAndSetData<true, false>(encoder, EMITTER_ADVECT_CONST_INDEX, &consts, sizeof(EmitterAdvectConsts));
			
			//render out data as points
			if(emitter->VisibleParticleCount() > 0)
			{
				//NOTE: the debug layer blows up if you try to submit a draw with 0 verts, so we're skipping this now.
				[encoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:emitter->VisibleParticleCount() instanceCount:1];
			}
			
			//swap this emitter's buffers to prepare for later rendering
			emitter->SwapBuffers();
			
		} //loop over emitters
	} //loop over actors
	
	[encoder endEncoding];
}

void MTL_Scene_30::RenderParticles(id <MTLRenderCommandEncoder> encoder)
{
	[encoder setCullMode:MTLCullModeNone];
	[encoder setDepthStencilState:m_particleDepthStencilState];
	
	for( KCL::uint32 i=0; i<m_actors.size(); i++)
	{
		KCL::Actor *actor = m_actors[i];
		
		for( KCL::uint32 i=0; i<actor->m_emitters.size(); i++)
		{
			MetalRender::TF_emitter *emitter = static_cast<MetalRender::TF_emitter*>(actor->m_emitters[i]);
			
			EmitterAdvectConsts consts;
			emitter->GetEmitterParams(&consts);
			
			meshConsts mConsts;
			mConsts.mv = m_active_camera->GetView();
			mConsts.mvp = m_active_camera->GetViewProjection();
			//TODO: need to actually copy these?
			//mConsts.model = model;
			//mConsts.inv_model = inv_model;
			//mConsts.inv_modelview = inv_modelview;
			
			//TODO: verify this is the correct buffer
			//should have been switched earlier, in MoveParticles
			id <MTLBuffer> source = emitter->GetReadBuffer();
			//Render particles
			{
				
				//MTL_TODO don't do a string lookup
				MetalRender::Material *material = NULL;
				
				//These can never be hit
				if( emitter->m_name.find( "smoke") != std::string::npos)
				{
					material = dynamic_cast<MetalRender::Material*>(m_instanced_smoke_material);
				}
				if( emitter->m_name.find( "fire") != std::string::npos)
				{
					material = dynamic_cast<MetalRender::Material*>(m_instanced_fire_material);
				}
				if( emitter->m_name.find( "spark") != std::string::npos)
				{
					material = dynamic_cast<MetalRender::Material*>(m_instanced_spark_material);
				}
				 
				if( emitter->m_name.find( "soot") != std::string::npos)
				{
					material = dynamic_cast<MetalRender::Material*>(m_instanced_spark_material);
				}
				else
				{
					material = dynamic_cast<MetalRender::Material*>(m_instanced_fire_material);
				}
				
				material->SetPipelineAndTextures(encoder, 0, 0, m_default_shader_type);
				
				{
					MetalRender::Texture* fireTex = static_cast<MetalRender::Texture*>(m_fireTexture);
					fireTex->Set(encoder, TEXTURE_3D_SLOT);
					#ifdef TEXTURE_COUNTING
					m_textureCounter.insert( ((GLBTexture*)fire_texid)->getId());
					#endif
				}
				
				//bind billboard geometry
				[encoder setVertexBuffer:MetalRender::TF_emitter::GetBillboardGeometryBuffer() offset:0 atIndex:0];
				
				//instanced data that was fed back earlier
				[encoder setVertexBuffer:source offset:0 atIndex:1];
				[encoder setFragmentBuffer:source offset:0 atIndex:1];
				
				//bind constants
				//NOTE: frame consts don't need to be set
				m_dynamicDataBuffer->WriteAndSetData<true, true>(encoder, MESH_CONST_INDEX, &mConsts, sizeof(meshConsts));
				m_dynamicDataBuffer->WriteAndSetData<true, true>(encoder, EMITTER_ADVECT_CONST_INDEX, &consts, sizeof(EmitterAdvectConsts));
				
				if(emitter->VisibleParticleCount() > 0)
				{
					[encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4 instanceCount:emitter->VisibleParticleCount()];
					m_num_draw_calls++;
					m_num_triangles += 2 * (emitter->VisibleParticleCount());
					m_num_vertices += 6 * (emitter->VisibleParticleCount());
				}
			}
		}
	}
}