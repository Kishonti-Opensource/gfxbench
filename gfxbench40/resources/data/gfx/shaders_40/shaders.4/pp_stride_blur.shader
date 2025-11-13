/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

highp in vec2 out_texcoord0;

#ifdef PASS_DEPTH
out vec4 frag_color;
#else
out vec2 frag_color;
#endif

#define A ((MAX_LINEAR_DEPTH)/MAX_BYTE)

#define B (-MAX_LINEAR_DEPTH)

#define F(x) ((x-B)/A)
#define F_(x) (x/A)
#define C(x) (x.z*MAX_BYTE + x.w)

//Shadow far is 220m, SSAO is 500m, so pointless to blur far pixels
#define MAX_FAR F(500.0)
#define DEPTH_DELTA F_(0.5)

void main()
{
    const float gauss_weights[] = float[]( GAUSS_WEIGHTS );
    const highp vec2 offsets[] = vec2[]( OFFSETS );

    float sum_weights = 1.0;
    vec4 fragment = texture(texture_unit0, out_texcoord0);
    
    vec2 result = fragment.xy;
    float depth = C(fragment);
   
    if (depth < MAX_FAR)  
    {
        for (int i = 0; i < 2 * KS; i++)
        {
           vec4 s = texture(texture_unit0, out_texcoord0 + offsets[i]);
              
           if (abs(C(s) - depth) <= DEPTH_DELTA)
           {
                result += s.xy * gauss_weights[i];
                sum_weights += gauss_weights[i];
           }
        }
    }
    frag_color.xy = result / sum_weights;
#ifdef PASS_DEPTH
    frag_color.zw = fragment.zw;
#endif

    //Comment out to turn off screen space AO, shadow blur
   //frag_color.xy = texture(texture_unit0, out_texcoord0).xy;
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
