/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include lightLensFlareConsts;
#else
	uniform vec3 light_color;
#endif

in vec2 out_texcoord0;
uniform sampler2D texture_unit0;

out vec4 frag_color;

void main()
{   
#ifdef USE_UBOs
	frag_color = texture( texture_unit0, out_texcoord0) * vec4( light_color_pad.xyz, 1.0) * 0.1;
#else
	frag_color = texture( texture_unit0, out_texcoord0) * vec4( light_color, 1.0) * 0.1;
#endif	

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
#endif	
}

