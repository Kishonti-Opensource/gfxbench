/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct lightInstancingStruct
{
	highp mat4 model;
	highp vec4 light_color;
	highp vec4 light_pos;
	highp vec4 attenuation_parameter;
	highp vec4 light_x;
	highp vec4 spot_cos; 
};


layout(std140) uniform lightInstancingConsts
{	
	// Note: This should never exceed GL_MAX_UNIFORM_BLOCK_SIZE / sizeof(lightInstancingStruct)
	uniform lightInstancingStruct lights[18];
};

uniform mediump int instance_offset;