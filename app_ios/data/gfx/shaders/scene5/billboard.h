/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_vertex

uniform float4x4 mv;
uniform float4x4 model;
uniform float4 view_pos;

float3 makeBillboardPosition(float3 in_position)
{
	float3 up;
	float3 right;
	float3 fwd;

#ifdef AALIGNED
	up = float3( 0.0, 1.0, 0.0);
#else
	up = float3( mv[0][1], mv[1][1], mv[2][1]);
#endif

	right = float3( mv[0][0], mv[1][0], mv[2][0]);

	float3 delta = float3( model[3][0], model[3][1], model[3][2]) - view_pos.xyz;
	delta = normalize( delta);

	float3 position = in_position.x * right + in_position.y * up + in_position.z * delta;
	return position;
}

#endif

