/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D shadow_unit : register(t10);

SamplerState SampleTypeClamp : register(s1);

#include "../include/types.hlsl"

#include "../include/functions.hlsl"

float4 main(VertexShaderOutput input) : SV_TARGET
{	  
	float4 color = float4(1.0, 1.0, 1.0, 1.0);
	float2 tc = input.texcoord0;
	//tc.y = 1.0f - tc.y;

#if defined RGB_ENCODED
	float color0 = 0.333;
#else
	float color0 = shadow_unit.Sample(SampleTypeClamp, tc).x;
#endif

  
	if (input.texcoord1.x < 1.0 && input.texcoord1.x > 0.0)
	{
		color = float4(color0, color0, color0, 1.0f);
	}
  
	return color;

}
