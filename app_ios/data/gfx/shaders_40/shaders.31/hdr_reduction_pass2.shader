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
#include "ubo_frame.h"

layout(std430, binding = 0) readonly buffer input_buffer_layout
{
    highp float values[];
} input_buffer;

layout(std140, binding = 1) buffer output_buffer_layout
{
    //.x - adaptive luminance
    //.y - current frame average luminance
    //.zw - padding
    vec4 adaptive_avg_pad2;
} output_buffer;

highp uniform float texture_samples_inv;
uniform int adaptation_mode; // 0: adaptive, 1 - disabled, 2 - predefined
uniform float predefined_luminance;

highp shared float samples[WORK_GROUP_SIZE];

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    // Load the samples to the fast shared memory
    samples[gl_LocalInvocationIndex] = input_buffer.values[gl_LocalInvocationIndex];
    memoryBarrierShared();
    barrier();

    // Parallel reduction
    for(uint s = uint(WORK_GROUP_SIZE) / 2u; s > 0u; s >>= 1u)
    {
        if(gl_LocalInvocationIndex < s)
        {
            samples[gl_LocalInvocationIndex] += samples[gl_LocalInvocationIndex + s];
        }
        memoryBarrierShared();
        barrier();
    }

    // The first thread stores the results
    if (gl_LocalInvocationIndex == 0u)
    {
        // The avg luminance of the current frame
        float avg_lum = exp(samples[0] * texture_samples_inv - 0.00000001);

        if (adaptation_mode == 0)
        {
            // Adaptive luminance mode
            float dt = time_dt_pad2.y;
            if (dt <= 0.0)
            {
                // This is the first frame or the scene is restarted or going back in time
                output_buffer.adaptive_avg_pad2.x = avg_lum;
            }
            else
            {
                // Pattanaik's exponential decay function
                float tau = EFW_tau.w;
                float prev_lum = output_buffer.adaptive_avg_pad2.x;
                output_buffer.adaptive_avg_pad2.x = prev_lum + (avg_lum - prev_lum) * (1.0 - exp(-dt * tau));
            }
        }
        else if (adaptation_mode == 1)
        {
            // Adaptation disabled mode
            output_buffer.adaptive_avg_pad2.x = avg_lum;
        }
        else
        {
            // Predefined mode
            output_buffer.adaptive_avg_pad2.x = predefined_luminance;
        }

        output_buffer.adaptive_avg_pad2.y = avg_lum;

        // Uncomment this to turn off adaptation
        //output_buffer.adaptive_avg_pad2.x = avg_lum;
    }
}
