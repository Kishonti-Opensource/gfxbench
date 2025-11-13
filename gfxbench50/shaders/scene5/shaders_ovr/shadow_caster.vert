/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4x4 vp;
uniform float4x4 mvp;
#ifdef SKELETAL
uniform float4 bones[MAX_BONES];
#endif

in float3 in_position;
in float2 in_texcoord0_;
#ifdef SKELETAL
in float4 in_bone_index;
in float4 in_bone_weight;
#endif

out float2 texcoord0;
void main()
{
	float4 pos = float4(in_position, 1.0);
#ifdef SKELETAL
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

	float4 world_pos = (pos * M4);
	gl_Position = vp * world_pos;
#else

	gl_Position = mvp * pos;
#endif

	texcoord0 = in_texcoord0_;

	// "pancaking"
#ifdef NGL_DX_NDC
	gl_Position.z = max(gl_Position.z, 0.0);
#else
	gl_Position.z = max(gl_Position.z, -1.0);
#endif
}
