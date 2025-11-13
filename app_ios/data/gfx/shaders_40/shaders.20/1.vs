/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;
#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
#endif
uniform vec3 view_pos;
uniform float time;
uniform float fog_density;
uniform vec3 light_pos;
uniform mat4 shadow_matrix0;
uniform mat4 shadow_matrix1;
uniform vec4 clip_plane;
uniform vec2 translate_uv;
uniform float camera_focus;


attribute vec3 in_position;
attribute vec4 in_bone_index;
attribute vec4 in_bone_weight;
attribute vec3 in_normal;
attribute vec3 in_tangent;
attribute vec2 in_texcoord0;
attribute vec2 in_texcoord1;


varying vec2 out_texcoord0;
varying vec2 out_texcoord1;
varying vec2 out_texcoord4;
varying vec2 out_texcoord5;
varying float out_texcoord4z;
varying float out_texcoord5z;
varying vec3 out_view_dir;
varying vec3 out_normal;
varying vec3 out_tangent;
varying float fog_distance;
varying vec2 out_texcoord01;
varying vec2 out_texcoord02;
varying vec2 out_texcoord03;
varying vec2 out_texcoord04;

varying float dof;

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

#ifndef ZPREPASS

void main()
{    
	vec4 tmp;
	vec3 position;
	vec3 normal = in_normal;
	vec3 tangent = in_tangent;

	
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


	gl_Position = mvp * vec4( position, 1.0);

	out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV
	out_texcoord0 += translate_uv;
#endif


#ifdef LIGHTMAP
	out_texcoord1 = in_texcoord1;
#endif

#if defined LIGHTING || defined REFLECTION || defined RELIEF

	vec4 world_position = model * vec4( position, 1.0);

	out_view_dir = view_pos - world_position.xyz;

	tmp = vec4( normal, 0.0) * inv_model;
	out_normal = tmp.xyz;

	tmp = vec4( tangent, 0.0) * inv_model;
	out_tangent = tmp.xyz;


#if defined SHADOW_MAP
	vec4 shadow_texcoord0 = shadow_matrix0 * world_position;
	out_texcoord4 = shadow_texcoord0.xy / shadow_texcoord0.w;
	out_texcoord4z = shadow_texcoord0.z / shadow_texcoord0.w;

	vec4 shadow_texcoord1 = shadow_matrix1 * world_position;
	out_texcoord5 = shadow_texcoord1.xy / shadow_texcoord1.w;
	out_texcoord5z = shadow_texcoord1.z / shadow_texcoord1.w;
#endif

#endif


#if defined FOG
	vec4 fog_position = mv * vec4( position, 1.0);
	fog_distance = clamp( -fog_position.z * fog_density, 0.0, 1.0);
#endif

#if defined DOF
	vec4 distance_position = mv * vec4( position, 1.0);
	dof = clamp( 1.0 - abs( -distance_position.z - camera_focus) / 20.0, -1.0, 1.0);
#endif

#ifdef ANIMATE_NORMAL
	out_texcoord01 = out_texcoord0 * 1.3 + (4.0 * time) * wave0;
	out_texcoord02 = out_texcoord0 * 1.5 + (4.0 * time) * wave1;
	out_texcoord03 = out_texcoord0 * 3.0 + (4.0 * time) * wave2;
	out_texcoord04 = out_texcoord0 * 1.1 + (4.0 * time) * wave3;
#endif
}

#else

void main()
{    
	vec4 tmp;
	vec3 position;

#ifdef SKELETAL
	mat4 M4;

#define RESULT M4
#define BONE bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }


	position = (vec4( in_position, 1.0) * M4).xyz;
#else
	position = in_position;
#endif


	gl_Position = mvp * vec4( position, 1.0);

	out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV
	out_texcoord0 += translate_uv;
#endif

	out_tangent = vec3( 1.0, 0.0, 0.0);
	out_normal = vec3( 0.0, 1.0, 0.0);
	out_view_dir = vec3( 0.0, 0.0, 1.0);

#ifdef ANIMATE_NORMAL
	out_texcoord01 = out_texcoord0 * 1.3 + (4.0 * time) * wave0;
	out_texcoord02 = out_texcoord0 * 1.5 + (4.0 * time) * wave1;
	out_texcoord03 = out_texcoord0 * 3.0 + (4.0 * time) * wave2;
	out_texcoord04 = out_texcoord0 * 1.1 + (4.0 * time) * wave3;
#endif
}

#endif
