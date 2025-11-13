/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp;
uniform float4x4 mvp;
uniform float4 center;
uniform float4 scale;

in float3 in_position;
in float2 in_texcoord0_;

out float3 position;
out float2 tex_coord;
void main()
{
	position = in_position * scale.xyz + center.xyz;
	tex_coord = in_texcoord0_;

	gl_Position = mvp * float4(position, 1.0);
}
