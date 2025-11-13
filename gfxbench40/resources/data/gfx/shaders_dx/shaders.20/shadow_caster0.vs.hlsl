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

#ifdef SKELETAL
		uint4 I = input.bone_index;
		float4 W = input.bone_weight;
		float4x3 B43 = bones[I.x] * W.x + bones[I.y] * W.y + bones[I.z] * W.z + bones[I.w] * W.w;

		pos     = float4( mul( float4( input.position, 1.0f) , B43 ), 1.0f );
#else
		pos = float4(input.position, 1.0f) ;
#endif

    VertexShaderOutput output;
    pos = mul(pos, mvp);

    output.position = pos;
    output.texcoord0 = input.texcoord0;

#if defined RGB_ENCODED
	output.texcoord1.x = pos.z;
	output.texcoord1.y = pos.w;
#endif

    return output;
}
