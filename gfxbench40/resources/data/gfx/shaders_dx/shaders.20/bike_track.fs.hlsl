/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);

SamplerState SampleType;
SamplerState SampleTypeClamp;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"


float4 main(VertexShaderOutput input) : SV_TARGET
{
	float3 color = texture0.Sample(SampleType, input.texcoord0).xyz;
	
	if( (1.0 - input.texcoord1.y) > alpha_threshold)
	{
		color = float3( 0.5, 0.5, 0.5 );
	}

	return float4( color, 1.0);
}
