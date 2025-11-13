/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPUTE_LIGHTNING
#define COMPUTE_LIGHTNING

#include <vector>
#include "kcl_base.h"
#include "kcl_math3d.h"
#include "kcl_actor.h"
#include "kcl_camera2.h"
#include "opengl/glb_texture.h"
#include "opengl/glb_shader2.h"

class GLB_Scene_ES2_;

class ComputeLightning
{
public:
	static const KCL::uint32 LIGHTNING_COUNT = 9;	
	static const KCL::uint32 LIGHTNING_BUFFER_SIZE = 1024;
	static const KCL::uint32 RENDER_BUFFER_SIZE = LIGHTNING_BUFFER_SIZE * 6 * LIGHTNING_COUNT + 128;

	ComputeLightning();
	~ComputeLightning();

	void Init(KCL::uint32 lights_buffer, GLB_Scene_ES2_ *scene);
	bool IsInited()
	{
		return status == KCL::KCL_TESTERROR_NOERROR;
	}

	void Run(float animation_time, KCL::Actor *focus);
    void Draw(KCL::Camera2 *camera);
	
	KCL::uint32 GetLightCount();
private:
	std::string m_data_prefix;

	KCL::uint32 LoadEndpoints(const char *filename);
	GLB::GLBShader2 *LoadShader(const char *filename);
	void GetBonesForActor(KCL::Node * node);
	
	float m_lightning_header[LIGHTNING_COUNT];
	KCL::uint32 m_lightning_count;

	KCL::KCL_Status status;
	std::vector<KCL::Vector4D> m_endpoints;
	KCL::uint32 m_ground_endoint_offset;
	KCL::uint32 m_sky_endpoint_offset;
	
	GLB::GLBTexture *m_noise_texture;
	
	std::vector<KCL::Vector4D> m_bone_segments;
	
	KCL::uint32 m_lights_buffer;

	KCL::uint32 m_vao;

	KCL::uint32 m_render_buffer;
	KCL::uint32 m_lightning_buffer;
	KCL::uint32 m_endpoint_buffer;
	
	GLB::GLBShader2 *m_shader_pass1;
	GLB::GLBShader2 *m_shader_pass2;
	GLB::GLBShader2 *m_shader_render;
    //for statistics
    GLB_Scene_ES2_ *m_scene;
};

#endif