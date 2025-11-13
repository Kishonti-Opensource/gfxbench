/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_particle.hlsli"

particleStruct main(VertexShaderInput input)
{  
    particleStruct output;

	float3 up; 
	float3 right;
	float3 fwd;

	up = float3( mv[0][1], mv[1][1], mv[2][1]);
	right = float3( mv[0][0], mv[1][0], mv[2][0]);

	int idx = int(input.position.x);
	
	float2 offset = input.texcoord0 * 2.0 - 1.0;

	offset *= 0.2+sqrt(particle_data[idx].w)*1.8;

	float3 position = particle_data[idx].xyz + offset.x * right + offset.y * up;

	output.position = mul(float4(position, 1.0f), mvp);
	
	float deltay = floor( particle_data[idx].w * 63.0);
	float deltax = floor( fmod( deltay, 8.0)) ;

	deltay = floor( deltay / 8.0);

	output.texcoord0 = float2( (deltax + input.texcoord0.x) / 8.0, (1 + deltay - input.texcoord0.y) / 8.0 );
	
	output.life = particle_data[idx].w;
	output.color = particle_color[idx];
	
	output.texcoord1 = mul(float4(position, 1.0f), world_fit).xy;

	return output;
}
