/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_emitter.hlsli"

ParticleVSOutput main(ParticleRenderData input)
{
	ParticleVSOutput output;
	float3 up; 
	float3 right;
		
	up = float3( emitter_mv[0][1], emitter_mv[1][1], emitter_mv[2][1]);
	right = float3( emitter_mv[0][0], emitter_mv[1][0], emitter_mv[2][0]);

	float size =  lerp(emitter_begin_size, emitter_end_size, clamp(input.InstanceAge, 0.0, 1.0));
	float3 my_position = input.InstancePos + size * (input.VertexPos.x * right + input.VertexPos.y * up);

	output.position = mul(float4(my_position, 1.0), emitter_mvp);
	output.texcoord0 = float3(input.VertexTexCoord, input.InstanceAge);
	output.visibility = 1.0 - input.InstanceAge;

	return output;
}