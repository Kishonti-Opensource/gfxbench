/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

VertexShaderOutput main(VertexShaderInput input)
{
	float4 tmp;
	float4 pos;
#ifdef UBYTE_NORMAL_TANGENT
	float3 normal = input.normal * 2 - 1;
	float3 tangent = input.tangent * 2 - 1;
#else
	float3 normal = input.normal;
	float3 tangent = input.tangent;
#endif 
    VertexShaderOutput output;
	pos = float4(input.position, 1.0f) ;
    pos = mul(pos, mvp);

    output.position = pos;
    output.coord0 = output.position.xy;
    output.coord1 = output.position.zw;
    output.texcoord0 = input.texcoord0;
	
#if defined TRANSLATE_UV
	output.texcoord0 += translate_uv;
#endif
	
#if defined FOG
	float4 fog_position = mul( pos, mv);
	output.fog_distance = clamp( -fog_position.z * fog_density, 0.0f, 1.0f);
#endif

    return output;
}
