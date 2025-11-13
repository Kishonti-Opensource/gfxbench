/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState SampleType;

float4 main(particleStruct input) : SV_TARGET
{
	float3 texel = texture0.Sample(SampleType, input.texcoord0).xyz;
	float3 texel2 = texture1.Sample(SampleType, input.texcoord1).xyz;
	
	float4 ret;

	ret.xyz = saturate(texel.xyz * 4.0 + 0.25) * texel2.xyz;
    ret.w = texel.x - ( 1.0 - pow(abs(input.life), 0.1));

	return ret;
}
