/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef GL_ES

#define TEX_TYPE rgba8

layout (TEX_TYPE, binding = IN_TEX_BIND)  uniform readonly mediump image2D in_tex ;

#else

#ifdef RG16F_SUPPORTED
#define TEX_TYPE rg16f
#else
#define TEX_TYPE rgba8
#endif

layout (TEX_TYPE, binding = IN_TEX_BIND)  uniform readonly  image2D in_tex ;

#endif

layout(std430, binding = MAX_BUFFER_BIND) buffer MaxBuffer
{
	vec2 v[];
} max_buffer ;


bool isValid(ivec2 tc, ivec2 image_size)
{
	return (tc.x >=0 ) && (tc.y >= 0) && (tc.x < image_size.x) && (tc.y < image_size.y) ;
}


layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = 1) in;
void main() {
	ivec2 g_i = ivec2(gl_GlobalInvocationID.xy) ;
	
	ivec2 in_image_size  = imageSize(in_tex) ;
	ivec2 max_image_size = ivec2(NEIGHBOR_MAX_TEX_WIDTH,NEIGHBOR_MAX_TEX_HEIGHT)  ;
	
	
	//
	//	Out of range check. Require, because we use a 1D buffer instead of a texture
	//
	if ( (g_i.y >= NEIGHBOR_MAX_TEX_HEIGHT) ||
		 (g_i.x >= NEIGHBOR_MAX_TEX_WIDTH) )
	{
		return ;
	}
	
	
	float max_length = -1.0 ;
	vec2 max_value = vec2(0.0) ;
	
	for (int i = 0 ; i < K; i++) 
	{
		for(int j = 0 ; j < K; j++)
		{
			ivec2 offset = ivec2(i,j) ;
			
			ivec2 tc = K*g_i + offset ;
			
			
			
			vec2 s = vec2(0.0) ;
			
			if (isValid(tc,in_image_size))
			{
				s = unpackVec2FFromVec4( imageLoad( in_tex,tc)  ) ;
			}
			
			float ls = length(s) ;
			
			if (max_length < ls) 
			{
				max_length = ls ;
				max_value = s ;
			}
		}
	}
	
	int b_id = g_i.y * max_image_size.x + g_i.x ;
	
	max_buffer.v[ b_id ] = max_value ;
}