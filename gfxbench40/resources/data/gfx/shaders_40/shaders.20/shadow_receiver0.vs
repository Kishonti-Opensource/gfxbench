/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 model;
uniform mat4 shadow_matrix0;
#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
#endif

attribute vec3 in_position;
attribute vec4 in_bone_index;
attribute vec4 in_bone_weight;

varying vec3 out_texcoord0;

void main()
{    
	vec3 position;
	vec4 tmp;
	
#ifdef SKELETAL
	mat4 M4;
	
#define RESULT M4
#define BONE bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }

	position = (vec4( in_position, 1.0) * M4).xyz;
#else
	position = in_position;
#endif
	
	gl_Position = mvp * vec4( position, 1.0);
		
	vec4 world_position = model * vec4( position, 1.0);
	
	vec4 shadow_texcoord = shadow_matrix0 * world_position;
	
	out_texcoord0 = shadow_texcoord.xyz / shadow_texcoord.w;
}
