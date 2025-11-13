/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

#include "hdr_luminance.h"

#ifdef SV_40
#include "sat_functions.h"
#include "rgbm_helper.h"
#endif

#define THREAD_COUNT uint(WORK_GROUP_SIZE_X * WORK_GROUP_SIZE_Y)

layout(binding = 0) uniform mediump sampler2D input_texture;

layout(std430, binding = 0) writeonly buffer output_buffer_layout
{
    // Sum of the tile samples
    highp float values[];
} output_buffer;

uniform vec2 texel_center;  // Half texel size of the original texture
uniform vec2 step_uv;       // Step between the samples
uniform ivec2 sample_count; // Count of samples in a tile

shared highp float samples[THREAD_COUNT];

#define DELTA 0.00000001
highp float fetchTile(vec2 global_id)
{
    // Sample the texture
    highp float result = 0.0;
    vec2 offset = step_uv * (global_id * vec2(sample_count)) + texel_center;
    for (int y = 0; y < sample_count.y; y++)
    {
        float v = offset.y + float(y) * step_uv.y;
        for (int x = 0; x < sample_count.x; x++)
        {
            float u = offset.x + float(x) * step_uv.x;
    #ifdef SV_40
            vec3 tap_color = RGBDtoRGB_lightcombine( texture(input_texture, vec2(u, v)) );
    #else // SV_31
            vec3 tap_color = texture(input_texture, vec2(u, v)).xyz * 4.0; //uncompress from GL_RGB10_A2
    #endif
            result += log(getLuminance(tap_color) + DELTA);
        }
    }
    return result;
}

layout (local_size_x = WORK_GROUP_SIZE_X, local_size_y = WORK_GROUP_SIZE_Y, local_size_z = 1) in;
void main()
{
    // Collect the samples
    samples[gl_LocalInvocationIndex] = fetchTile(vec2(gl_GlobalInvocationID.xy));
    memoryBarrierShared();
    barrier();

    // Parallel reduction
    for(uint s = THREAD_COUNT / 2u; s > 0u; s >>= 1u)
    {
        if(gl_LocalInvocationIndex < s)
        {
            samples[gl_LocalInvocationIndex] += samples[gl_LocalInvocationIndex + s];
        }
        memoryBarrierShared();
        barrier();
    }

    // The first thread stores the result
    if (gl_LocalInvocationIndex == 0u)
    {
        output_buffer.values[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = samples[0];
    }
}
