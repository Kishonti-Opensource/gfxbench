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

layout(std430, binding = 1) buffer output_buffer_layout
{
    //.x - adaptive luminance
    //.y - current frame average luminance
    //.zw - padding
    vec4 adaptive_avg_pad2;
}output_buffer;

highp uniform float texture_samples_inv;
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
        output_buffer.adaptive_avg_pad2.y = avg_lum;

        float adaptive_lum;
#if (ADAPTATION_MODE == 0)

        // Adaptive luminance mode
        // Pattanaik's exponential decay function
        float dt = time_dt_pad2.y;
        float tau = EFW_tau.w;
        float prev_lum = output_buffer.adaptive_avg_pad2.x;

        adaptive_lum = prev_lum + (avg_lum - prev_lum) * (1.0 - exp(-dt * tau));

#elif (ADAPTATION_MODE == 1)

         // Adaptation disabled mode
         adaptive_lum = avg_lum;

#else

        // Pre-defined mode
        adaptive_lum = predefined_luminance;

#endif

        // Uncomment this to turn off adaptation
        //adaptive_lum = avg_lum;

        float val = adaptive_lum + 1.0;
        float log10val = log(val) / log(10.0);
        float middleGrey = 1.03 - (2.0 / (2.0 + log10val));
        middleGrey *= exposure_bloomthreshold_tone_map_white_pad.x;
        float linExposure = middleGrey / adaptive_lum;
        float exposure = log2(max(linExposure, 0.0001));

        output_buffer.adaptive_avg_pad2.x = adaptive_lum;
        output_buffer.adaptive_avg_pad2.z = exposure;
    }
}
