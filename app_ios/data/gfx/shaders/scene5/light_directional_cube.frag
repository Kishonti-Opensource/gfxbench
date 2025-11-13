/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<half> gbuffer_color_texture { color(JOB_ATTACHMENT0) };
uniform subpassInput<half> gbuffer_normal_texture { color(JOB_ATTACHMENT1) };
uniform subpassInput<half> gbuffer_specular_texture { color(JOB_ATTACHMENT2) };

#if SHADER_METAL_IOS && defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
uniform subpassInput<float> gbuffer_depth_texture { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#else
uniform subpassInput<float> gbuffer_depth_texture { color(JOB_ATTACHMENT4) };
#endif

#ifdef HAS_SHADOW
uniform float4x4 shadow_matrix;
uniform sampler2DShadow<float> shadow_map;
#endif

uniform float4 light_color;
uniform float4 light_dir;

uniform float4 view_pos;
uniform float4 depth_parameters;

in float4 out_view_dir;
in float4 out_pos;
//in float2 out_texcoord;
out half4 out_color { color(LIGHTING_RT_INDEX) };
void main()
{
	float2 tc = (out_pos.xy / out_pos.w * 0.5) + 0.5;

	float depth = subpassLoad(gbuffer_depth_texture).x;
	/*
	if(depth == 1.0)
	{
		discard;
	}
	*/

	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);

#ifdef HAS_SHADOW
	float4 shadow_coords = shadow_matrix * float4(world_pos, 1.0);
	if (shadow_coords.x < 0.0 || shadow_coords.x > 1.0 || shadow_coords.y < 0.0 || shadow_coords.y > 1.0 || shadow_coords.z < 0.0 || shadow_coords.z > 1.0)
	{
		discard;
	}
	half shadow = get_linear_shadow(shadow_map, shadow_coords.xyz);
#else
	const half shadow = 1.0h;
#endif

	view_dir = normalize(view_dir);

	// Material params
	half3 albedo = subpassLoad(gbuffer_color_texture).xyz;
	half3 specular_color = subpassLoad(gbuffer_specular_texture).xyz;
	albedo = srgb_to_linear_half(albedo);
	specular_color = srgb_to_linear_half(specular_color);

	albedo = get_energy_conservative_albedo(albedo, specular_color);

	half4 tex_norm_gloss = subpassLoad(gbuffer_normal_texture);
	half3 normal = decode_normal(tex_norm_gloss.xyz);

	half roughness = 1.0h - tex_norm_gloss.w;

	// Shading
	half3 diffuse = direct_diffuse(albedo, normal, half3(light_dir.xyz), half3(light_color.xyz), 1.0h);

	// PREC_TODO
	float3 specular = direct_specular(float3(specular_color), float(roughness), float3(normal), -view_dir, light_dir.xyz, light_color.xyz, 1.0);
	half3 color = half3(merge_direct_lighting(float3(diffuse), specular, float(shadow)));

	out_color = half4(color, 1.0h);
}