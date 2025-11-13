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
	hfloat4 position [[position]];
	hfloat2 out_texcoord0;
};

#ifdef TYPE_fragment

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,depth2d<hfloat> texture_unit0 [[texture(0)]]
							,sampler sampler0 [[sampler(0)]])

{
    return texture_unit0.sample(sampler, in.out_texcoord0);
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = vec4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

    return output;
}

#endif
