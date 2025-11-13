/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
#endif

attribute vec3 in_position;
attribute vec4 in_bone_index;
attribute vec4 in_bone_weight;
	
//varying vec2 depth;



void main()
{    
	vec3 position;

#ifdef SKELETAL
	mat4 M4;

#define RESULT M4
#define BONE bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }

	position = (vec4( in_position, 1.0) * M4).xyz;
#else
	position = in_position;
#endif


	gl_Position = mvp * vec4( position, 1);
	
#if defined RGB_ENCODED
	//depth.x = gl_Position.z;
	//depth.y = gl_Position.w;
#endif

}
