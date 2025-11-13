/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

Texture2D texture0 : register(t0);
Texture2D texture3 : register(t3);
SamplerState SampleType;

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit3;

float4 main(float4 screenPosition : SV_POSITION,
			float4 view_dir : VIEW_DIR) : SV_TARGET
{
	float2 out_texcoord = screenPosition.xy;
	float4 texel_color = texture0.Sample(SampleType, out_texcoord);
	float4 float_params = texture3.Sample(SampleType, out_texcoord);

	float d = float_params.r;
	float3 position = d * view_dir.xyz / view_dir.w + view_pos.xyz;

	float3 light_dir = light_pos.xyz - position;
	float sq_distance = dot(light_dir, light_dir);

	float atten = attenuation_parameter * sq_distance + 1.0;
	
	if( atten <= 0.0)
	{
		discard;
	}

	atten = clamp( atten, 0.0, 1.0);
	
	light_dir = light_dir / sqrt( sq_distance);

	float3 c = texel_color.xyz * light_color.xyz * atten;
	return float4( c, 1.0);	
}