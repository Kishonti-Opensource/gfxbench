/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __MTL_COMPUTE_LIGHTNING_H__
#define __MTL_COMPUTE_LIGHTNING_H__

#include <vector>
#include "kcl_base.h"
#include "kcl_math3d.h"
#include "kcl_actor.h"
#include "kcl_camera2.h"
#include "mtl_texture.h"
#include "mtl_pipeline.h"
#include "mtl_dynamic_data_buffer.h"

class MTLComputeLightning
{
public:
	static const KCL::uint32 LIGHTNING_COUNT = 9;	
	static const KCL::uint32 LIGHTNING_BUFFER_SIZE = 1024;
	static const KCL::uint32 RENDER_BUFFER_SIZE = LIGHTNING_BUFFER_SIZE * 6 * LIGHTNING_COUNT + 128;

	MTLComputeLightning(id <MTLDevice> device);
	~MTLComputeLightning();

    void Init(id <MTLBuffer> lights_buffer, MetalRender::ShaderType main_shader_type);
	bool IsInited()
	{
		return status == KCL::KCL_TESTERROR_NOERROR;
	}

    void RunPass1(float animation_time, KCL::Actor *focus, id <MTLCommandBuffer> command_buffer, MetalRender::DynamicDataBuffer* ddb);
    void RunPass2(KCL::Camera2 *camera, id <MTLCommandBuffer> command_buffer, MetalRender::DynamicDataBuffer* ddb) ;
    void Draw(KCL::Camera2 *camera, id <MTLRenderCommandEncoder> render_encoder);
	
	KCL::uint32 GetLightCount();
private:
	std::string m_data_prefix;

	KCL::uint32 LoadEndpoints(const char *filename);
    MetalRender::Pipeline *LoadShader(const char *filename, bool force_highp, MetalRender::ShaderType shader_type);
	void GetBonesForActor(KCL::Node * node);
	
	KCL::uint32 m_lightning_count;

	KCL::KCL_Status status;
	std::vector<KCL::Vector4D> m_endpoints;
	KCL::uint32 m_ground_endoint_offset;
	KCL::uint32 m_sky_endpoint_offset;
	
	MetalRender::Texture *m_noise_texture;
	
	std::vector<KCL::Vector4D> m_bone_segments;
	
	id <MTLBuffer> m_lights_buffer;

	id <MTLBuffer> m_render_buffer;
	id <MTLBuffer> m_lightning_buffer;
	id <MTLBuffer> m_endpoint_buffer;
	
    id <MTLDepthStencilState> m_lightningDepthStencilState;
    
	MetalRender::Pipeline *m_shader_pass1;
	MetalRender::Pipeline *m_shader_pass2;
	MetalRender::Pipeline *m_shader_render;
    
    id <MTLDevice> m_device ;
};

#endif // __MTL_COMPUTE_LIGHTNING_H__

