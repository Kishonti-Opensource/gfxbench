/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mvp2;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;
#ifdef SKELETAL
uniform vec4 bones[3*SKELETAL];
uniform vec4 prev_bones[3*SKELETAL];
#endif
uniform vec3 view_pos;
uniform float time;
uniform float fog_density;
uniform vec3 light_pos;
uniform mat4 shadow_matrix;
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
varying vec3 out_texcoord4;
varying vec3 out_view_dir;
varying vec3 out_normal;
varying vec3 out_tangent;
varying float fog_distance;
varying vec2 out_texcoord01;
varying vec2 out_texcoord02;
varying vec2 out_texcoord03;
varying vec2 out_texcoord04;

//#define MBLUR_ALT

#ifdef MBLUR_ALT
varying vec2 out_diff;
#else
varying vec4 out_curpos;
varying vec4 out_prevpos;
#endif

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

void main()
{    
#ifdef MBLUR_ALT
	vec4 out_curpos;
	vec4 out_prevpos;
#endif

	vec4 tmp;
	vec3 position;
	vec3 prev_position;
	vec3 normal = in_normal;
	vec3 tangent = in_tangent;

	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

#ifdef SKELETAL
	mat4 M4;
	mat4 prev_M4;

#define RESULT M4
#define BONE bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }

	position = (vec4( in_position, 1.0) * M4).xyz;

#undef RESULT
#undef BONE

#define RESULT prev_M4
#define BONE prev_bones
	{ ivec4 I = ivec4(in_bone_index); mat4 M0 = mat4( BONE[I.x * 3 + 0],BONE[I.x * 3 + 1],BONE[I.x * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M1 = mat4( BONE[I.y * 3 + 0],BONE[I.y * 3 + 1],BONE[I.y * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M2 = mat4( BONE[I.z * 3 + 0],BONE[I.z * 3 + 1],BONE[I.z * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); mat4 M3 = mat4( BONE[I.w * 3 + 0],BONE[I.w * 3 + 1],BONE[I.w * 3 + 2],vec4( 0.0, 0.0, 0.0, 1.0)); RESULT = M0 * in_bone_weight.x + M1 * in_bone_weight.y + M2 * in_bone_weight.z + M3 * in_bone_weight.w; }

	prev_position = (vec4( in_position, 1.0) * prev_M4).xyz;

	out_curpos = mvp * vec4( position, 1.0);
	out_prevpos = mvp2 * vec4( prev_position, 1.0);

#else
	position = in_position;

	out_curpos = mvp * vec4( position, 1.0);
	out_prevpos = mvp2 * vec4( position, 1.0);

#endif


#ifdef MBLUR_ALT

	out_diff = (out_curpos.xy / out_curpos.w) - (out_prevpos.xy / out_prevpos.w);

#endif
	
	gl_Position = out_curpos;
}
