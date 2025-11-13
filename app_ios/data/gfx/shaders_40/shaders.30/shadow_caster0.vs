/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include meshConsts;
#else
	uniform mat4 mvp;
#endif

#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
#endif 

in vec3 in_position;
in vec4 in_bone_index;
in vec4 in_bone_weight;
	
//out vec2 depth;

void main()
{    
	vec3 position;

#ifdef SKELETAL
	ivec4 I = ivec4(in_bone_index);
	mat3 B30 = mat3( 
		bones[I.x * 3 + 0].xyz,
		bones[I.x * 3 + 1].xyz,
		bones[I.x * 3 + 2].xyz);
	mat3 B31 = mat3( 
		bones[I.y * 3 + 0].xyz,
		bones[I.y * 3 + 1].xyz,
		bones[I.y * 3 + 2].xyz);
	mat3 B32 = mat3( 
		bones[I.z * 3 + 0].xyz,
		bones[I.z * 3 + 1].xyz,
		bones[I.z * 3 + 2].xyz);
	mat3 B33 = mat3( 
		bones[I.w * 3 + 0].xyz,
		bones[I.w * 3 + 1].xyz,
		bones[I.w * 3 + 2].xyz);
	
	vec3 T0 = vec3( 
		bones[I.x * 3 + 0].w,
		bones[I.x * 3 + 1].w,
		bones[I.x * 3 + 2].w);
	vec3 T1 = vec3( 
		bones[I.y * 3 + 0].w,
		bones[I.y * 3 + 1].w,
		bones[I.y * 3 + 2].w);
	vec3 T2 = vec3( 
		bones[I.z * 3 + 0].w,
		bones[I.z * 3 + 1].w,
		bones[I.z * 3 + 2].w);
	vec3 T3 = vec3( 
		bones[I.w * 3 + 0].w,
		bones[I.w * 3 + 1].w,
		bones[I.w * 3 + 2].w);
	
	mat3 B3 = B30 * in_bone_weight.x + B31 * in_bone_weight.y + B32 * in_bone_weight.z + B33 * in_bone_weight.w;
	vec3 T = T0 * in_bone_weight.x + T1 * in_bone_weight.y + T2 * in_bone_weight.z + T3 * in_bone_weight.w;
	position = B3 * in_position + T;
#else
	position = in_position;
#endif


	//gl_Position = mvp * vec4( position, 1);
	mat4 mvp_1 = mvp;
	gl_Position = mvp_1 * vec4( position, 1);
	
#if defined RGB_ENCODED
	//depth.x = gl_Position.z;
	//depth.y = gl_Position.w;
#endif

}
