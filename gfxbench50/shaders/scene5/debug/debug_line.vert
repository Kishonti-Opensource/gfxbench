/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp;
uniform float4 pos0;
uniform float4 pos1;
uniform float4 color0;
uniform float4 color1;

in float3 in_position;
out float4 vertex_color;
void main()
{
	float3 pos = mix(pos0.xyz, pos1.xyz, in_position.x);
	vertex_color = mix(color0, color1, in_position.x);
	gl_Position = vp * float4(pos, 1.0);
}
