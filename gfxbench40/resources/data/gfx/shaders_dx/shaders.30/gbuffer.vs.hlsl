/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer.hlsl"

float2 wave0 = float2( 1.01, 1.08);
float2 wave1 = float2(-1.02,-1.02);
float2 wave2 = float2(-1.03, 1.03);
float2 wave3 = float2( 1.05,-1.07);


GBufferStruct main(VertexShaderInput input)
{
	GBufferStruct output;

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
	normal = mul(float4(normal, 0.0), M43).xyz;
	tangent = mul(float4(tangent, 0.0), M43).xyz;
#endif

#ifdef INSTANCING
	output.position = mul( mul(float4(position, 1.0), input.instance_mv), mvp);
#else
	output.position = mul(float4(position, 1.0), mvp);
#endif
	
#if defined TRANSLATE_UV
	output.texcoord0 = input.texcoord0 + translate_uv.xy;
#else
	output.texcoord0 = input.texcoord0;
#endif

#ifdef INSTANCING
	output.worldPos = mul(float4(position,1.0), input.instance_mv).xyz;
	output.viewDir = view_pos.xyz - output.worldPos.xyz;
	output.normal = mul(float4(normal, 0.0), input.instance_inv_mv).xyz;
	output.tangent = mul(float4(tangent, 0.0), input.instance_inv_mv).xyz;
#else
	output.worldPos = mul(float4(position,1.0), model).xyz;
	output.viewDir = view_pos.xyz - output.worldPos.xyz;
	output.normal = mul(float4(normal, 0.0), inv_model).xyz;
	output.tangent = mul(float4(tangent, 0.0), inv_model).xyz;
#endif
	
	
#ifdef TRANSITION_EFFECT
	output.eyeSpaceNormal = mul(float4(normal, 0.0), inv_mv).xyz;
	output.worldPos = position.xyz * 0.3;
#else
	output.eyeSpaceNormal = float3(0, 0, 1);
#endif

#ifdef ANIMATE_NORMAL
	output.texcoord0 = output.texcoord0 * 1.3 + (4.0 * time) * wave0;
#endif

	return output;
}