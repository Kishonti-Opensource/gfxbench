/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

Texture2D texture_unitWorldDepth : register(t0);
Texture2D texture_unitShadowDepth : register(t1);
SamplerState SampleType : register(s0);
//SamplerState SampleType2 : register(s1);
SamplerComparisonState SampleCompareType : register(s1);

float3 get_world_pos(float2 texcoord, float4 viewDir) 
{
	float depth = texture_unitWorldDepth.Sample(SampleType, texcoord).r;
	float linearDepth = depth_parameters.y / (depth - depth_parameters.x);
	return linearDepth * viewDir.xyz / viewDir.w + view_pos.xyz;
}

float4 main(LightVSOutput input) : SV_TARGET
{
	float2 screenCoord = input.screen_coord.xy / input.screen_coord.w  * 0.5 + 0.5;
	screenCoord.y = 1 - screenCoord.y;

	float3 position = get_world_pos(screenCoord, input.view_dir);

	float4 shadow_texcoord = mul(float4( position, 1.0), shadow_matrix0);
	shadow_texcoord.xyz /= shadow_texcoord.w;
	
	float bias = 0.00001;
	shadow_texcoord.z = saturate(shadow_texcoord.z) - bias;

	//shadow = texture_unitShadowDepth.Sample(SampleType, screenCoord);

	//float depth = texture_unitWorldDepth.Sample(SampleType, texcoord).r;
	
	//return shadow.xxxx;	

	//shadow = texture_unitShadowDepth.SampleCmp(SampleCompareType, shadow_texcoord.xy / shadow_texcoord.w, shadow_texcoord.z);

	//return lerp(float4(0.25, 0.25, 0.25, 1.0), float4(1.0, 1.0, 1.0, 1.0), shadow);	
	
	
	float shadow = 1.0;
	
	shadow = texture_unitShadowDepth.SampleCmp(SampleCompareType, shadow_texcoord.xy, shadow_texcoord.z);
	
	return lerp(0.25,1.0,shadow);

	//return texture_unitShadowDepth.Sample(SampleType, shadow_texcoord.xy / shadow_texcoord.w).xxxx;
/*	

	float2 svt = shadow_texcoord.xy / shadow_texcoord.w;
		
	float2 test = float2(0.5,1.5);

	//float shadowDepth = texture_unitShadowDepth.Sample(SampleType2, shadow_texcoord.xy / shadow_texcoord.w);
	float shadowDepth = texture_unitShadowDepth.Sample(SampleType2, shadow_texcoord.xy / shadow_texcoord.w);
	//return (abs(shadowDepth - 0.5) - 0.5) * 50.0;
	
	//return shadow_texcoord.z / shadow_texcoord.w - 6.0;
	
	if(shadowDepth > saturate(shadow_texcoord.z / shadow_texcoord.w) - 0.00001)
		return 1.0;
	else
		return 0.0;
*/
	
//	return float4(1,0,0,1);

}
