/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include emitterRenderConsts;
#else
	uniform mat4 mvp;
	uniform mat4 mv;
	uniform vec4 emitter_maxlifeX_sizeYZ_pad;
#endif

layout (location = 0) in vec3 in_instance_position;
layout (location = 1) in vec3 in_Age01_speed_accel;

layout (location = 9) in vec4 in_pos_uv;

out vec4 out_UVxyz_Visibility;
	
void main()
{
	vec3 up; 
	vec3 right;
	vec3 fwd;	
	
	//billboard
	up = vec3( mv[0][1], mv[1][1], mv[2][1]);
	right = vec3( mv[0][0], mv[1][0], mv[2][0]);
	
	float size = mix(emitter_maxlifeX_sizeYZ_pad.y, emitter_maxlifeX_sizeYZ_pad.z, clamp(in_Age01_speed_accel.x,0.0,1.0));
	
	vec3 my_position = in_instance_position + size * in_pos_uv.x * right + size * in_pos_uv.y * up;

	gl_Position = mvp * vec4( my_position, 1.0);
	
	out_UVxyz_Visibility.xyz = vec3( in_pos_uv.zw, in_Age01_speed_accel.x);
	out_UVxyz_Visibility.w = 1.0 - in_Age01_speed_accel.x;
}
