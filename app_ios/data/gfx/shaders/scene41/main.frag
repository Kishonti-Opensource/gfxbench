/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define sampler2D hsampler2D
#define vec4 float4
#define vec2 float2
#define vec3 float3
#define mat3 float3x3

out float4 out_albedo { color(0) };
out float4 out_specular { color(1) };
out float4 out_normal { color(2) };
out float4 out_velocity { color(3) };

#endif

uniform sampler2D color_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;

uniform vec4 velocity_min_max_scale_factor;

// The max velocity value will be never bigger than 0.05
#define MAX_VELOCITY 0.05
vec4 pack_velocity(vec2 velocity)
{
	return vec4(velocity / (2.0 * MAX_VELOCITY) + 0.5, 0.0, 0.0);
}

vec2 velocity_function(vec4 pos, vec4 prev_pos, vec4 velocity_min_max_scale_factor)
{
	const float blur_strength = 0.5;

	vec2 a = pos.xy / pos.w;
	vec2 b = prev_pos.xy / prev_pos.w;

	vec2 delta = a - b;

	delta *= blur_strength;

	float delta_length = length(delta);

	#define EPS 0.0001
	if (delta_length < EPS)
	{
		delta = vec2(0.0, 0.0);
	}
	else
	{
#ifdef NGL_DX_NDC
		delta.y = -delta.y;
#endif
		delta = ( delta * clamp( delta_length, velocity_min_max_scale_factor.x, velocity_min_max_scale_factor.y ) ) / ( delta_length + EPS);
	}
	return delta;
}

mat3 calc_tbn(vec3 normal, vec3 tangent)
{
	vec3 n = normalize( normal);
	vec3 t = normalize( tangent);
	vec3 b = cross( t, n);
	b = normalize( b);
	return mat3( t, b, n);
}

vec3 calc_world_normal(vec3 tex_norm, vec3 normal, vec3 tangent)
{
	vec3 ts_normal = tex_norm * 2.0 - 1.0;

	mat3 tbn = calc_tbn( normal, tangent) ;

	// GL
	vec3 calc_normal = tbn * ts_normal;

	return normalize( calc_normal);
}

in vec3 normal;
in vec3 tangent;
in vec2 texcoord0;
in vec4 clip_space_pos;
in vec4 clip_space_prev_pos;
void main()
{
	vec4 albedo = texture(color_texture, texcoord0);

#ifdef ALPHA_TEST
	if( albedo.w < 0.5)
	{
		discard;
	}
#endif

#ifndef KSL_COMPILER
	// Albedo
	gl_FragData[0] = albedo;

	// Specular
	gl_FragData[1] = texture(specular_texture, texcoord0);

	// Normal
	vec3 tex_normal = texture(normal_texture, texcoord0).xyz;
	gl_FragData[2] = vec4(0.5 * calc_world_normal(tex_normal, normal, tangent) + 0.5, 1.0);

	// Velocity
	gl_FragData[3] = pack_velocity(velocity_function(clip_space_pos, clip_space_prev_pos,velocity_min_max_scale_factor));

#else
	out_albedo = albedo;

	// Specular
	out_specular = texture(specular_texture, texcoord0);

	// Normal
	vec3 tex_normal = texture(normal_texture, texcoord0).xyz;
	out_normal = float4(0.5 * calc_world_normal(tex_normal, normal, tangent) + 0.5, 1.0);

	// Velocity
	out_velocity = pack_velocity(velocity_function(clip_space_pos, clip_space_prev_pos,velocity_min_max_scale_factor));
#endif
}
