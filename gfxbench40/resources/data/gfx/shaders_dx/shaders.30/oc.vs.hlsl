/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

float4 main(uint VertexID: SV_VertexID) : SV_POSITION
{
	// gl_PointSize = 6.0;
	
	float2 temp = float2( VertexID % 2, VertexID / 2 ); //0.0 1.0 0.1 1.1
	float2 temp2 = temp * 2.0 + ( -1.0);
	temp2.y *= -1.0;
	
	temp2 *= inv_screen_resolution * 6.0; //a.k.a. gl_PointSize
	
	float4 offset = mul(light_pos, mvp);
	offset.xyz /= offset.w;
	temp2.x += offset.x;
	temp2.y += offset.y;
	
	float4 screenpos = float4( temp2.x, temp2.y, offset.z, 1.0f ); //-1.1 1.1 -1.-1 1.-1
		
	//return mul(mvp, position);
	return screenpos;
}
