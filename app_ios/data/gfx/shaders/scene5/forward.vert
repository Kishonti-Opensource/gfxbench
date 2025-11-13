/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 vp;
uniform mat4 model;
#ifdef SKELETAL
uniform vec4 bones[MAX_BONES];
#endif

in vec3 in_position;
in vec2 in_texcoord0_;
in vec3 in_normal;
in vec3 in_tangent;
#ifdef SKELETAL
in vec4 in_bone_index;
in vec4 in_bone_weight;
#endif

out vec2 texcoord;
out vec3 world_position;
out vec3 normal;
out vec3 tangent;


void main()
{
	vec4 p = vec4( in_position, 1.0);
	vec4 n = vec4( in_normal, 0.0);
	vec4 t = vec4( in_tangent, 0.0);

#ifdef SKELETAL
	mat4 M4;
	{
		#define RESULT M4
		#define BONE bones
		ivec4 I = ivec4(in_bone_index);
		mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0));
		mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0));
		mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0));
		mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	world_position = (p * M4).xyz;
	normal = (n * M4).xyz;
	tangent = (t * M4).xyz;
#else
	world_position = (model * p).xyz;
	normal = (model * n).xyz;
	tangent = (model * t).xyz;
#endif
	texcoord = in_texcoord0_;

	gl_Position = vp * vec4(world_position, 1.0);
}
