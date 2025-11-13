/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"
#include "../include/types.hlsl"

Texture2D texture_unit0 : register(t0);	// color (albedo)
Texture2D texture_unit1 : register(t1);	// normal
Texture2D texture_unit2 : register(t2);	// environment
Texture2D texture_unit3 : register(t3);	// params
Texture2D texture_unit4 : register(t4);	// depth
Texture2D shadowMap : register(t5);

SamplerState SampleType;

float3 get_world_pos(float2 texcoord, float4 viewDir) 
{
	float depth = texture_unit4.Sample(SampleType, texcoord).r;
	float linearDepth = depth_parameters.y / (depth - depth_parameters.x);
	return linearDepth * viewDir.xyz / viewDir.w + view_pos.xyz;
}

float4 main(LightVSOutput input) : SV_TARGET
{
	float3 viewDir = normalize(input.view_dir.xyz);
	float2 screenCoord = input.screen_coord.xy / input.screen_coord.w  * 0.5 + 0.5;
	screenCoord.y = 1.0 - screenCoord.y;

#if defined POINT_LIGHT
	float3 worldPosition = get_world_pos(screenCoord, input.view_dir);
	float3 light_dir = light_pos.xyz - worldPosition;
	float sq_distance = dot(light_dir, light_dir);
	float atten = attenuation_parameter * sq_distance + 1.0;
	if( atten <= 0.0)
	{
		discard;
	}

	atten = clamp(atten, 0.0, 1.0);
	light_dir /= sqrt(sq_distance);

#elif defined SPOT_LIGHT
	float3 worldPosition = get_world_pos(screenCoord, input.view_dir);
	float3 light_dir = light_pos.xyz - worldPosition;
	float sq_distance = dot(light_dir, light_dir);
	float atten = attenuation_parameter * sq_distance + 1.0;	
	if (atten <= 0.0)
	{
		discard;
	}

	atten = clamp(atten, 0.0, 1.0);
	light_dir /= sqrt(sq_distance);
	float fov_atten = dot(light_dir, -light_x.xyz) - spot_cos.x;	
	if (fov_atten <= 0.0)
	{
		discard;
	}

	fov_atten *= spot_cos.y;
	atten *= fov_atten;

#elif defined SHADOW_MAP
	float3 worldPosition = get_world_pos(screenCoord, input.view_dir);
	float3 light_dir = light_x.xyz;
	float atten = 1.0;

#else
	float3 light_dir = light_x.xyz;
	float atten = 1.0;
#endif

#ifdef SHADOW_MAP
	float4 shadow_texcoord = mul(shadow_matrix0, float4(worldPosition, 1));
	shadow_texcoord = shadow_texcoord / shadow_texcoord.w;
	float shadow = shadowMap.Sample(SampleType, shadow_texcoord.xy).x;
	if (shadow_texcoord.z < 1.0 && 
		shadow_texcoord.z > 0.0 &&
		shadow_texcoord.z > shadow)
	{
		discard;
	}
	
	atten *= shadow;
#endif

	float4 texel_color = texture_unit0.Sample(SampleType, screenCoord);
	float4 float_params = texture_unit3.Sample(SampleType, screenCoord);
	float3 normal = texture_unit1.Sample(SampleType, screenCoord).xyz;
	normal = normalize(normal * 2.0 - 1.0);

	float3 half_vector = normalize(light_dir - viewDir);
	float diffuse = dot(normal, light_dir);	

#ifdef SPECIAL_DIFFUSE_CLAMP
	diffuse = diffuse * 0.5 + 0.5;
#else
	diffuse = clamp(diffuse, 0.0, 1.0);
#endif
	diffuse *= 3.0;

	float specular = dot(normal.xyz, half_vector);
	specular = clamp(specular, 0.0, 1.0);

	float A = vector2ToFloat(float_params.rg) * 128;
	float B = vector2ToFloat(float_params.ba) * 4096;

	specular = A * pow( specular, B);

	float3 c = (texel_color.xyz * diffuse + specular) * light_color.xyz * atten;
	return float4(c, 1.0);
}