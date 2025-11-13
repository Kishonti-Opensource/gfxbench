/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"
		
ScreenVSOutput main(VertexShaderInput In)
{
	ScreenVSOutput Out;
	float2 size = In.position.xy;
	size.y *= screen_resolution.x / screen_resolution.y;
	
	float4 pos4 = mul(light_pos, mvp);
	pos4.xy /= pos4.w;

	// lerp(pos, -pos, z) = pos + (-pos - pos) * z = pos * (1 - 2*z)
	float2 pos2 = (1 - 2 * In.position.z) * pos4.xy;

	Out.texcoord0 = In.texcoord0;
	Out.position = float4(size.xy + pos2.xy, 0.0, 1);
	return Out;
}