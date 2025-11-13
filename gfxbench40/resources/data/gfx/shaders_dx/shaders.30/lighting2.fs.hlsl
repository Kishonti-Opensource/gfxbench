/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"
#include "../include/types.hlsl"

Texture2D colorTexture : register(t0);	// color (albedo)
Texture2D normalTexture : register(t1);	// normal
Texture2D envTexture : register(t2);	// environment
Texture2D paramsTexture : register(t3);	// params
Texture2D depthTexture : register(t4);	// depth
Texture2D shadowMap : register(t5);

SamplerState SampleType;

float3 get_world_pos(float2 screenCoord, float3 viewDir) 
{
	float depth = depthTexture.Sample(SampleType, screenCoord).r;
	float linearDepth = depth_parameters.y / (depth - depth_parameters.x);
	return linearDepth * viewDir + view_pos.xyz;
}

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}