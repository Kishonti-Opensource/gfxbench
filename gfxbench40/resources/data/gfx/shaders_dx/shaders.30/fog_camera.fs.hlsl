/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

Texture2D texture_unit0 : register(t0);
Texture2D texture_unit1 : register(t1);
Texture3D texture_3D_unit0 : register(t2);

SamplerState SampleType;

float4 main(FogStruct input) : SV_TARGET
{
	////float t = time * 8.0;
	////
	////float2 depth_texcoord = input.position.xy;
	////float d = texture_unit0.Sample(SampleType, depth_texcoord).x;
	////float z_view_depth = depth_parameters.y / (d - depth_parameters.x);
	////float depthDiffScale = 0.1f;

	////float3 texcoord0 = input.shadow_texcoord.xyz / input.shadow_texcoord.w;
	////
	////float world_atten = clamp((35.0f - input.world_pos.y) / 5.0f, 0.0f, 1.0f);

	////float shadow_depth = texture_3D_unit0.Sample(SampleType, float3(texcoord0.xy, input.scrollspeed_depth.x)).x;
	////float atten = texture_3D_unit0.Sample(SampleType, float3(input.world_pos.xz / 120.0f + t / 10.0f, input.world_pos.y / 20.0f - t)).x * world_atten * 5.0f;

	////float2 offset = float2(-112.0f, -98.0f);
	////float scale = 1.0f / 408.8f;
	////float2 volColUV = (input.world_pos.zx - offset) * float2(scale, scale);
	////float3 volumeColor = texture_unit1.Sample(SampleType, volColUV ).xyz * 2.0f * clamp((15.0f - input.world_pos.y) / 5.0f, 0.0f, 1.0f);;
	////
	////// frag_color = 0.2 * float4(light_color.xyz + volumeColor, atten)
	////
	////float fogValue = texture_3D_unit0.Sample(SampleType, float3(input.world_pos.xz / 120.0f + t / 10.0f, input.world_pos.y / 10.0f - t)).x;
	
	float fogColor = 0.4f;
	return float4(fogColor, fogColor, fogColor, 1.0); // fogValue * world_atten * 0.6f);
}

