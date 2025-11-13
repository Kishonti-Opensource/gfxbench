/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
SamplerState SampleType;

#include "../include/types.hlsl"

float3 EncodeFloatRGB( float v ) 
{
	float3 enc = float3(1.0, 255.0, 65025.0) * v;
	enc = frac( enc);
	//enc -= enc.yzz * float3( 1.0/255.0, 1.0/255.0, 0.0);
	enc.xy -= enc.yz * float2( 1.0/255.0, 1.0/255.0);
	return enc;
}


float4 main(VertexShaderOutput input) : SV_TARGET
{
#if defined RGB_ENCODED
	float d = input.texcoord1.x / input.texcoord1.y;
	
	//d = d * 0.5 + 0.5;	
	
	return float4( EncodeFloatRGB( d), 1.0);

#else
	return float4(0.3f, 0.3f, 0.3, 0.3f);
#endif
}
