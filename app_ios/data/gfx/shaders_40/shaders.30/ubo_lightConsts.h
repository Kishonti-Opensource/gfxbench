/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform lightConsts
{
	uniform lowp 		vec4 light_colorXYZ_pad;	
	uniform highp		vec4 light_posXYZ_pad;	
	uniform lowp 		vec4 light_x;
	uniform mediump		vec4 spotcosXY_attenZ_pad;	
};

/*
//for tiled lighting
struct lightInfo
{
	vec3 light_color;
	float fpad0;
	
	vec3 light_position;
	float fpad1;
	
	vec4 light_x;
	
	vec2 spot_cos;	
	float attenuation_parameter;
	float fpad2;
};

layout(std140) uniform lightConsts
{
	lightInfo lightData[32];	
	int num_lights;
};
*/