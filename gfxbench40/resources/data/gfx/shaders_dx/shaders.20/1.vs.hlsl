/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"

static const float2 wave0 = float2(  1.01, 1.08);
static const float2 wave1 = float2(  -1.02,   -1.02 );
static const float2 wave2 = float2( -1.03,  1.03 );
static const float2 wave3 = float2(  1.05,  -1.07 );

#ifndef ZPREPASS
VertexShaderOutput main(VertexShaderInput input)
{
	float4 tmp;

	float4 pos;	
#ifdef UBYTE_NORMAL_TANGENT
	float3 normal = input.normal * 2 - 1;
	float3 tangent = input.tangent * 2 - 1;
#else
	float3 normal = input.normal;
	float3 tangent = input.tangent;
#endif 
    VertexShaderOutput output;

#ifdef SKELETAL
		uint4 I = input.bone_index;
		float4 W = input.bone_weight;
		float4x3 B43 = bones[I.x] * W.x + bones[I.y] * W.y + bones[I.z] * W.z + bones[I.w] * W.w;

		pos     = float4( mul( float4( input.position, 1.0f) , B43 ), 1.0f );
		normal  =         mul( float4( normal,         0.0f) , B43 ), 1.0f  ;
		tangent =         mul( float4( tangent,        0.0f) , B43 ), 1.0f  ;
#else
		pos = float4(input.position, 1.0f) ;
#endif

    output.position = mul(pos, mvp);
    output.texcoord0 = input.texcoord0;

#if defined TRANSLATE_UV
	output.texcoord0 += (float2)translate_uv;
#endif

#ifdef LIGHTMAP
    output.texcoord1 = input.texcoord1;
#endif

#if defined LIGHTING || defined REFLECTION

	float4 world_position = mul( pos, model);

	output.view_dir = (float3)view_pos - (float3)world_position;

	tmp = mul( inv_model, float4( normal, 0.0f ));
	output.normal = (float3)tmp;
	
	tmp = mul( inv_model, float4( tangent, 0.0f ));
	output.tangent = (float3)tmp;

	#if defined SHADOW_MAP
		float4 shadow_texcoord0 = mul( world_position, shadow_matrix0);
		output.coord0 = shadow_texcoord0.xy / shadow_texcoord0.w;
		output.coord0z = shadow_texcoord0.z / shadow_texcoord0.w;
		float4 shadow_texcoord1 = mul( world_position, shadow_matrix1);
		output.coord1 = shadow_texcoord1.xy / shadow_texcoord1.w;
		output.coord1z = shadow_texcoord1.z / shadow_texcoord1.w;
	#endif

#endif
	
#if defined FOG
	float4 fog_position = mul( pos, mv);
	output.fog_distance = clamp( -fog_position.z * fog_density, 0.0f, 1.0f);
#endif

#ifdef ANIMATE_NORMAL
	output.animatecoord0 = output.texcoord0 * 1.3 + (4.0 * time) * wave0; 
	output.animatecoord1 = output.texcoord0 * 1.5 + (4.0 * time) * wave1;
	output.animatecoord2 = output.texcoord0 * 3.0 + (4.0 * time) * wave2;
	output.animatecoord3 = output.texcoord0 * 1.1 + (4.0 * time) * wave3;
#endif

    return output;
}

#else

VertexShaderOutput main(VertexShaderInput input)
{
	float4 tmp;

	float4 pos;	
	float3 normal = input.normal;
	float3 tangent = input.tangent;

    VertexShaderOutput output;

#ifdef SKELETAL
		uint4 I = input.bone_index;
		float4 W = input.bone_weight;
		float4x3 B43 = bones[I.x] * W.x + bones[I.y] * W.y + bones[I.z] * W.z + bones[I.w] * W.w;

		pos     = float4( mul( float4( input.position, 1.0f) , B43 ), 1.0f );
		normal  =         mul( float4( normal,         0.0f) , B43 ), 1.0f  ;
		tangent =         mul( float4( tangent,        0.0f) , B43 ), 1.0f  ;
#else
		pos = float4(input.position, 1.0f) ;
#endif

    output.position = mul(pos, mvp);
    output.texcoord0 = input.texcoord0;

#if defined TRANSLATE_UV
	output.texcoord0 += (float2)translate_uv;
#endif

#ifdef LIGHTMAP
    output.texcoord1 = input.texcoord1;
#endif

	output.tangent = float3( 1.0, 0.0, 0.0);
	output.normal = float3( 0.0, 1.0, 0.0);
	output.view_dir = float3( 0.0, 0.0, 1.0);

    return output;
}
#endif