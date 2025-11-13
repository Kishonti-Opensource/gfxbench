/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 mvp;
uniform float fire_time;

uniform float emissive_intensity;

in float3 in_position;
in float2 in_texcoord0_;

out float2 v_texcoord0;
out float2 v_texcoord1;
out float2 v_texcoord2;
out float2 v_texcoord3;


void main()
{
	// parameters
	const float4 scales       = float4(0.5,1.0,1.5,1.0);
	const float4 scrollSpeeds = float4(1.3*0.8,2.1*0.8,2.3*0.8,1.0);

	float3 position = makeBillboardPosition(in_position);

	position.y *= emissive_intensity;
	position.y -= (1.0-emissive_intensity)*0.4;

	gl_Position = mvp * float4( position, 1);

	v_texcoord0 = in_texcoord0_;

	v_texcoord1 = v_texcoord0 * scales.x;
	v_texcoord1.y += fire_time * scrollSpeeds.x;
	v_texcoord2 = v_texcoord0 * scales.y;
	v_texcoord2.y += fire_time * scrollSpeeds.y;
	v_texcoord3 = v_texcoord0 * scales.z;
	v_texcoord3.y += fire_time * scrollSpeeds.z;
}

