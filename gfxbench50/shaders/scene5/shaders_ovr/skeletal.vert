/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#define MAX_BONES 3*42 //comes from cpu side

uniform float4x4 vp; //only vp in case of skeletal
uniform float4x4 model;

uniform float4 bones[MAX_BONES];

in float3 in_position;

in float2 in_texcoord0_;
in float3 in_normal;
in float3 in_tangent;

in float4 in_bone_index;
in float4 in_bone_weight;

out float2 texcoord;
out float3 world_position;
out float3 normal;
out float3 tangent;


void main()
{
	float4 p = float4( in_position, 1.0);
	float4 n = float4( in_normal, 0.0);
	float4 t = float4( in_tangent, 0.0);
	
	float4x4 M4;
	{
		#define RESULT M4
		#define BONE bones
		int4 I = int4(in_bone_index);
		float4x4 M0 = float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M1 = float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M2 = float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M3 = float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	world_position = (p * M4 /** trans*/).xyz;
	normal = (n * M4).xyz;
	tangent = (t * M4).xyz;
	gl_Position = vp * float4(world_position,1.0);

	texcoord = in_texcoord0_;
}
