/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef GL_ES

precision mediump float ;
layout (GLSL_TEX_FORMAT, binding = OUT_TEX_BIND) writeonly uniform mediump image2D out_tex ;

#else

layout (GLSL_TEX_FORMAT, binding = OUT_TEX_BIND) writeonly uniform image2D out_tex ;

#endif

#if defined HORIZONTAL

#define LOCAL_WORK_SIZE_X LOCAL_WORK_SIZE
#define LOCAL_WORK_SIZE_Y 1

#elif defined VERTICAL

#define LOCAL_WORK_SIZE_X 1
#define LOCAL_WORK_SIZE_Y LOCAL_WORK_SIZE

#endif

layout(binding = 0) uniform sampler2D texture_unit0_highp;
uniform int gauss_lod_level;

layout (local_size_x = LOCAL_WORK_SIZE_X, local_size_y = LOCAL_WORK_SIZE_Y, local_size_z = 1) in;
void main() {	
	ivec2 g_i  = ivec2(gl_GlobalInvocationID.xy) ;	
	
	const vec2 stepxy[] = vec2[]( STEPXY ) ;
	
	float stepx = stepxy[gauss_lod_level].x ;
	float stepy = stepxy[gauss_lod_level].y ;
	
	float px = (float(g_i.x)+0.5)*stepx ;
	float py = (float(g_i.y)+0.5)*stepy ;
	
	const float offsets[]       = float[]( GAUSS_OFFSETS ) ;
	const float gauss_weights[] = float[]( GAUSS_WEIGHTS ) ;
	
	VEC_TYPE s = VEC_TYPE(0.0) ;
	
	for (int i = 0 ; i < KS ; i++)
	{
#if defined HORIZONTAL	
		vec2 offset = vec2(stepx*offsets[i],0.0) ;
#elif defined VERTICAL
		vec2 offset = vec2(0.0,stepy*offsets[i]) ;
#endif
		float w = gauss_weights[i] ;
		
		vec2 si = vec2(px,py) + offset ;
		
		s += w * unpack(textureLod( texture_unit0_highp, si,float(gauss_lod_level)));
	}
	
	imageStore(out_tex,g_i,pack(s)) ;
}

