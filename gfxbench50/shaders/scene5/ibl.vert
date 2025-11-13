/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4 view_pos;
uniform float4 view_dir;
uniform float4x4 vp_inv;

in float2 in_position;
in float2 in_texcoord0_;

out float2 out_texcoord;
out float4 out_view_dir;
void main()
{
	float4 in_pos = float4(in_position, 1.0, 1.0);
	gl_Position = in_pos;

	float4 pos = vp_inv * in_pos;
	pos /= pos.w;
	out_view_dir.xyz = pos.xyz - view_pos.xyz;
	out_view_dir.w = dot(view_dir.xyz, out_view_dir.xyz);

	out_texcoord = in_texcoord0_;
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	out_texcoord.y = 1.0-out_texcoord.y;
#endif	
}
