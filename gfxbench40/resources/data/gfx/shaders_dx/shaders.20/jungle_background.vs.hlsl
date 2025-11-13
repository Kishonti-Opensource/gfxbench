/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

VertexShaderOutput main(VertexShaderInput input)
{
	float4 pos;
    VertexShaderOutput output;
	
	pos = float4(input.position, 1.0f) ;

    output.position = mul(pos, mvp);
	output.position.z = output.position.w;
    output.texcoord0 = input.texcoord0;
	
#if defined FOG
	float4 fog_position = mul( pos, mv);
	output.fog_distance = clamp( -fog_position.z * fog_density, 0.0f, 1.0f);
#endif
	
    return output;
}
