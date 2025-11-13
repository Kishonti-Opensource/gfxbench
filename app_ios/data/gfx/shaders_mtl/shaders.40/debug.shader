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


#ifdef TYPE_vertex
vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

    return output;
}

#endif


#ifdef TYPE_fragment

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,texture2d<hfloat> texture0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]]
							)
{
	return texture0.sample(sampler0, input.out_texcoord0);
}



#endif