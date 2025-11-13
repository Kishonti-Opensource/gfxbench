/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture_unit0;
in vec2 out_texcoord0;
out vec4 frag_color;

uniform highp float gauss_offsets[KS] ;
uniform highp float gauss_weights[KS] ; 

void main()
{         
    vec4 s = vec4(0.0);
       
    for (int i = 0; i < KS; i++)
    {
#if defined HORIZONTAL	
        vec2 offset = vec2(gauss_offsets[i],0.0);
#elif defined VERTICAL
        vec2 offset = vec2(0.0,gauss_offsets[i]);
#endif
        float w = gauss_weights[i];
        s += w * texture( texture_unit0, out_texcoord0 + offset);
    }
    frag_color = s;
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
