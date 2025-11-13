/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp;
uniform float4x4 model;
uniform float4 view_pos;
uniform float4 view_dir;
uniform float4 vertex_array[LIGHTSHAFT_MAX_VERTICES];

out float4 world_position;
out float4 projected_pos;
out float4 out_view_dir;
void main()
{
	world_position = model * vertex_array[gl_VertexID];

	out_view_dir.xyz = world_position.xyz - view_pos.xyz;

	out_view_dir.w = dot( view_dir.xyz, out_view_dir.xyz);

	projected_pos = vp * world_position;

	projected_pos.z += 0.0001;
	
	gl_Position = projected_pos;
	
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	projected_pos.y *= -1.0;
#endif	
}
