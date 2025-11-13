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
	float3 up; 
	float3 right;
	float3 delta;
	
    VertexShaderOutput output;

#ifdef AALIGNED	
	up = float3( 0.0f, 1.0f, 0.0f);
#else
	up = float3( mv[0][1], mv[1][1], mv[2][1] );
#endif

	right = float3( mv[0][0], mv[1][0], mv[2][0] );
	
	delta = float3( model[3][0], model[3][1], model[3][2] ) - (float3)view_pos;
	delta = normalize( delta) ;

	float3 position = input.position.x * right + input.position.y * up;// + input.position.z * delta;


	pos = float4(position, 1.0f) ;
    pos = mul(pos, mvp);

    output.position = pos;
    output.texcoord0 = input.texcoord0;

    return output;
}
