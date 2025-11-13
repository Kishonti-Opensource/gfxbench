/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define mat4 float4x4
#define vec4 float4
#define vec3 float3
#define vec2 float2

#endif

uniform mat4 mvp;

in vec3 in_position;
in vec2 in_texcoord0_;

out vec2 texcoord0;
void main()
{
	vec4 pos = vec4(in_position, 1.0);
	gl_Position = mvp * pos;

	gl_Position.z = gl_Position.w;

	texcoord0 = in_texcoord0_;
}
