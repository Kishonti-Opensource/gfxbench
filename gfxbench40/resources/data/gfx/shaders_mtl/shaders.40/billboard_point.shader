/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#define LIFETIME (1.1)

struct _particle
{
	hfloat4 Pos; // particle instance's position
	hfloat4 Velocity;
	hfloat4 T;
	hfloat4 B;
	hfloat4 N;
	hfloat4 Phase;
	hfloat4 Frequency;
	hfloat4 Amplitude;
	hfloat4 Age_Speed_Accel;
};

struct VertexOut
{
	hfloat4 position [[position]];
	hfloat3 out_worldpos;
	hfloat2 out_texcoord;
	hfloat out_norm_age [[flat]];
};

struct ParticleInstanceData
{
	hfloat4x4 mvp;
	hfloat4x4 view;
};

#ifdef TYPE_vertex

vertex VertexOut shader_main(device _particle* vertexData [[buffer(0)]]
							 ,device ushort* indexData [[buffer(1)]]
							 ,constant _float4x4& mvp [[buffer(2)]]
							 ,constant _float4x4& view [[buffer(3)]]
							 ,uint vertex_id [[vertex_id]]
							 ,uint instance_id [[instance_id]])
{
	const auto input = vertexData[indexData[instance_id]];

	_float3 up;
	_float3 right;
	_float norm_age = 1.0 - input.Age_Speed_Accel.x / LIFETIME;

	up = _float3( view[0][1], view[1][1], view[2][1]);
	right = _float3( view[0][0], view[1][0], view[2][0]);

	_float c = cos( _float(instance_id) + input.Age_Speed_Accel.x);
	_float s = sin( _float(instance_id) + norm_age);

	_float3 vUpNew    = c * right + s * up;
	_float3 vRightNew = s * right - c * up;

	up = vUpNew;
	right = vRightNew;

	_float size = mix( 0.6, 1.0, input.Age_Speed_Accel.x);

	VertexOut out;

	out.out_worldpos.xyz = input.Pos.xyz - _float3(0.0f, 0.372f, 0.0f);
	out.out_texcoord.x = (vertex_id & 1 ? 1.0f : 0.0f);
	out.out_texcoord.y = (vertex_id & 2 ? 1.0f : 0.0f);
	out.out_worldpos.xyz += ((vertex_id & 1 ? 1.0 : -1.0) * right + (vertex_id & 2 ? 1.0 : -1.0) * up) * size;
	out.out_norm_age = norm_age;
	out.position = mvp * _float4(out.out_worldpos, 1.0f);

	return out;
}

#endif

#ifdef TYPE_fragment
#include "rgbm_helper.h"

fragment _float4 shader_main(VertexOut input [[stage_in]]
							 ,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							 ,texture2d<_float> texture_unit7 [[texture(TEXTURE_UNIT7_SLOT)]]
							 ,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]]
							 ,sampler sampler7 [[sampler(TEXTURE_UNIT7_SLOT)]])
{
	/*highp*/ _float2 uv = _float2(input.out_worldpos.xz / 1950.0);
	uv += _float2(-0.5, 0.5);
	_float baked_ao = texture_unit7.sample(sampler7, hfloat2(uv)).y;
	baked_ao = clamp( baked_ao + 0.2, 0.0, 1.0);

	_float3 color = _float3( 0.6, 0.4, 0.2);
	_float4 texel = texture_unit0.sample(sampler0, input.out_texcoord);

	_float c = 0.8 * input.out_norm_age;

	_float4 frag_color;
#define HDR_SPACE 0
#if HDR_SPACE
	frag_color.xyz = color * texel.x * baked_ao;
	frag_color.w = texel.x * c * 0.08;
#else
	_float4 res = _float4(0.0) ;
	res.xyz = 2.0 * color * texel.x * baked_ao;
	res.w = texel.x * c * 0.2;

	res.x = powr(res.x, _float(0.45));
	res.y = powr(res.y, _float(0.45));
	res.z = powr(res.z, _float(0.45));

	frag_color = res;
#endif

	return frag_color;
}

#endif




