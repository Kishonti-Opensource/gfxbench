/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "hdr_luminance.h"
#include "hdr_reduction.h"


// hfloat4 adaptive_avg_pad2;
// .x - adaptive luminance
// .y - current frame average luminance
// .zw - padding
// 


//layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
kernel void shader_main( constant hdr_reduction_pass2_uniforms* ru_pass2               [[ buffer ( HDR_REDUCTION_PASS2_UNIFORMS_BFR_SLOT ) ]],
						 device   hfloat*                       input_values           [[ buffer ( HDR_REDUCTION_PASS2_INPUT_BFR_SLOT )    ]],
						 device   hfloat4*                      adaptive_avg_pad2      [[ buffer ( HDR_REDUCTION_PASS2_OUTPUT_BFR_SLOT )   ]],
						          uint                          local_invocation_index [[ thread_index_in_threadgroup                      ]])
{
	//highp shared float samples[WORK_GROUP_SIZE];
	threadgroup hfloat samples[WORK_GROUP_SIZE];

    // Load the samples to the fast shared memory
    samples[local_invocation_index] = input_values[local_invocation_index];
    threadgroup_barrier(mem_flags::mem_threadgroup) ;

    // Parallel reduction
    for(uint s = uint(WORK_GROUP_SIZE) / 2u; s > 0u; s >>= 1u)
    {
        if(local_invocation_index < s)
        {
            samples[local_invocation_index] += samples[local_invocation_index + s];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup) ;
    }

    // The first thread stores the results
    if (local_invocation_index == 0u)
    {
        // The avg luminance of the current frame
        _float avg_lum = exp(samples[0] * ru_pass2->texture_samples_inv - 0.00000001);

        if (ru_pass2->adaptation_mode == 0)
        {
            // Adaptive luminance mode
            _float dt = ru_pass2->time_dt_pad2.y;
            if (dt <= 0.0)
            {
                // This is the first frame or the scene is restarted or going back in time
                adaptive_avg_pad2->x = avg_lum;
            }
            else
            {
                // Pattanaik's exponential decay function
                _float tau = ru_pass2->EFW_tau.w;
                _float prev_lum = adaptive_avg_pad2->x;
                adaptive_avg_pad2->x = prev_lum + (avg_lum - prev_lum) * (1.0 - exp(-dt * tau));
            }
        }
        else if (ru_pass2->adaptation_mode == 1)
        {
            // Adaptation disabled mode
            adaptive_avg_pad2->x = avg_lum;
        }
        else
        {
            // Predefined mode
            adaptive_avg_pad2->x = ru_pass2->predefined_luminance;
        }

        adaptive_avg_pad2->y = avg_lum;

        // Uncomment this to turn off adaptation
        // adaptive_avg_pad2->x = avg_lum;
    }
}

#endif

