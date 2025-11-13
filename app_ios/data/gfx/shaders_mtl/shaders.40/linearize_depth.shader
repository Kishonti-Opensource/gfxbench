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

#define NEAR_MINUS_FAR	depth_parameters->x
#define NEAR			depth_parameters->z
#define FAR				depth_parameters->w

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,constant hfloat4 * depth_parameters
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]]
							,sampler sampler0 [[sampler(DEPTH_UNIT0_SLOT)]])
{
    /*
     1, LINEAR_Z = (2.0 * Near) / (Far + Near - Z * (Far - Near));
     2, Z = texture_depth * 2 - 1
     3, In LINEAR_Z 0 is NEAR, 1 is FAR
        Camera_Z = mix(NEAR, FAR, LINEAR_Z)

    1, 2 gives us:
    LINEAR_Z = N / (F + (N - F) * texture_depth)
    */

    hfloat linear_depth = NEAR / (FAR + NEAR_MINUS_FAR * depth_unit0.sample(sampler0, input.out_texcoord0));

    // Encode to RGBA
    // http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
    hfloat4 v = hfloat4(1.0, 255.0, 65025.0, 160581375.0) * linear_depth;
    v = fract(v);
    v -= v.yzww * hfloat4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
	return v;
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
