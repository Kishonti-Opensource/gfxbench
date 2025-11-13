/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
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

#include "hdr_common40.h"
#include "rgbm_helper.h"
#include "bright_pass40.h"

fragment hfloat4 shader_main(         VertexOutput      input             [[ stage_in ]],
							          texture2d<_float> texture_unit0     [[ texture ( BRIGHT_PASS_INPUT_TEXTURE_SLOT ) ]],
							          sampler           sampler0          [[ sampler ( BRIGHT_PASS_INPUT_SAMPLER_SLOT ) ]],
							 constant HDRConsts*        hdr_consts        [[ buffer  ( BRIGHT_PASS_HDRCONTS_BFR_SLOT  ) ]],
				        	 constant hfloat4*          adaptive_avg_pad2 [[ buffer  ( BRIGHT_PASS_LUMINANCE_BFR_SLOT ) ]])
{
	hfloat3 result = hfloat3(RGBDtoRGB_lightcombine(texture_unit0.sample(sampler0, input.out_texcoord0)));
	result = bright_pass(result, hdr_consts, adaptive_avg_pad2);
    return hfloat4(result, 1.0);
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = hfloat4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

    return output;
}

#endif
