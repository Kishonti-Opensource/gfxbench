/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define sampler2D hsampler2D
#define vec4 float4
#define vec3 float3
#define vec2 float2
#define mat4 float4x4

out float4 out_res { color(0) };

#endif

uniform sampler2D gbuffer_color_texture;
uniform sampler2D gbuffer_specular_texture;
uniform sampler2D gbuffer_normal_texture;
uniform sampler2D gbuffer_depth_texture;

uniform vec4 view_pos;
uniform vec4 depth_parameters;

uniform vec4 light_dir;

uniform sampler2D shadow_map;
uniform mat4 shadow_matrix0;
uniform mat4 shadow_matrix1;
uniform mat4 shadow_matrix2;
uniform mat4 shadow_matrix3;

vec3 get_world_pos( float depth_, vec4 depth_parameters_, vec3 view_dir_, vec3 view_pos_)
{
	float d = depth_parameters_.y / (depth_ - depth_parameters_.x);

	return d * view_dir_ + view_pos_.xyz;
}

in vec4 out_view_dir;
in vec2 texcoord;
void main()
{
	vec3 albedo = texture( gbuffer_color_texture, texcoord).xyz;
	vec3 specular = texture( gbuffer_specular_texture, texcoord).xyz;
	vec3 normal = texture( gbuffer_normal_texture, texcoord).xyz;
	float depth = texture( gbuffer_depth_texture, texcoord).x;

	vec3 res;
	if (depth == 1.0)
	{
		// Sky
		res = albedo * 3.0;
	}
	else
	{
		vec3 view_dir = out_view_dir.xyz / out_view_dir.w;
		vec3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
		view_dir = normalize(view_dir);

		// Hack some basic shading
		normal = 2.0 * normal.xyz - 1.0;
		normal = normalize(normal);

		float NdotL = clamp(dot(normal, light_dir.xyz), 0.0, 1.0);
		float light_intensity = NdotL * 2.0;

		vec3 ambient_light = albedo * vec3(0.01, 0.01, 0.02) * clamp(dot(normal, -light_dir.xyz), 0.0, 1.0) * 2.0;

		// Shadow
		float shadow = 1.0;
		{
			// Select the appropriate shadow cascade
			vec4 world_pos4 = vec4(world_pos, 1.0);
			vec4 shadow_coords0 = shadow_matrix0 * world_pos4;
			vec4 shadow_coords1 = shadow_matrix1 * world_pos4;
			vec4 shadow_coords2 = shadow_matrix2 * world_pos4;
			vec4 shadow_coords3 = shadow_matrix3 * world_pos4;

			bool b0 = min(shadow_coords0.x, shadow_coords0.y) >= 0.0 && max(shadow_coords0.x, shadow_coords0.y) < 1.0;
			bool b1 = min(shadow_coords1.x, shadow_coords1.y) >= 0.0 && max(shadow_coords1.x, shadow_coords1.y) < 1.0;
			bool b2 = min(shadow_coords2.x, shadow_coords2.y) >= 0.0 && max(shadow_coords2.x, shadow_coords2.y) < 1.0;
			bool b3 = min(shadow_coords3.x, shadow_coords3.y) >= 0.0 && max(shadow_coords3.x, shadow_coords3.y) < 1.0;

			float shadow_depth = 1.0;
			float compare_depth = 1.0;
			float bias = 0.0;
			if ( b0)
			{
				shadow_depth = texture(shadow_map, shadow_coords0.xy * 0.5).x;
				compare_depth = shadow_coords0.z;
				bias = 0.0005;
			}
			else if ( b1)
			{
				shadow_depth = texture(shadow_map, shadow_coords1.xy * 0.5 + vec2(0.5, 0.0)).x;
				compare_depth = shadow_coords1.z;
				bias = 0.0005;
			}
			else if ( b2)
			{
				shadow_depth = texture(shadow_map, shadow_coords2.xy * 0.5 + vec2(0.0, 0.5)).x;
				compare_depth = shadow_coords2.z;
				bias = 0.0005;
			}
			else if ( b3)
			{
				shadow_depth = texture(shadow_map, shadow_coords3.xy * 0.5 + vec2(0.5, 0.5)).x;
				compare_depth = shadow_coords3.z;
				bias = 0.005;
			}

			bool in_shadow = shadow_depth < clamp(compare_depth - bias, 0.0, 1.0);
			if(in_shadow)
			{
				shadow = 0.20;
			}

#if 0
			// Debug shadow cascades
			gl_FragData[0] = vec4(albedo, 1.0);
			if ( b0)
			{
				gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);
			}
			else if ( b1)
			{
				gl_FragData[0] = vec4(0.0, 1.0, 0.0, 1.0);
			}
			else if ( b2)
			{
				gl_FragData[0] = vec4(0.0, 0.0, 1.0, 1.0);
			}
			else if ( b3)
			{
				gl_FragData[0] = vec4(1.0, 0.0, 1.0, 1.0);
			}
			return;
#endif
		}

		res = albedo * shadow * light_intensity + ambient_light;
	}

	// Gamma correction
#if GAMMA_ENABLED
	res = max(res, vec3(0.0, 0.0, 0.0));
	const float a = 1.0 / 2.2;
	res = pow(res, vec3(a, a, a));
#endif

#ifdef KSL_COMPILER
	out_res = float4(res, 1.0);
#else
	gl_FragData[0] = vec4(res, 1.0);
#endif
	
}
