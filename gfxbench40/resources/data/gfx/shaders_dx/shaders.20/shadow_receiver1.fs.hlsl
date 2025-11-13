/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture10 : register(t10);

SamplerState SampleTypeClamp : register(s1);

#include "../include/types.hlsl"

#include "../include/functions.hlsl"



float DecodeFloatRGB( float3 rgba ) 
{
	return dot( rgba, float3(1.0, 1.0/255.0, 1.0/65025.0) );
}

float shadow( float4 tc)
{
	//tc /= tc.w;

#if defined RGB_ENCODED
	float color0 = DecodeFloatRGB( texture10.Sample( SampleTypeClamp, float2( tc.x, tc.y) ).xyz);
#else
	float color0 = 0.0;
#endif

	float ret = 1.0;
	if( tc.z < 1.0 && tc.z > 0.0 && color0 < tc.z)
	{
		ret = 0.333;
	}
	return ret;
}

float4 main(VertexShaderOutput input) : SV_TARGET
{	  
	float4 tc = float4(input.texcoord0.x, input.texcoord0.y, input.texcoord1.x, 1.0);
	float color0 = shadow(tc);
	color0 = 0.5;
	float4 color = float4(color0, color0, color0, 1.0f);
	
	return color;

}
