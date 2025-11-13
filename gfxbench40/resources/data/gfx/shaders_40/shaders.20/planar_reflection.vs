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

attribute vec3 in_position;
attribute vec4 in_bone_index;
attribute vec4 in_bone_weight;
attribute vec3 in_normal;
attribute vec3 in_tangent;
attribute vec2 in_texcoord0;


varying vec2 out_texcoord0;


#if defined FOG
varying float fog_distance;
#endif	


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
	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

	position = in_position;

	gl_Position = mvp * vec4( position, 1);

	out_texcoord0 = in_texcoord0;

#if defined TRANSLATE_UV
	out_texcoord0 += translate_uv;
#endif


#if defined FOG
	vec4 fog_position = mv * vec4( position, 1.0);
	fog_distance = clamp( -fog_position.z * fog_density, 0.0, 1.0);
#endif
}
