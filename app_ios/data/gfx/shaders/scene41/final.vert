/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define vec4 float4
#define vec2 float2
#define mat4 float4x4

#endif

uniform vec4 view_pos;
uniform vec4 view_dir;
uniform mat4 vp_inv;

in vec2 in_position;
in vec2 in_texcoord0_;

out vec4 out_view_dir;
out vec2 texcoord;
void main()
{
	vec4 in_pos = vec4( in_position, 0.0, 1.0);

	vec4 pos = vp_inv * in_pos;
	pos /= pos.w;
	out_view_dir.xyz = pos.xyz - view_pos.xyz;
	out_view_dir.w = dot( view_dir.xyz, out_view_dir.xyz);

	gl_Position = in_pos;
	texcoord = in_texcoord0_;
}
