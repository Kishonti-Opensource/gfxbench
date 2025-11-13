/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform highp sampler2D texture_unit0; // Depth texture
uniform mediump ivec2 texture_size;

in vec2 out_texcoord0;
out vec2 frag_color;

void main()
{
    highp vec4 samples;

    //samples.x = texture(texture_unit0, out_texcoord0).x;
    //samples.y = textureOffset(texture_unit0, out_texcoord0, ivec2(-1, 0)).x;
    //samples.z = textureOffset(texture_unit0, out_texcoord0, ivec2(-1,-1)).x;
    //samples.w = textureOffset(texture_unit0, out_texcoord0, ivec2( 0,-1)).x;

    samples = textureGatherOffset(texture_unit0, out_texcoord0, ivec2(0, 0));
    highp float max_z = max(max(samples.x, samples.y), max(samples.z, samples.w));

    highp vec3 extra;
    ivec2 src_choords = ivec2(gl_FragCoord + 0.5) * 2;
    bvec2 is_odd = bvec2((texture_size.x & 1) != 0, (texture_size.y & 1) != 0);
    if (is_odd.x && src_choords.x == texture_size.x - 3)
    {
         if (is_odd.y && src_choords.y == texture_size.y - 3)
        {
            extra.z = textureOffset(texture_unit0, out_texcoord0, ivec2( 1, 1)).x;
            max_z = max( max_z, extra.z );
        }

        extra.x = textureOffset(texture_unit0, out_texcoord0, ivec2( 1, 0)).x;
        extra.y = textureOffset(texture_unit0, out_texcoord0, ivec2( 1,-1)).x;
        max_z = max( max_z, max( extra.x, extra.y ) );
    }
    else if (is_odd.y && src_choords.y == texture_size.y - 3)
    {
         extra.x = textureOffset(texture_unit0, out_texcoord0, ivec2( -1, 1)).x;
         extra.y = textureOffset(texture_unit0, out_texcoord0, ivec2( 0,1)).x;
         max_z = max( max_z, max( extra.x, extra.y ) );
    }

    gl_FragDepth = max_z;
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
