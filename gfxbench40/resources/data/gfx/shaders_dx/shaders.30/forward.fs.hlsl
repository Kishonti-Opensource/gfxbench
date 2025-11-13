/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"
#include "../include/types.hlsl"

Texture2D diffuseTexture : register(t0);
Texture2D emissionTexture : register(t1);
Texture2D maskTexture : register(t2);
Texture2D normalMap : register(t3);
TextureCube envmap0 : register(t8);
TextureCube envmap1 : register(t9);
Texture2DArray textureArray : register(t10);

SamplerState SampleType;

float4 main(VertexShaderOutput input) : SV_TARGET
{
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	float3 bitangent = cross(tangent, normal);

	float3 texel_color = diffuseTexture.Sample(SampleType, input.texcoord0).xyz;
	float3 ts_normal = normalMap.Sample(SampleType, input.texcoord0).xyz;
#ifdef MASK
	float4 mask = maskTexture.Sample(SampleType, input.texcoord0);
#else
	float4 mask = float4( 1.0, 0.0, 0.0, 1.0);
#endif

#ifdef EMISSION
	float4 emission = emissionTexture.Sample(SampleType, input.texcoord0);
#endif

#if defined ALPHA_TEST
	if( mask.x < 0.25)
	{
		discard;
	}
#endif

	ts_normal.xyz = ts_normal.xyz * 2.0 - 1.0;
		
	float3x3 mat = float3x3( tangent, bitangent, normal); 

	normal = mul(ts_normal, mat);
	
	float3 reflect_vector = reflect(input.view_dir, normal);
	float3 env_color0 = envmap0.Sample(SampleType, reflect_vector).xyz;
	float3 env_color1 = envmap1.Sample(SampleType, reflect_vector).xyz;

	float3 env_color = lerp( env_color1.xyz, env_color0.xyz, envmaps_interpolator);
	
	float3 color = lerp(texel_color, env_color, reflect_intensity * mask.z); 
	
#ifdef EMISSION
	//color = emission.x * 8.0;
#endif

	return float4(color, mask.x * transparency);
	//frag_color = float4( color, transparency);
	//frag_color = float4( color, 0.5);
}
