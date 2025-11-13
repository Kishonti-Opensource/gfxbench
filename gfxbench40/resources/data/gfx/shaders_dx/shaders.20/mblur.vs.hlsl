/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_mblur.hlsli"

MBlurVertexShaderOutput main(VertexShaderInput input)
{
	float4 tmp;

	float4 curr_pos;	
	float4 prev_pos;	

    MBlurVertexShaderOutput output;

#ifdef SKELETAL
		uint4 I = input.bone_index;
		float4 W = input.bone_weight;

		float4x3 B43curr = bones[I.x] * W.x + bones[I.y] * W.y + bones[I.z] * W.z + bones[I.w] * W.w;
		float4x3 B43prev = prev_bones[I.x] * W.x + prev_bones[I.y] * W.y + prev_bones[I.z] * W.z + prev_bones[I.w] * W.w;

		curr_pos= float4( mul( float4( input.position, 1.0f) , B43curr ), 1.0f );
		prev_pos= float4( mul( float4( input.position, 1.0f) , B43prev ), 1.0f );
#else
		curr_pos = float4(input.position, 1.0f) ;
		prev_pos = float4(input.position, 1.0f) ;
#endif

	output.curr_pos = mul(curr_pos, mvp);
    output.prev_pos = mul(prev_pos, mvp2);
	
    output.position = output.curr_pos;

    return output;
}