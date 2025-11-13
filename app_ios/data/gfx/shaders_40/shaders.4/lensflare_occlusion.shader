/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef GL_ES

layout (COLOR_TEX_TYPE, binding = COLOR_TEX_BIND)  uniform writeonly mediump image2D color_tex ;

#else

layout (COLOR_TEX_TYPE, binding = COLOR_TEX_BIND)  uniform writeonly image2D color_tex ;

#endif

uniform highp vec4 sun_pos ;

layout (binding = 0) uniform highp sampler2D depth_tex ;
layout (binding = 0) uniform highp atomic_uint visibility;

layout(std430, binding = OFFSET_BUFFER_BIND) buffer Offsets
{
	vec2 v[];
} offsets ;


layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main() {

	int idx = int(gl_GlobalInvocationID.x) ;
	
	float sample_region_size = 0.2 ;

	vec2 offset = sample_region_size*offsets.v[idx] ;
	
	vec2 sc = sun_pos.xy + offset ;
	
	if ( (sc.x < 0.0) || (sc.y < 0.0) || (sc.x > 1.0) || (sc.y > 1.0) || (sun_pos.w > 0.0))
	{
		return ;
	}
	
	highp float d = texture(depth_tex, sc).x ;
	
	
	if ( abs(1.0-d) < 0.0001) 
	{
		atomicCounterIncrement(visibility);

#if 0		
#define DEBUG_SIZE 8
		ivec2 debug_pos = ivec2( int(sc.x*VIEWPORT_WIDTH), int(sc.y*VIEWPORT_HEIGHT) ) ;

		for (int i = -DEBUG_SIZE ; i < DEBUG_SIZE ; i++)
		{
			for(int j = -DEBUG_SIZE ; j < DEBUG_SIZE ; j++)
			{
				//imageStore(color_tex, pos + ivec2(i,j) , vec4(vec3(sun_pos.w)/4.0,1.0) ) ;
				imageStore(color_tex, debug_pos + ivec2(i,j), vec4(0.0,1.0,0.0,1.0) ) ;
			}
		}
#endif 		

	}
}

