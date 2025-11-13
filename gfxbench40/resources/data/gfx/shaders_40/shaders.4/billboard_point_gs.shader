/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform mat4 mvp;
uniform mat4 view;

uniform sampler2D texture_unit0;
uniform sampler2D texture_unit7;

#define LIFETIME (1.1)

#ifdef TYPE_vertex

in vec3 in_position;
in vec2 in_texcoord;

out vec2 Age_Speed_Accel;

void main()
{
	vec3 p = in_position;
	p.y -= 0.372;
	gl_Position = vec4( p, 1.0);
	Age_Speed_Accel = in_texcoord;
}

#endif


#ifdef TYPE_geometry

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in vec2 Age_Speed_Accel[];

flat out float out_norm_age;
out vec2 out_texcoord;
out vec3 out_worldpos;

void main()
{
	vec3 up;
	vec3 right;

	float norm_age = 1.0 - Age_Speed_Accel[0].x / LIFETIME;

	up = vec3( view[0][1], view[1][1], view[2][1]);
	right = vec3( view[0][0], view[1][0], view[2][0]);

	float c = cos( float(gl_PrimitiveIDIn) + Age_Speed_Accel[0].x);
	float s = sin( float(gl_PrimitiveIDIn) + norm_age);

	vec3 vUpNew    = c * right + s * up;
	vec3 vRightNew = s * right - c * up;

	up = vUpNew;
	right = vRightNew;

	float size = mix( 0.6, 1.0, Age_Speed_Accel[0].x);

	vec3 position0 = gl_in[0].gl_Position.xyz + ( -1.0 * right + -1.0 * up) * size;
	vec3 position1 = gl_in[0].gl_Position.xyz + ( 1.0 * right + -1.0 * up) * size;
	vec3 position2 = gl_in[0].gl_Position.xyz + ( -1.0 * right + 1.0 * up) * size;
	vec3 position3 = gl_in[0].gl_Position.xyz + ( 1.0 * right + 1.0 * up) * size;
	vec2 texcoord0 = vec2( 0.0, 0.0);
	vec2 texcoord1 = vec2( 1.0, 0.0);
	vec2 texcoord2 = vec2( 0.0, 1.0);
	vec2 texcoord3 = vec2( 1.0, 1.0);

	gl_Position = mvp * vec4(position0, 1.0);
	out_norm_age = norm_age;
	out_texcoord = texcoord0;
	out_worldpos = position0;
	EmitVertex();

	gl_Position = mvp * vec4(position1, 1.0);
	out_norm_age = norm_age;
	out_texcoord = texcoord1;
	out_worldpos = position1;
	EmitVertex();

	gl_Position = mvp * vec4(position2, 1.0);
	out_norm_age = norm_age;
	out_texcoord = texcoord2;
	out_worldpos = position2;
	EmitVertex();

	gl_Position = mvp * vec4(position3, 1.0);
	out_norm_age = norm_age;
	out_texcoord = texcoord3;
	out_worldpos = position3;
	EmitVertex();

	EndPrimitive();
}

#endif


#ifdef TYPE_fragment


#include "rgbm_helper.h"

flat in float out_norm_age;
in vec2 out_texcoord;
in vec3 out_worldpos;

out vec4 frag_color;

void main()
{
	/*highp*/ vec2 uv = out_worldpos.xz / 1950.0;
	uv += vec2(-0.5, 0.5);
	float baked_ao = texture(texture_unit7, uv).y;
	baked_ao = clamp( baked_ao + 0.2, 0.0, 1.0);

	vec3 color = vec3( 0.6, 0.4, 0.2);
	vec4 texel = texture( texture_unit0, out_texcoord);

	float c = 0.8 * out_norm_age;

#define HDR_SPACE 0
#if HDR_SPACE
	frag_color.xyz = color * texel.x * baked_ao;
	frag_color.w = texel.x * c * 0.08;
#else
	vec4 res = vec4(0.0) ;
	res.xyz = 2.0 * color * texel.x * baked_ao;
	res.w = texel.x * c * 0.2;

	res.x = pow(res.x, 0.45);
	res.y = pow(res.y, 0.45);
	res.z = pow(res.z, 0.45);

	frag_color = res ;
#endif

}

#endif
