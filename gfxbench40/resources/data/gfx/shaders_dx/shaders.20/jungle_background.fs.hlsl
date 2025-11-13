/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);
Texture2D texture2 : register(t2);

SamplerState SampleType;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"

float4 main(VertexShaderOutput input) : SV_TARGET
{
#ifdef MASK
	float4 mask  = texture2.Sample(SampleType, input.texcoord0);

#else
	const float4 mask = float4( 1.0f, 1.0f, 1.0f, 1.0f);
#endif

#ifdef ALPHA_TEST
	if ( mask.x < 0.6f ) 
		discard;
#endif
	
	float3 color  = (float3)texture0.Sample(SampleType, input.texcoord0);

#if defined FOG
	color = lerp( color, (float3)background_color, input.fog_distance * input.fog_distance);
#endif	
		
#if defined TRANSPARENCY
	float trp = mask.x * transparency;
	return float4( color, trp);
#else
	return float4( color, 1.0f);
#endif
}
