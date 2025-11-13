/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp_inv;
uniform float4 view_pos;
uniform float4 view_dir;

in float2 in_position;
in float2 in_texcoord0_;

out float2 texcoord;
out float4 out_view_dir;

void main()
{
	float4 p = float4( in_position, 0.0, 1.0);
	
	float4 pos = vp_inv * p;
	pos /= pos.w;
	out_view_dir.xyz = pos.xyz - view_pos.xyz;
	out_view_dir.w = dot( view_dir.xyz, out_view_dir.xyz);
	
	gl_Position = p;
	texcoord = in_texcoord0_;
}
