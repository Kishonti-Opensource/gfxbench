/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_opengl_state_manager.h"


namespace GLB
{
	unsigned int OpenGLStateManager::m_capabilities_table[max_CAPABILITY_IDX] =
	{
		GL_BLEND,
		GL_CULL_FACE,
		GL_DEPTH_TEST,
		GL_DITHER,
		GL_POLYGON_OFFSET_FILL,
		GL_SAMPLE_ALPHA_TO_COVERAGE,
		GL_SAMPLE_COVERAGE,
		GL_SCISSOR_TEST,
		GL_STENCIL_TEST
	};


	bool OpenGLStateManager::m_actual_capabilities[max_CAPABILITY_IDX] =
	{
		false,
		false,
		false,
		true ,
		false,
		false,
		false,
		false,
		false
	};


	bool OpenGLStateManager::m_desired_capabilities[max_CAPABILITY_IDX][2] =
	{
		{false, false},
		{false, false},
		{false, false},
		{true , true },
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false}
	};
	
	
	bool OpenGLStateManager::m_actual_enabled_vertex_attrib_arrays[m_vertex_attrib_arrays_count] =
	{
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false
	};


	bool OpenGLStateManager::m_desired_enabled_vertex_attrib_arrays[m_vertex_attrib_arrays_count][2] =
	{
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false},
		{false, false}
	};
	
	
	unsigned int OpenGLStateManager::m_actual_texture_unit[2] = {GL_TEXTURE0, GL_TEXTURE0};
	
	unsigned int OpenGLStateManager::m_actual_blendfunc_sfactor = GL_ONE;
	unsigned int OpenGLStateManager::m_actual_blendfunc_dfactor = GL_ZERO;
	unsigned int OpenGLStateManager::m_desired_blendfunc_sfactor[2] = {GL_ONE, GL_ONE};
	unsigned int OpenGLStateManager::m_desired_blendfunc_dfactor[2] = {GL_ZERO, GL_ZERO};
	
	unsigned int OpenGLStateManager::m_actual_cullface_mode = GL_BACK;
	unsigned int OpenGLStateManager::m_desired_cullface_mode[2] = {GL_BACK, GL_BACK};
	
	unsigned int OpenGLStateManager::m_actual_depth_func = GL_LESS;
	unsigned int OpenGLStateManager::m_desired_depth_func[2] = {GL_LESS, GL_LESS};
	
	unsigned char OpenGLStateManager::m_actual_depth_mask[2] = {GL_TRUE, GL_TRUE};

	unsigned int OpenGLStateManager::m_actual_shader_program[2] = {0, 0};
}
