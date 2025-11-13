/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture_unit0;
uniform int gauss_lod_level;

uniform /*highp*/ vec2 inv_resolution[LOD_LEVEL_COUNT];

in vec2 out_texcoord0;
out vec4 frag_color;

uniform /*highp*/ float gauss_offsets[KS] ;
uniform /*highp*/ float gauss_weights[KS] ;

void main()
{			
	vec4 s = vec4(0.0);	
	for (int i = 0; i < KS; i++)
	{
#if defined HORIZONTAL	
		vec2 offset = vec2(inv_resolution[gauss_lod_level].x * gauss_offsets[i],0.0);
		
#elif defined VERTICAL
		vec2 offset = vec2(0.0,inv_resolution[gauss_lod_level].y * gauss_offsets[i]);
#endif
		float w = gauss_weights[i];
		
		vec2 si = out_texcoord0 + offset;
		
		s += w * textureLod( texture_unit0, si, float(gauss_lod_level));
	}

	frag_color = s;
}
