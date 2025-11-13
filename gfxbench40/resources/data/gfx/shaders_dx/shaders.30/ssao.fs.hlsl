/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"

Texture2D texture1 : register(t0);
Texture2D texture3 : register(t1);
SamplerState SampleType;

#define SAMPLE_KERNEL_SIZE	8

float get_depth(float2 tc)
{
	float d = texture3.Sample(SampleType, tc).x;
	return depth_parameters.y / (d - depth_parameters.x);
}


float4 main(float4 position : SV_POSITION,	float3 viewDir : VIEW_DIR) : SV_TARGET
{
	float2 offsets2[SAMPLE_KERNEL_SIZE];
	float px = inv_screen_resolution.x;
	float py = inv_screen_resolution.x;

	offsets2[0] = float2( px,  0.0);
	offsets2[1] = float2(-px,  0.0);
	offsets2[2] = float2( 0.0,  py);
	offsets2[3] = float2( 0.0, -py);
	offsets2[4] = float2( px,  py);
	offsets2[5] = float2(-px, -py);
	offsets2[6] = float2(-px,  py);
	offsets2[7] = float2( px, -py);

	float d = get_depth( position.xy);
	float tcRandom = rand1(position.xy);

	float occlusion = 0.0;
	for (int j = 1; j <= 32; j+= 4) 
	{
		for (int i=0; i < SAMPLE_KERNEL_SIZE; i++)
		{
			float radius = tcRandom * j;
			float2 sample_coord = position.xy + offsets2[i].xy * radius;

			float sample_depth = get_depth(sample_coord);
			float sample_diff = (sample_depth - d) / d;
			
			if ((sample_diff > -1.0) &&
				(sample_diff < -0.004 * tcRandom))
			{
				occlusion += 1.0 + sample_diff;
			}
		}  
	}

	occlusion /= SAMPLE_KERNEL_SIZE * 8;	
	occlusion = 1.0 - occlusion;
	return  pow(occlusion, 4.0) * 4.0;
}