/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);

SamplerState SampleType;

#include "../include/types.hlsl"
#include "../include/functions.hlsl"

float4 main(MBlurVertexShaderOutput input) : SV_TARGET
{
	float2 a = (input.curr_pos.xy / input.curr_pos.w);
	float2 b = (input.prev_pos.xy / input.prev_pos.w);

	float2 diff = a - b;
	float l = length( diff);
	
	if( l > 0.0f)
	{
		diff.x /= 0.0666667;
		diff.y /= 0.125;
		diff += 0.5;
	}
	else
	{
		diff = float2( 0.5f,  0.5f);
	}

	return float4( diff, 0.0, 0.0);

}
