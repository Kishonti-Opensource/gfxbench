/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"

Texture2D texture0 : register(t0);
SamplerState SampleType;

float4 main(ScreenVSOutput input) : SV_TARGET
{   
	float3 c2 = texture0.Sample(SampleType, input.texcoord0, 15.0).xyz;
	float l2 = 0.0;//dot( c2, float3( 0.2126 , 0.7152 , 0.0722 ));

	float4 cc = texture0.Sample(SampleType, input.texcoord0);
	float3 c = texture0.Sample(SampleType, input.texcoord0).xyz;
	float l = dot( c, float3( 0.2126 , 0.7152 , 0.0722 )) - l2;
	
	l = clamp( cc.w + pow(l, 4.0) * 128.0, 0.0, 1.0);
	c = lerp(float3( 0.0, 0.0, 0.0), c, l);

	return float4( c, 1.0);
}