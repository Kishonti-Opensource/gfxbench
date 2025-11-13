/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

struct VertexInput
{
	hfloat3 in_position [[attribute(0)]];
};

struct VertexOutput
{
	hfloat4 out_position [[position]];
	hfloat2 out_texcoord0;
};

#ifdef TYPE_fragment

#ifdef PASS_DEPTH
#define OUTPUT_TYPE _float4
#else
#define OUTPUT_TYPE _float2
#endif

#define A ((MAX_LINEAR_DEPTH)/MAX_BYTE)

#define B (-MAX_LINEAR_DEPTH)

#define F(x) ((x-B)/A)
#define F_(x) (x/A)
#define C(x) (x.z*MAX_BYTE + x.w)

//Shadow far is 220m, SSAO is 500m, so pointless to blur far pixels
#define MAX_FAR F(500.0)
#define DEPTH_DELTA F_(0.5)

constant _float gauss_weights[] = { GAUSS_WEIGHTS };
constant hfloat2 offsets[] = { OFFSETS };

fragment OUTPUT_TYPE shader_main(VertexOutput input [[stage_in]]
								,texture2d<_float> texture_unit0 [[texture(0)]]
								,sampler sampler0 [[sampler(0)]])
{
	OUTPUT_TYPE frag_color;

    _float sum_weights = 1.0;
    _float4 frag = texture_unit0.sample(sampler0, input.out_texcoord0);
    
    _float2 result = frag.xy;
    _float depth = C(frag);
   
    if (depth < MAX_FAR)  
    {
        for (int i = 0; i < 2 * KS; i++)
        {
           _float4 s = texture_unit0.sample(sampler0, input.out_texcoord0 + offsets[i]);
              
           if (abs(C(s) - depth) <= DEPTH_DELTA)
           {
                result += s.xy * gauss_weights[i];
                sum_weights += gauss_weights[i];
           }
        }
    }
    frag_color.xy = result / sum_weights;
#ifdef PASS_DEPTH
    frag_color.zw = frag.zw;
#endif

    //Comment out to turn off screen space AO, shadow blur
    //frag_color.xy = texture_unit0.sample(sampler0, input.out_texcoord0).xy;
	return frag_color;
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

    return output;
}


#endif
