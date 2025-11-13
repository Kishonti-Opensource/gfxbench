/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in vec2 out_texcoord0;
out vec4 frag_color;

#ifdef USE_UBOs
	#include staticMeshConsts;
#else
	uniform vec3 color;
#endif

uniform sampler2D texture_unit0;


void main()
{   
#ifdef USE_UBOs
	vec3 color = color_pad.xyz;
#endif
	vec4 texel = texture( texture_unit0, out_texcoord0) * vec4( color, 1.0);
	frag_color = vec4( texel.xyz, 0.0);
}

