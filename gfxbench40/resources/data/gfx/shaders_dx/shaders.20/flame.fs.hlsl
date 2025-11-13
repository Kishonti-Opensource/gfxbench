/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);

SamplerState SampleType;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

float4 main(VertexShaderOutput input) : SV_TARGET
{	
	float2 noise = (float2)texture1.Sample(SampleType, float2( input.texcoord0.x, input.texcoord0.y + 100.0f * time));
	
	noise.x = noise.x * 2.0 - 1.0;
	
	float2 flameCoord;

	flameCoord.x = input.texcoord0.x + noise.x * (1.0f / 8.0f);
	flameCoord.y = input.texcoord0.y + noise.y * (1.0f / 32.0f);

	float4 c = texture0.Sample(SampleType, flameCoord);
	return c * float4( (float3)color, 1.0f);
}
