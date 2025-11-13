/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#define HALF_SIZED_LIGHTING

in vec2 out_texcoord0;
uniform sampler2D texture_unit0;

#ifdef HALF_SIZED_LIGHTING
	out vec4 frag_color[4];
#else
	out vec4 frag_color;
#endif

void main()
{   
	vec4 out_color = texture( texture_unit0, out_texcoord0);
	out_color.w = 0.0;

#ifdef SV_31
	out_color = pow( out_color + vec4(0.1,0.1,0.1,0.0), vec4( 3.0));
#endif

#ifdef HALF_SIZED_LIGHTING
	frag_color[0] = out_color;
	frag_color[1] = vec4(0,0,0,0.6); //needs emissive due to being unlit
	frag_color[2] = vec4(0,0,0,0);
	frag_color[3] = vec4(0,0,0,0);
#else
	frag_color = out_color;
#endif

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif		
}

