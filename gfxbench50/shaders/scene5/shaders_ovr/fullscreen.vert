/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 mvp;
uniform float4x4 model;

in float2 in_position;

in float2 in_texcoord0_;

out float2 texcoord;
out float3 world_position;


void main()
{
	float4 p = float4( in_position, 0.0, 1.0);

	world_position = (model * p).xyz;
	texcoord = in_texcoord0_;

	gl_Position = p;
}
