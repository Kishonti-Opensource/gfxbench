/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_ALU2_H
#define GL_ALU2_H

#include "alu2_base.h"

#include "kcl_base.h"
#include "kcl_math3d.h"

#include "opengl/glb_shader2.h"

#include <vector>

class ALUTest2 : public ALUTest2_Base
{
public:
	ALUTest2(const GlobalTestEnvironment* const gte);
	virtual ~ALUTest2();
	
protected:
	virtual KCL::KCL_Status init ();
	virtual bool render ();
	
	virtual void FreeResources();

private:
	KCL::uint32 m_LightsVao;
    KCL::uint32 m_EmissiveVao;
	KCL::uint32 m_vertex_buffer;
	KCL::uint32 m_index_buffer;

	KCL::uint32 m_color_texture;
	KCL::uint32 m_normal_texture;
	KCL::uint32 m_depth_texture;
	KCL::uint32 m_sampler;

	GLB::GLBShader2 *m_shaderLights;
    GLB::GLBShader2 *m_shaderAmbEmit;
	KCL::uint32 m_uniform_light_pos_array;
    KCL::uint32 m_uniform_light_color_array;

	KCL::uint32 LoadTexture(const char * filename);
	void CreateLights(const float * depth_data);

	void SaveScene();
};

#endif