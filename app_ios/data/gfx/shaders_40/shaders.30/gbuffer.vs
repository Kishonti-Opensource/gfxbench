/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include cameraConsts;
	#include meshConsts;
	#include staticMeshConsts;
	#include translateConsts;
#else
	uniform mat4 mvp;
	uniform mat4 mv;
	uniform mat4 model;
	uniform mat4 inv_model;
	uniform mat4 inv_modelview;
	
	uniform vec3 view_pos;
	uniform float time;
	
	uniform vec2 translate_uv;
#endif

#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
#endif 


in vec3 in_position;
in vec4 in_bone_index;
in vec4 in_bone_weight;
in vec3 in_normal;
in vec3 in_tangent;
in vec2 in_texcoord0;
in vec2 in_texcoord1;

#ifdef INSTANCING
//in mat4 in_instance_mv;
in vec4 in_instance_mv0;
in vec4 in_instance_mv1;
in vec4 in_instance_mv2;
in vec4 in_instance_mv3;
in mat4 in_instance_inv_mv;
#endif

out vec2 out_texcoord0;
out vec2 out_texcoord1;
out vec3 out_texcoord4;
out vec3 out_view_dir;
out vec3 out_normal;
out vec3 out_tangent;
out vec2 out_texcoord01;
out vec2 out_texcoord02;
out vec2 out_texcoord03;
out vec2 out_texcoord04;
out vec3 out_eye_space_normal;
out vec3 out_world_pos;

vec2 wave0 = vec2(  1.01, 1.08);
vec2 wave1 = vec2(  -1.02,   -1.02 );
vec2 wave2 = vec2( -1.03,  1.03 );
vec2 wave3 = vec2(  1.05,  -1.07 );


void decodeFromByteVec3(inout vec3 myVec)
{
#ifdef UBYTE_NORMAL_TANGENT
	myVec = 2.0 * myVec - 1.0;
#endif
}

void main()
{    
	vec4 tmp;
	vec3 position;
	vec3 normal = in_normal;
	vec3 tangent = in_tangent;
	
#ifdef INSTANCING
	mat4 in_instance_mv = mat4(in_instance_mv0, in_instance_mv1, in_instance_mv2, in_instance_mv3);
#endif
	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

#ifdef SKELETAL
	mat4 M4;

#define RESULT M4
#define BONE bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }

	position = (vec4( in_position, 1.0) * M4).xyz;
	normal = (vec4( normal, 0.0) * M4).xyz;
	tangent = (vec4( tangent, 0.0) * M4).xyz;
#else
	position = in_position;
#endif

	mat4 mvp_1 = mvp;
#ifdef INSTANCING
    mat4 mvp2 = mvp_1 * in_instance_mv;
	gl_Position = mvp2 * vec4( position, 1.0);
#else
	gl_Position = mvp_1 * vec4( position, 1.0);
#endif

	out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV	
	#ifdef USE_UBOs
		out_texcoord0 += translate_uv_pad2.xy;
	#else
		out_texcoord0 += translate_uv;
	#endif
#endif


#ifdef INSTANCING
	vec4 world_position = in_instance_mv * vec4( position, 1.0);

	#ifdef USE_UBOs
		out_view_dir = view_posXYZ_normalized_time.xyz - world_position.xyz;
	#else
		out_view_dir = view_pos - world_position.xyz;
	#endif

	tmp = vec4( normal, 0.0) * in_instance_inv_mv;
	out_normal = tmp.xyz;

	tmp = vec4( tangent, 0.0) * in_instance_inv_mv;
	out_tangent = tmp.xyz;

#else
	//vec4 world_position = model * vec4( position, 1.0);
	mat4 model_1 = model;
	vec4 world_position = model_1 * vec4( position, 1.0);

	#ifdef USE_UBOs
		out_view_dir = view_posXYZ_normalized_time.xyz - world_position.xyz;
	#else
		out_view_dir = view_pos - world_position.xyz;
	#endif

	//tmp = vec4( normal, 0.0) * inv_model;
	mat4 inv_model_1 = inv_model;
	tmp = vec4( normal, 0.0) * inv_model_1;
	out_normal = tmp.xyz;

	//tmp = vec4( tangent, 0.0) * inv_model;
	tmp = vec4( tangent, 0.0) * inv_model_1;
	out_tangent = tmp.xyz;
#endif


#ifdef ANIMATE_NORMAL

	#ifdef USE_UBOs
		out_texcoord01 = out_texcoord0 * 1.3 + (4.0 * view_posXYZ_normalized_time.w) * wave0;
		out_texcoord02 = out_texcoord0 * 1.5 + (4.0 * view_posXYZ_normalized_time.w) * wave1;
		out_texcoord03 = out_texcoord0 * 3.0 + (4.0 * view_posXYZ_normalized_time.w) * wave2;
		out_texcoord04 = out_texcoord0 * 1.1 + (4.0 * view_posXYZ_normalized_time.w) * wave3;	
	#else
		out_texcoord01 = out_texcoord0 * 1.3 + (4.0 * time) * wave0;
		out_texcoord02 = out_texcoord0 * 1.5 + (4.0 * time) * wave1;
		out_texcoord03 = out_texcoord0 * 3.0 + (4.0 * time) * wave2;
		out_texcoord04 = out_texcoord0 * 1.1 + (4.0 * time) * wave3;
	#endif

#endif

#ifdef TRANSITION_EFFECT	
	//out_eye_space_normal = (vec4( normal, 0.0) * inv_modelview).xyz;
	mat4 inv_modelview_1 = inv_modelview;
	out_eye_space_normal = (vec4( normal, 0.0) * inv_modelview_1).xyz;
	
	out_world_pos = position.xyz * 0.3;
#endif
}
