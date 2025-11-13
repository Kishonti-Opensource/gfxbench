/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#ifdef NEIGHTBOR_TEXTURE_RGBA8
layout (rgba8, binding = OUT_NEIGHBOR_TEX_BIND) uniform highp writeonly image2D out_neighbor_tex;
#else
layout (r32ui, binding = OUT_NEIGHBOR_TEX_BIND) uniform highp writeonly uimage2D out_neighbor_tex;
#endif

layout(std430, binding = MAX_BUFFER_BIND) buffer MaxBuffer
{
    vec2 v[];
} max_buffer;


bool isValid(ivec2 tc, ivec2 image_size)
{
    return (tc.x >=0 ) && (tc.y >= 0) && (tc.x < image_size.x) && (tc.y < image_size.y);
}


layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = 1) in;
void main() {
    ivec2 g_i = ivec2(gl_GlobalInvocationID.xy);
    if (g_i.x >= NEIGHBOR_MAX_TEX_WIDTH || g_i.y > NEIGHBOR_MAX_TEX_HEIGHT)
    {
        return;
    }

    ivec2 max_image_size = ivec2(NEIGHBOR_MAX_TEX_WIDTH, NEIGHBOR_MAX_TEX_HEIGHT);

    float max_length = -1.0;
    vec2 max_value = vec2(0.0);

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            ivec2 offset = ivec2(i,j);

            ivec2 tc = g_i + offset;
            vec2 s = vec2(0.0);
            if (isValid(tc,max_image_size))
            {
                int b_id = tc.y * max_image_size.x + tc.x;

                s = max_buffer.v[ b_id ];


                float ls = dot(s, s);

                if (max_length < ls)
                {
                    max_length = ls;
                    max_value = s;
                }
            }
        }
    }

#ifdef NEIGHTBOR_TEXTURE_RGBA8
    imageStore(out_neighbor_tex,g_i, pack2FloatToVec4( max_value ));
#else
    imageStore(out_neighbor_tex,g_i, uvec4(packHalf2x16( max_value )));
#endif
}
