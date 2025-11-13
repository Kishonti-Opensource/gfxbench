/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INSTANCED_LIGHT_RENDERER_H
#define INSTANCED_LIGHT_RENDERER_H

#include "kcl_base.h"
#include "mtl_lightbuffer.h"
#include "mtl_pipeline.h"
#include "mtl_dynamic_data_buffer.h"


class MTLInstancedLightRenderer
{
	static const KCL::uint32 MAX_LIGHTS = 18;
public:
	struct _instanced_light
    {
		KCL::Matrix4x4 model;
        //xyz - color, w - unused
        KCL::Vector4D color;
        //xyz - position, w - unused
        KCL::Vector4D position;
		//x - radius, y - atten, z - spotx, w - spoty
        KCL::Vector4D atten_parameters;
        //xyz - dir, w - unused
        KCL::Vector4D dir;
        KCL::Vector4D spot;
    };

    KCL::uint32 m_num_omni_lights;
    _instanced_light m_omni_lights[64];

	KCL::uint32 m_num_spot_lights;
    _instanced_light m_spot_lights[64];
	
	KCL::uint32 m_num_lightning_lights;

	void Init();	

    void Draw(id <MTLRenderCommandEncoder> render_encoder, KCL::Camera2 *camera, MetalRender::LightBuffer **m_lbos, MetalRender::Pipeline **m_lighting_pipelines);

	id <MTLBuffer> GetLightningInstanceDataVBO() const;
    
    MTLInstancedLightRenderer(id <MTLDevice> device, MetalRender::DynamicDataBuffer* data_buffer);
	~MTLInstancedLightRenderer();
private:
    id <MTLBuffer> m_lightning_lights_instance_data_vbo;

    id <MTLDepthStencilState> m_lightDepthState;
    
    id <MTLDevice> m_Device;
    
    MetalRender::DynamicDataBuffer *m_data_buffer ;
};
#endif
