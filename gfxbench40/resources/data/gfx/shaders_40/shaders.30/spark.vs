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
layout (location = 8) in vec4 in_velocity;

layout (location = 9) in vec4 in_pos_uv;

out vec3 out_UVxy_Visibility;
	
void main()
{
	vec3 up; 
	vec3 right;
	vec3 fwd;	
	
	//axis aligned, camera facing quad
	up = in_velocity.xyz; 
	vec3 camFwd = vec3( mv[0][2], mv[1][2], mv[2][2]);
	right = cross(up, camFwd);

	float size = mix(emitter_maxlifeX_sizeYZ_pad.y, emitter_maxlifeX_sizeYZ_pad.z, clamp(in_Age01_speed_accel.x,0.0,1.0));
		
	float width = 0.3;
	float height = 0.3;
	vec3 my_position = in_instance_position + size * in_pos_uv.x * width * right + size * in_pos_uv.y * height * up;

	gl_Position = mvp * vec4( my_position, 1.0);
	
	out_UVxy_Visibility.xy = in_pos_uv.zw;
	out_UVxy_Visibility.z = 1.0 - in_Age01_speed_accel.x;
}