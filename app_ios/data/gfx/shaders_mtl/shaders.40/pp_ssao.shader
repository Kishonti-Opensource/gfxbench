/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//https://gist.github.com/fisch0920/6770311
//https://research.nvidia.com/publication/scalable-ambient-obscurance

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

// Total number of samples at each fragment
#define NUM_SAMPLES           7
#define NUM_SPIRAL_TURNS      3

// Beta4 sample counts
//#define NUM_SAMPLES           10
//#define NUM_SPIRAL_TURNS      6

//#define NUM_SAMPLES           32
//#define NUM_SPIRAL_TURNS      3

//#define NUM_SAMPLES           64
//#define NUM_SPIRAL_TURNS      17

#define MAX_DISTANCE				(500.0)
#define MAX_DISTANCE_HALF	 		(MAX_DISTANCE * 0.5)
#define MAX_DISTANCE_HALF_INV		(1.0 / MAX_DISTANCE_HALF)

#define LOG_MAX_OFFSET (2)

#define RADIUS 1.0

#define RADIUS2 (RADIUS * RADIUS)


constexpr sampler sampler0(coord::normalized, filter::linear, mip_filter::linear, address::clamp_to_edge); // CHECKED
constexpr sampler sampler1(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge); // CHECKED


hfloat unpackFloatFromVec4i(hfloat4 value)
{
	// http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
	return dot( value, hfloat4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/160581375.0) );
}

template<class T>
hfloat3 getOffsetPosition(T texture_unit0, hfloat2 uv, int2 ssC, hfloat2 unitOffset, _float ssR, constant SSAOUniforms * ssao_consts)
{
	hfloat2 sample_uv = (ssR / ssao_consts->projection_scale * unitOffset) + uv;

#ifdef USE_HIZ_DEPTH
	int mipLevel = clamp(31-clz(int(ssR)) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	hfloat linear_z = unpackFloatFromVec4i(texture_unit0.sample(sampler0, sample_uv, level(_float(mipLevel))));

	hfloat linear_depth = mix(ssao_consts->depth_parameters.z, ssao_consts->depth_parameters.w, linear_z);
	return getPositionVS(linear_depth, sample_uv, ssao_consts->corners);
#else
	return getPositionVS(depth_unit0, sampler0, sample_uv, ssao_consts->depth_parameters, ssao_consts->corners);
#endif
}


/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
hfloat3 tapLocation(int sampleNumber, _float spinAngle)
{
	// Radius relative to ssR
	hfloat alpha = (hfloat(sampleNumber) + 0.5) * (1.0 / hfloat(NUM_SAMPLES));
	hfloat angle = alpha * (hfloat(NUM_SPIRAL_TURNS) * 2.0 * PI) + spinAngle;
	return hfloat3(cos(angle), -sin(angle), alpha);
}


template<class T>
_float sampleAO(T depth_unit0, hfloat2 uv, int2 sspace_coords, hfloat3 vs_pos, hfloat3 vs_normal, _float sspace_radius, int tapIndex, _float random_angle, constant SSAOUniforms * ssao_consts)
{
	// Offset on the unit disk, spun for this pixel
	hfloat3 unitOffset = tapLocation(tapIndex, random_angle);

	_float alpha = unitOffset.z * sspace_radius;

	// The occluding point in camera space
	hfloat3 Q = getOffsetPosition(depth_unit0, uv, sspace_coords, unitOffset.xy, alpha, ssao_consts);

	hfloat3 v = Q - vs_pos;

	_float vv = _float(dot(v, v));
	_float vn = _float(dot(v, vs_normal) - 0.1);

	_float f = max(RADIUS2 * 4.0 - vv, 0.0) / RADIUS2 / 4.0;
	f = powr(f, _float(0.01));

#define EPSILON 0.01
	return f * max(vn / (EPSILON + vv), 0.0);
}


fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,constant SSAOUniforms * ssao_consts [[buffer(FILTER_UNIFORMS_SLOT)]]
#ifdef USE_HIZ_DEPTH
							,texture2d<hfloat> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
#else
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]]
#endif
							,texture2d<_float> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]])
{
    // [AAPL] Y-flip; this is the only way I can get the SSAO to closely match the GL version.
    hfloat2 texcoord0_yflip = input.out_texcoord0;
    texcoord0_yflip.y = 1.0 - texcoord0_yflip.y;
#ifdef USE_HIZ_DEPTH
	hfloat linear_z = unpackFloatFromVec4i(texture_unit0.sample(sampler0, texcoord0_yflip, level(0.0)));
	hfloat linear_depth  = mix(ssao_consts->depth_parameters.z, ssao_consts->depth_parameters.w, linear_z);
#else
	hfloat linear_depth = getLinearDepth(depth_unit0, sampler0, texcoord0_yflip, ssao_consts->depth_parameters);
#endif

	if (linear_depth >= MAX_DISTANCE)
	{
		return 1.0;
	}

	hfloat3 vs_pos = getPositionVS(linear_depth, texcoord0_yflip, ssao_consts->corners);
	hfloat3 vs_normal = decodeNormal(texture_unit1, sampler1, texcoord0_yflip);

	// Flip coords for correct algo
	vs_normal = hfloat3(-vs_normal.y, vs_normal.x, vs_normal.z);

	int2 sspace_coords = int2(input.out_position.xy);

	_float random_angle = 2.0 * PI * rand(input.out_texcoord0);

	_float sspace_radius = ssao_consts->projection_scale * RADIUS /  linear_depth;

	_float occlusion = 0.0;
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
#ifdef USE_HIZ_DEPTH
		occlusion += sampleAO(texture_unit0, texcoord0_yflip, sspace_coords, vs_pos, vs_normal, sspace_radius, i, random_angle, ssao_consts);
#else
		occlusion += sampleAO(depth_unit0, texcoord0_yflip, sspace_coords, vs_pos, vs_normal, sspace_radius, i, random_angle, ssao_consts);
#endif
	}

	occlusion = 1.0 - occlusion / _float(  NUM_SAMPLES);
	occlusion = powr(occlusion, _float(4.0));

	// Remove artifacts
	_float distanceFade = clamp((linear_depth - MAX_DISTANCE_HALF) * MAX_DISTANCE_HALF_INV, 0.0, 1.0);
    return mix(occlusion, _float(1.0), sqrt(distanceFade));
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position.x, -input.in_position.y, input.in_position.z, 1.0); // [AAPL] Y-flip
    output.out_texcoord0 = input.in_position.xy * 0.5 + 0.5;

    return output;
}

#endif
