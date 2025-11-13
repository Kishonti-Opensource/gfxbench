/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
SamplerState SampleType: register(s0);
SamplerState SampleTypeClamp: register(s1);

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);

#include "../include/types.hlsl"

float4 main(ScreenVSOutput input) : SV_TARGET
{
	const int n = 4;
	float3 motion = texture1.Sample(SampleTypeClamp, input.texcoord0).xyz; 
	float4 texel = texture0.Sample(SampleTypeClamp, input.texcoord0);
	float3 color = texel.xyz / float( n); 

	motion.xy = (motion.xy - 0.5) * float2( 0.0666667,.125);
	motion.xy *= texel.a;
	
	for( int i=1; i<n; i++)
	{
		float2 tc = input.texcoord0 - motion.xy * float( i);
		color += texture0.Sample(SampleTypeClamp, tc).xyz / float( n); 
	}

	return float4(color, 1.0);
}
