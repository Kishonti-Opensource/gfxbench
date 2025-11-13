/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 mvp;
uniform float4x4 vp;
uniform float4x4 model;
uniform float4 view_pos;
uniform float4 view_dir;
uniform float4 flip_vector;

in float3 in_position;

out float4 world_position;
out float4 projected_pos;
out float4 out_view_dir;
out float inside;

void main()
{	
	inside = 1.0;

	world_position = model * float4( in_position, 1.0);
	//div by w if cone
	world_position /= world_position.w;
	
	//world_position = mvp * world_position;
	
	//ray to surface of cone
	out_view_dir.xyz = world_position.xyz - view_pos.xyz;

	//normalize
	out_view_dir.w = dot( view_dir.xyz, out_view_dir.xyz);
	
	projected_pos = vp * world_position;
	
	gl_Position = projected_pos;

#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	projected_pos.y *= -1.0;
#endif	
}
