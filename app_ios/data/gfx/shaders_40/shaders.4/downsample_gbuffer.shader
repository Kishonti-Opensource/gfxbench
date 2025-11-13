/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

in vec2 out_texcoord0;
//out vec4 frag_color;

void main()
{
	//frag_color = texture( texture_unit0, out_texcoord0);	
    gl_FragDepth = texture( depth_unit0, out_texcoord0).x;	
	//gl_FragDepth = 0.0;
	
	//ivec2 ssP = ivec2(gl_FragCoord.xy);
	//gl_FragDepth = texelFetch(depth_unit0, clamp(ssP * 2 + ivec2(ssP.y & 1, ssP.x & 1), ivec2(0), textureSize(depth_unit0, 0) - ivec2(1)), 0).x;
}

#endif

#ifdef TYPE_vertex

in vec3 in_position;
out vec2 out_texcoord0;

void main()
{
    gl_Position = vec4( in_position, 1.0);
    out_texcoord0 = in_position.xy * 0.5 + 0.5;
}

#endif
 