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

constexpr sampler sampler0(coord::normalized, filter::linear, address::clamp_to_edge);
constexpr sampler sampler1(coord::normalized, filter::linear, address::clamp_to_edge);
constexpr sampler depth_sampler(coord::normalized, filter::nearest, address::clamp_to_edge);

_float Dof(depth2d<hfloat> depthTex, hfloat2 uv, constant FilterUniforms * filter_consts)
{
	_float d_s = (filter_consts->dof_strength * 0.1);

	_float depth = getLinearDepth(depthTex, depth_sampler, uv, filter_consts->depth_parameters);

    _float dof = metal::precise::clamp( abs(log(depth/filter_consts->camera_focus)) / d_s, 0.0, 1.0);

	dof = powr( dof, _float(0.6) );

	return dof;
}


fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,constant FilterUniforms * filter_consts [[buffer(FILTER_UNIFORMS_SLOT)]]
							,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,texture2d<_float> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]]
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]])
{
	_float4 c0 = texture_unit0.sample(sampler0, input.out_texcoord0);
	_float4 c1 = texture_unit1.sample(sampler1, input.out_texcoord0);

	_float f = clamp(2.0*Dof(depth_unit0, input.out_texcoord0, filter_consts)-0.75,0.0,1.0) ;

#ifdef DRAW_GS_WIREFRAME	
	f = 0.0;
#endif
	_float3 t = mix(c0.xyz, c1.xyz, f );
	return hfloat4(hfloat3(mix(c0.xyz,c1.xyz, f )), 1.0) ; //NOTE: Use 0.0 instead of f to turn off DoF
}


#endif


#ifdef TYPE_vertex

#ifndef ROTATE_DOF
#error ROTATE_DOF not defined
#endif

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

#if ROTATE_DOF
	output.out_texcoord0 = _float2(1.0 - output.out_texcoord0.y, output.out_texcoord0.x);
#endif

    return output;
}



#endif