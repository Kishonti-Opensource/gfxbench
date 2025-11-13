/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/functions.hlsl"
#include "../include/types.hlsl"

float2 wave0 = float2( 1.01,  1.08);
float2 wave1 = float2(-1.02, -1.02);
float2 wave2 = float2(-1.03,  1.03);
float2 wave3 = float2( 1.05, -1.07);

VertexShaderOutput main(VertexShaderInput input)
{   
	VertexShaderOutput output;
	float3 position = input.position;

#ifdef UBYTE_NORMAL_TANGENT
	float3 normal = input.normal * 2 - 1;
	float3 tangent = input.tangent * 2 - 1;
#else
	float3 normal = input.normal;
	float3 tangent = input.tangent;
#endif

#ifdef SKELETAL
	uint4 I = input.bone_index;
	float4 W = input.bone_weight;
	float4x3 M43 = bones[I.x] * W.x + bones[I.y] * W.y + bones[I.z] * W.z + bones[I.w] * W.w;

	position = mul(float4(position, 1.0), M43).xyz;
	normal = mul(float4(normal, 0.0), M43);
	tangent = mul(float4(tangent, 0.0), M43);
#endif
	
	output.position = mul(float4(position, 1.0), mvp);
	output.texcoord0 = input.texcoord0;

#if defined TRANSLATE_UV
	output.texcoord0 += translate_uv;
#endif

	float4 world_position = mul(float4(position, 1.0), model);

	output.view_dir = view_pos.xyz - world_position.xyz;
	output.normal = mul(float4(normal, 0.0), inv_model).xyz;
	output.tangent = mul(float4(tangent, 0.0), inv_model).xyz;

#ifdef ANIMATE_NORMAL
	output.animatecoord0 = output.texcoord0 * 1.3 + (4.0 * time) * wave0;
	output.animatecoord0 = output.texcoord0 * 1.5 + (4.0 * time) * wave1;
	output.animatecoord0 = output.texcoord0 * 3.0 + (4.0 * time) * wave2;
	output.animatecoord0 = output.texcoord0 * 1.1 + (4.0 * time) * wave3;
#endif

	return output;
}
