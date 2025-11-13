/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INSTANCED_LIGHT_RENDERER_H
#define INSTANCED_LIGHT_RENDERER_H

#include "kcl_base.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_instance_manager.h"
#include "glb_kcl_adapter.h"

class GLB_Scene_ES2_;
struct PP;
struct LightBufferObject;
class Shader;

class InstancedLightRenderer
{
	// Note: This should never exceed GL_MAX_UNIFORM_BLOCK_SIZE / sizeof(lightInstancingStruct)
	// Keep in sync with "ubo_lightInstancingConsts.h" shader!
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

	void Draw( KCL::Camera2 *camera, const PP &pp, LightBufferObject *m_lbos, Shader *m_lighting_shaders[16]);

	KCL::uint32 GetLightningInstanceDataVBO() const;
    
    InstancedLightRenderer();
	~InstancedLightRenderer();
private:
    KCL::uint32 m_lightning_lights_instance_data_vbo;

    GLB::InstanceManager<_instanced_light> *m_instance_manager;
};
#endif
