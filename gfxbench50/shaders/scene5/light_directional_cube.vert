/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4 view_pos;
uniform float4 view_dir;
uniform float4x4 mvp;
uniform float4x4 model;

in float3 in_position;
//in float2 in_texcoord0_;

out float4 out_view_dir;
out float4 out_pos;
//out float2 out_texcoord;
void main()
{
	float4 in_pos = model *  float4( in_position, 1.0);
	float4 pos = mvp * in_pos;

	gl_Position = pos;

	out_pos = pos;
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	out_pos.y *= -1.0;
#endif

	pos = model * float4( in_position, 1.0);
	out_view_dir.xyz = pos.xyz - view_pos.xyz;
	out_view_dir.w = dot( view_dir.xyz, out_view_dir.xyz);
}