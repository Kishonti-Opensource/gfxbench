/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture3 : register(t3);
Texture2D texture2 : register(t2);
Texture2D planar_reflection : register(t11);

SamplerState SampleType : register(s0);
SamplerState SampleTypeClamp : register(s1);

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"

float3 planar_reflection_calculate( in float2 texcoord0, in float3 screen_position )
{
	float3 reflection_color;
	
	float3 ts_normal = (float3)texture3.Sample(SampleType, texcoord0) * 0.04 - 0.02;
	
	float2 tc = (float2)screen_position + ts_normal.xy;
	 
	reflection_color = (float3)planar_reflection.Sample(SampleTypeClamp, tc);
	
	return reflection_color;
}

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float3 screen_position = screen_position_normalize(float4(input.coord0.x, input.coord0.y, input.coord1.x, input.coord1.y));

#ifdef MASK
	float4 mask = texture2.Sample(SampleType, input.texcoord0);
#else
	const float4 mask = float4( 1.0f, 1.0f, 1.0f, 1.0f);
#endif
	
#if defined ALPHA_TEST
	if( mask.x < 0.6f)
	{
		discard;
	}
#endif
	
	float3 color = planar_reflection_calculate( input.texcoord0, screen_position );

#ifdef FOG
	color = lerp( color, background_color, fog_distance * fog_distance);
#endif	

	
#ifdef TRANSPARENCY
	float trp = mask.z * transparency;
    return float4(color, trp);
#else
    return float4(color, 1.0f);
#endif
}
