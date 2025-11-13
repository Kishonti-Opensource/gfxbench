/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "hdr_luminance.h"
#include "hdr_reduction40.h"

#ifdef SV_40
#include "rgbm_helper.h"
#endif

#define THREAD_COUNT uint(WORK_GROUP_SIZE_X * WORK_GROUP_SIZE_Y)


#define DELTA 0.00000001
hfloat fetchTile(_float2 global_id, constant hdr_reduction_pass1_uniforms* ru, texture2d<_float> input_texture, sampler input_sampler)
{
    // Sample the texture
    hfloat result = 0.0;
    _float2 offset = _float2(ru->step_uv) * (global_id * _float2(ru->sample_count)) + _float2(ru->texel_center);
    for (int y = 0; y < ru->sample_count.y; y++)
    {
        _float v = offset.y + _float(y) * ru->step_uv.y;
        for (int x = 0; x < ru->sample_count.x; x++)
        {
            _float u = offset.x + _float(x) * ru->step_uv.x;
    #ifdef SV_40
            _float3 tap_color = RGBDtoRGB_lightcombine( input_texture.sample(input_sampler, hfloat2(u, 1.0-v)) );
    #else // SV_31
            _float3 tap_color = RGBA10A2_DECODE( input_texture.sample(input_sampler, hfloat2(u, v)).xyz ); //uncompress from GL_RGB10_A2
    #endif
            result += log(getLuminance( hfloat3(tap_color) ) + DELTA);
        }
    }
    return result;
}


kernel void shader_main( constant hdr_reduction_pass1_uniforms* ru_pass1               [[ buffer ( HDR_REDUCTION_PASS1_UNIFORMS_BFR_SLOT )  ]],
						 device   hfloat*                       output_values          [[ buffer ( HDR_REDUCTION_PASS1_OUTPUT_BFR_SLOT )    ]],
								  texture2d<_float>             input_texture          [[ texture( HDR_REDUCTION_PASS1_INPUT_TEX_SLOT)      ]],
								  sampler                       input_sampler          [[ sampler( HDR_REDUCTION_PASS1_INPUT_SAMPLER_SLOT ) ]],
						          uint2                         global_invocation_id   [[ thread_position_in_grid                           ]],
						          uint                          local_invocation_index [[ thread_index_in_threadgroup                       ]],
						          uint2                         workgroup_id           [[ threadgroup_position_in_grid                      ]],
						          uint2                         num_workgroups         [[ threadgroups_per_grid                             ]] )
{
	// shared highp float samples[THREAD_COUNT];
	threadgroup hfloat samples[THREAD_COUNT];

    // Collect the samples
    samples[ local_invocation_index ] = fetchTile(_float2(global_invocation_id),ru_pass1, input_texture, input_sampler);
    
    threadgroup_barrier(mem_flags::mem_threadgroup) ;

    // Parallel reduction
    for(uint s = THREAD_COUNT / 2u; s > 0u; s >>= 1u)
    {
        if(local_invocation_index < s)
        {
            samples[local_invocation_index] += samples[local_invocation_index + s];
        }

        threadgroup_barrier(mem_flags::mem_threadgroup) ;
    }

    // The first thread stores the result
    if (local_invocation_index == 0u)
    {
        output_values[workgroup_id.y * num_workgroups.x + workgroup_id.x] = samples[0];
    }
}

#endif


