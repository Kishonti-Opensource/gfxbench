/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define mat4 float4x4
#define vec3 float3
#define vec2 float2
#define vec4 float4

#endif

uniform mat4 mvp;
uniform mat4 prev_mvp;
uniform mat4 model;

in vec3 in_position;
in vec2 in_texcoord0_;
in vec3 in_normal;
in vec3 in_tangent;

out vec3 normal;
out vec3 tangent;
out vec2 texcoord0;
out vec4 clip_space_pos;
out vec4 clip_space_prev_pos;
void main()
{
	vec4 p = vec4( in_position, 1.0);
	vec4 n = vec4( in_normal, 0.0);
	vec4 t = vec4( in_tangent, 0.0);

	normal = (model * n).xyz;
	tangent = (model * t).xyz;
	texcoord0 = in_texcoord0_;

	clip_space_pos = mvp * p;
	gl_Position = clip_space_pos;

	clip_space_prev_pos = prev_mvp * p;
}
