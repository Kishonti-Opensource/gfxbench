/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 mvp;
uniform float4x4 model;
uniform float4 view_pos;
uniform float4 view_dir;

in float3 in_position;

out half4 out_view_dir;
out float4 out_pos;
void main()
{
	float4 in_pos = float4( in_position, 1.0);
	float4 pos = mvp * in_pos;

	gl_Position = pos;

	out_pos = pos;
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	out_pos.y *= -1.0;
#endif	

	pos = model * in_pos;
	out_view_dir.xyz = half3(pos.xyz - view_pos.xyz);
	out_view_dir.w = dot(half3(view_dir.xyz), out_view_dir.xyz);
}
