/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 mvp;

in float3 in_position;
in float2 in_texcoord0_;

out float2 v_texcoord0;


void main() 
{
	float3 position = makeBillboardPosition(in_position);
	gl_Position = mvp * float4( position, 1);

	v_texcoord0 = in_texcoord0_;
}

