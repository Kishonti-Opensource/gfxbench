/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp;
uniform float4 view_pos;

uniform float4 center;
uniform float4 scale;

in float3 in_position;

out float3 normal;
out float3 view_dir;
void main()
{
	float3 position = in_position * scale.xyz + center.xyz;

	normal = in_position;
	view_dir = position - view_pos.xyz;
	gl_Position = vp * float4(position, 1.0);
}
