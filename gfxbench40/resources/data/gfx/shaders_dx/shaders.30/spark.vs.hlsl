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

	float3 up = input.Velocity;
	float3 camDir = float3( emitter_mv[0][2], emitter_mv[1][2], emitter_mv[2][2]);
	float3 right = cross(up, camDir);

	float size = lerp(emitter_begin_size, emitter_end_size, clamp(input.InstanceAge, 0.0, 1.0));

	float width = 0.3;
	float height = 0.3;
	float3 my_position = input.InstancePos + size * (input.VertexPos.x * width * right + input.VertexPos.y * height * up);

	output.position = mul(float4(my_position, 1.0), emitter_mvp);
	output.texcoord0 = float3(input.VertexTexCoord, 0.0);
	output.visibility = 1.0 - input.InstanceAge;

	return output;
}