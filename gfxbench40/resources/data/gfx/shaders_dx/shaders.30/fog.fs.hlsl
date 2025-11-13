/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

Texture2D fogTexture : register(t0);
Texture2D depthTexture : register(t1);
SamplerState SampleType;

float4 main(FogStruct input) : SV_TARGET
{   
	float2 screenCoord = input.screen_coord.xy / input.screen_coord.w * 0.5 + 0.5;
	screenCoord.y = 1.0 - screenCoord.y;

	float z_depth = depthTexture.Sample(SampleType, screenCoord).r;
	float z_view_depth = depth_parameters.y / (z_depth - depth_parameters.x);

	float t = time * 8.0;
	float final_atten = 0.0;

	float last_fov_atten = -1000.0;
	float last_atten = -1000.0;

	float3 world_pos = input.world_pos;
	float3 light_dir = world_pos.xyz - light_pos.xyz;

	float3 ray_dir = world_pos.xyz - view_pos.xyz;
	float3 ray_step = normalize(ray_dir);
	
	float start_depth = dot(ray_dir, view_dir.xyz);

	float max_iter = (z_view_depth - start_depth);
	if (max_iter > 1024.0)
	{
		max_iter = 1024.0;
	}
	
	//[unroll(50)]
	float i;
	for (i = 0.0; i < max_iter; i += 1.0) // trace only distance to opaque object
	{
		float sq_distance = dot(light_dir, light_dir);
		float3 light_dir_norm = light_dir / sqrt(sq_distance);
		
		float atten = attenuation_parameter * sq_distance + 1.0;
		
		// check for exit due distance (exit via bottom of cone)
		if ((atten < 0.0) && (atten < last_atten))
		{
			break;
		}

		last_atten = atten; // if we're going inside cone - previous distance will be smaller vs attenuated distance (it's going from 0.0 to 1.0 near light source)
		atten = clamp(atten, 0.0, 1.0);

		float fov_atten = dot(light_dir_norm, light_x.xyz) - spot_cos.x;
		
		// check for exit due angle (exit via side of cone)
		if ((fov_atten < 0.0) && (fov_atten < last_fov_atten))
		{
			break;
		}

		last_fov_atten = fov_atten; // if we're going inside cone - previous cos() will be smaller vs new cos() value of angle
		
		fov_atten *= spot_cos.y;

		atten *= fov_atten;
		
		if (atten > 1.0/255.0) // if we're above threshold - modulate by noise texture
		{
			float4 shadow_texcoord_f = mul(float4(world_pos, 1.0), shadow_matrix0);
			float3 shadow_texcoord = shadow_texcoord_f.xyz / shadow_texcoord_f.w;

			float shadow_depth = fogTexture.SampleLevel(SampleType, shadow_texcoord.xy + float2(t, 0.0), 0.0).x;
			atten *= shadow_depth;
		}

		final_atten += atten * (1.0 - final_atten);

		// advance our positions by raymarching step
		world_pos += ray_step;
		light_dir += ray_step;
	}

	return float4(light_color.xyz, final_atten) * 0.5;
}
