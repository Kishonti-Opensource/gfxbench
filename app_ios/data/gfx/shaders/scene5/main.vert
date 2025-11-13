/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef SKELETAL
uniform float4x4 vp;
uniform float4x4 prev_vp;
#else
uniform float4x4 mvp;
uniform float4x4 prev_mvp;
uniform float4x4 model;
#endif


#ifdef SKELETAL
uniform float4 bones[MAX_BONES];
uniform float4 prev_bones[MAX_BONES];
#endif

in float3 in_position;
in float2 in_texcoord0_;
in float3 in_normal;
in float3 in_tangent;
#ifdef SKELETAL
in float4 in_bone_index;
in float4 in_bone_weight;
#endif

#ifdef DEBUG_MIPMAP_LEVEL
uniform float2 orig_texture_size;
out float2 texcoord_uv;
#endif

out float2 texcoord;
#ifdef TEXTURE_DENSITY
out float3 world_position;
#endif
out float3 normal;
out float3 tangent;
out float4 clip_space_pos;
out float4 clip_space_prev_pos;
void main()
{
#ifndef TEXTURE_DENSITY
	float3 world_position;
#endif

	float4 p = float4( in_position, 1.0);
	float4 n = float4( in_normal, 0.0);
	float4 t = float4( in_tangent, 0.0);

#ifdef SKELETAL
	float4x4 M4;
	float4x4 PREV_M4;
	{
		#define RESULT M4
		#define BONE bones
		int4 I = int4(in_bone_index);
		float4x4 M0 = float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M1 = float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M2 = float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M3 = float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;

		#define PREV_RESULT PREV_M4
		#define PREV_BONE prev_bones
		M0 = float4x4( PREV_BONE[I.x * 3 + 0],PREV_BONE[I.x * 3 + 1],PREV_BONE[I.x * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		M1 = float4x4( PREV_BONE[I.y * 3 + 0],PREV_BONE[I.y * 3 + 1],PREV_BONE[I.y * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		M2 = float4x4( PREV_BONE[I.z * 3 + 0],PREV_BONE[I.z * 3 + 1],PREV_BONE[I.z * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		M3 = float4x4( PREV_BONE[I.w * 3 + 0],PREV_BONE[I.w * 3 + 1],PREV_BONE[I.w * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		PREV_RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w;
	}

	world_position = (p * M4).xyz;
	normal = (n * M4).xyz;
	tangent = (t * M4).xyz;
	clip_space_pos = vp * float4(world_position, 1.0);
	clip_space_prev_pos = prev_vp * float4((p * PREV_M4).xyz, 1.0);
	gl_Position = clip_space_pos;
#else
	world_position = (model * p).xyz;
	normal = (model * n).xyz;
	tangent = (model * t).xyz;
	clip_space_prev_pos = prev_mvp * p;
	clip_space_pos = mvp * p;
	gl_Position = clip_space_pos;
#endif
	texcoord = in_texcoord0_;

#ifdef DEBUG_MIPMAP_LEVEL
	texcoord_uv = in_texcoord0_ * orig_texture_size / 8.0;
#endif
}
