/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;
uniform vec3 view_pos;
uniform float time;
uniform float fog_density;
uniform vec3 light_pos;
uniform mat4 shadow_matrix0;
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
	gl_Position = mvp * vec4( in_position, 1.0);

	out_texcoord0 = in_texcoord0;
	out_texcoord1 = in_texcoord1;
}
