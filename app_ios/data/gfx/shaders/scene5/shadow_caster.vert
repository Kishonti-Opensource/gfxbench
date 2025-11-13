/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if PARABOLOID
	#ifdef SKELETAL
		uniform float4x4 view;
	#else
		uniform float4x4 mv;
	#endif
	uniform float4 depth_parameters; // .x - paraboloid index, .z - near, .w - far
#else
	#ifdef SKELETAL
		uniform float4x4 vp;
	#else
		uniform float4x4 mvp;
	#endif
#endif

#ifdef SKELETAL
uniform float4 bones[MAX_BONES];
float4 get_skeletal(float3 pos, float4 indices, float4 weights)
{
	float4x4 M4;
	{
		#define RESULT M4
		#define BONE bones
		int4 I = int4(indices);
		float4x4 M0 = float4x4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M1 = float4x4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M2 = float4x4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		float4x4 M3 = float4x4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],float4( 0.0, 0.0, 0.0, 1.0));
		RESULT = M0 * weights.x + M1 * weights.y + M2 * weights.z + M3 * weights.w;
	}

	return float4(pos, 1.0) * M4;
}
#endif

in float3 in_position;
#ifdef ALPHA_TEST
in float2 in_texcoord0_;
#endif
#ifdef SKELETAL
in float4 in_bone_index;
in float4 in_bone_weight;
#endif

#ifdef ALPHA_TEST
out float2 texcoord0;
#endif
#if PARABOLOID
out float v_side;
#endif
void main()
{
#if PARABOLOID
	#ifdef SKELETAL
		float4 sc_pos = view * get_skeletal(in_position, in_bone_index, in_bone_weight);
	#else
		float4 sc_pos = mv * float4(in_position, 1.0);
	#endif

	sc_pos.z = sc_pos.z * depth_parameters.x;

	v_side = sc_pos.z; //verts falling to the "other" parab get negative here, cullable in PS

	float L = length(sc_pos.xyz);
	sc_pos = sc_pos / L;
	
	sc_pos.z = sc_pos.z + 1.0;
	sc_pos.x = sc_pos.x / sc_pos.z;
	sc_pos.y = sc_pos.y / sc_pos.z;	
	
	sc_pos.z = (L - depth_parameters.z) / (depth_parameters.w - depth_parameters.z);//for proper depth sorting
	
	sc_pos += normalize(sc_pos) * 0.01; //small bias to remove seam
	sc_pos.z = sc_pos.z * 2.0 - 1.0; // fit to clipspace	
	
	sc_pos.w = 1.0;	

    gl_Position = sc_pos;

#else
	#ifdef SKELETAL	
		gl_Position = vp * get_skeletal(in_position, in_bone_index, in_bone_weight);
	#else
		gl_Position = mvp * float4(in_position, 1.0);
	#endif
#endif

#ifdef ALPHA_TEST
	texcoord0 = in_texcoord0_;
#endif

/*
	// "pancaking"
#ifdef NGL_ZERO_TO_ONE
	gl_Position.z = max(gl_Position.z, 0.0);
#else
	gl_Position.z = max(gl_Position.z, -1.0);
#endif
*/
}
