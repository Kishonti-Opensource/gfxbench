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

constexpr sampler sampler1(coord::normalized, filter::linear, mip_filter::none, address::clamp_to_edge);
constexpr sampler depth_sampler(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,constant SSDSConstants * ssds_consts [[buffer(FILTER_UNIFORMS_SLOT)]]
							,texture2d<hfloat> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,texture2d<hfloat> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]]
							,texture2d<_float> texture_unit7 [[texture(TEXTURE_UNIT7_SLOT)]]
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]]
							,depth2d_array<hfloat> shadow_array [[texture(CASCADED_SHADOW_TEXTURE_ARRAY_SLOT)]]
							,sampler shadow_sampler [[sampler(CASCADED_SHADOW_TEXTURE_ARRAY_SLOT)]]
                            ,sampler sampler7 [[sampler(TEXTURE_UNIT7_SLOT)]])
{
	hfloat depth = depth_unit0.sample(depth_sampler, input.out_texcoord0);
	hfloat linear_depth = getLinearDepth(depth, ssds_consts->depth_parameters);
	hfloat3 world_pos = getPositionWS(linear_depth, input.out_texcoord0, ssds_consts->corners, ssds_consts->view_pos);

	hfloat4 frag_color;
    // [AAPL] Y-flip; this is the only way I can get the SSAO to closely match the GL version.
    hfloat2 out_texcoord0_yflip = hfloat2(input.out_texcoord0.xy);
    out_texcoord0_yflip.y = 1.0 - out_texcoord0_yflip.y;
	frag_color.x = texture_unit1.sample(sampler1, out_texcoord0_yflip).x; // Copy the AO
	frag_color.y = shadowCascaded(texture_unit7, sampler7, shadow_array, shadow_sampler, depth, hfloat4(world_pos, 1.0), ssds_consts->cascaded_frustum_distances, ssds_consts->cascaded_shadow_matrices); // Shadow
	frag_color.zw = hfloat2(EncodeLinearDepthToVec2(linear_depth)); // Encode the linear depth to the BA channels

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
