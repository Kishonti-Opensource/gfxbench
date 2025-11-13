/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

Texture2D texture_unit0 : register(t0);
Texture2D texture_unit1 : register(t1);	
Texture2D texture_unit2 : register(t2);
Texture2D texture_unit3 : register(t3);
Texture2D texture_unit4 : register(t4);

SamplerState SampleType;

float4 main(ScreenVSOutput input) : SV_TARGET
{
	float4 color = texture_unit0.Sample(SampleType, input.texcoord0);
	float4 normal = texture_unit1.Sample(SampleType, input.texcoord0);
	float4 light = texture_unit3.Sample(SampleType, input.texcoord0);
	float4 reflection = texture_unit2.Sample(SampleType, input.texcoord0);

	float3 ambiLightCol = float3(0.0/255.0, 30.0/255.0, 50.0/255.0);

	light.xyz += color.xyz * ambiLightCol;
	light = lerp(light, reflection, reflection.a);
	light = lerp(light, color, normal.a);
	
	return float4(light.xyz, normal.a);
}
