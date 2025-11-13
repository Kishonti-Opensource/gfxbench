/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

in vec2 out_texcoord0;

out vec4 frag_color;

#define NEAR_MINUS_FAR depth_parameters.x
#define NEAR depth_parameters.z
#define FAR depth_parameters.w

void main()
{
    /*
     1, LINEAR_Z = (2.0 * Near) / (Far + Near - Z * (Far - Near));
     2, Z = texture_depth * 2 - 1
     3, In LINEAR_Z 0 is NEAR, 1 is FAR
        Camera_Z = mix(NEAR, FAR, LINEAR_Z)

    1, 2 gives us:
    LINEAR_Z = N / (F + (N - F) * texture_depth)
    */

    highp float linear_depth = NEAR / (FAR + NEAR_MINUS_FAR * texture(depth_unit0, out_texcoord0).x);

    // Encode to RGBA
    // http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
    highp vec4 v = vec4(1.0, 255.0, 65025.0, 160581375.0) * linear_depth;
    v = fract(v);
    v -= v.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
    frag_color = v;
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
