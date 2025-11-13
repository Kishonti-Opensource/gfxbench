/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_instancedlightrenderer.h"
#include "platform.h"

#include "kcl_camera2.h"
#include "krl_scene.h"
#include "render_statistics_defines.h"
#include "metal/shader.h"
#include "mtl_scene_30.h"


#define INSTANCED_LIGHTS_CONSTS_BFR_SLOT 13


MTLInstancedLightRenderer::MTLInstancedLightRenderer(id <MTLDevice> device, MetalRender::DynamicDataBuffer *data_buffer)
{
    m_Device = device ;
    m_data_buffer = data_buffer ;
    
    m_num_omni_lights = 0;
    m_num_spot_lights = 0;
	m_num_lightning_lights = 0;
    m_lightning_lights_instance_data_vbo = 0;
}


MTLInstancedLightRenderer::~MTLInstancedLightRenderer()
{
    releaseObj(m_lightning_lights_instance_data_vbo) ;
}


void MTLInstancedLightRenderer::Init()
{
#if TARGET_OS_EMBEDDED
    m_lightning_lights_instance_data_vbo = [m_Device newBufferWithLength:MAX_LIGHTS * sizeof(_instanced_light)
                                                                 options:MTLResourceOptionCPUCacheModeDefault];
#else
    m_lightning_lights_instance_data_vbo = [m_Device newBufferWithLength:MAX_LIGHTS * sizeof(_instanced_light)
                                                                 options:MTLResourceStorageModeManaged];
#endif
    
    m_num_omni_lights = 0;
	m_num_spot_lights = 0;
	m_num_lightning_lights = 0;
    
    
    MTLDepthStencilDescriptor *lightDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];

#define DEPTH_TEST_LIGHTS
    
#ifdef DEPTH_TEST_LIGHTS
    {
        //GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
        //GLB::OpenGLStateManager::GlDepthFunc( GL_GEQUAL);
        //GLB::OpenGLStateManager::GlDepthMask(0);
        
        lightDepthStateDesc.depthWriteEnabled = NO;
        lightDepthStateDesc.depthCompareFunction = MTLCompareFunctionGreaterEqual;
    }
#else
    {
        //GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
        
        lightDepthStateDesc.depthWriteEnabled = NO;
        lightDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    }
#endif
    
    m_lightDepthState = [m_Device newDepthStencilStateWithDescriptor:lightDepthStateDesc];
}


id <MTLBuffer> MTLInstancedLightRenderer::GetLightningInstanceDataVBO() const
{
	return m_lightning_lights_instance_data_vbo;
}


void MTLInstancedLightRenderer::Draw(id <MTLRenderCommandEncoder> render_encoder, KCL::Camera2 *camera, MetalRender::LightBuffer **m_lbos, MetalRender::Pipeline **m_lighting_pipelines)
{
	int light_shader_index = 3;
	int light_buffer_index = -1;
	int num_lights = 0;

    //  set by the lighting pipelines
	//  GLB::OpenGLStateManager::GlBlendFunc( 1, 1);
	//  GLB::OpenGLStateManager::GlEnable( GL_BLEND);
    
    //  GLB::OpenGLStateManager::GlCullFace( GL_FRONT);
    [render_encoder setDepthStencilState:m_lightDepthState];
    [render_encoder setCullMode:MTLCullModeFront];

    bool lightning_lights;
	for( int light_type=0; light_type<3; light_type++)
	{
        lightning_lights = false;
        if( light_type == MTL_Scene_30::LIGHT_POINT)
		{
            //omni
            if (!m_num_omni_lights)
            {
                continue;
            }
            		
            light_shader_index = MTL_Scene_30::LIGHT_POINT;
            light_buffer_index = MetalRender::LightBuffer::SPHERE;
			num_lights = m_num_omni_lights;

            m_data_buffer->WriteAndSetData<true, true>(render_encoder, INSTANCED_LIGHTS_CONSTS_BFR_SLOT, &m_omni_lights[0], m_num_omni_lights*sizeof(_instanced_light)) ;
		}
        else if( light_type == MTL_Scene_30::LIGHT_SPOT)
		{
            //spot
            if (!m_num_spot_lights)
            {
                continue;
            }
            			
            light_shader_index = MTL_Scene_30::LIGHT_SPOT;
            light_buffer_index = MetalRender::LightBuffer::CONE;
			num_lights = m_num_spot_lights;
            
            m_data_buffer->WriteAndSetData<true, true>(render_encoder, INSTANCED_LIGHTS_CONSTS_BFR_SLOT, &m_spot_lights[0], m_num_spot_lights*sizeof(_instanced_light)) ;
		}
		else
		{
            //lightning omni
            if (!m_num_lightning_lights)
            {
                continue;
            }
            			
			light_shader_index = MTL_Scene_30::LIGHT_POINT;
			light_buffer_index = MetalRender::LightBuffer::SPHERE;
			num_lights = m_num_lightning_lights;
            lightning_lights = true;
            
            [render_encoder setVertexBuffer:  m_lightning_lights_instance_data_vbo offset:0 atIndex:INSTANCED_LIGHTS_CONSTS_BFR_SLOT] ;
            [render_encoder setFragmentBuffer:m_lightning_lights_instance_data_vbo offset:0 atIndex:INSTANCED_LIGHTS_CONSTS_BFR_SLOT] ;
		}
        
        m_lighting_pipelines[light_shader_index]->Set(render_encoder) ;

        if (num_lights > 0)
        {
            m_lbos[light_buffer_index]->DrawInstanced(render_encoder,num_lights) ;
        }
	}

}

