/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<float> gbuffer_color_texture { color(JOB_ATTACHMENT0) };
uniform subpassInput<float> gbuffer_normal_texture { color(JOB_ATTACHMENT1) };
uniform subpassInput<float> gbuffer_specular_texture { color(JOB_ATTACHMENT2) };

#if SHADER_METAL_IOS && defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
uniform subpassInput<float> gbuffer_depth_texture { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#else
uniform subpassInput<float> gbuffer_depth_texture { color(JOB_ATTACHMENT4) };
#endif

uniform float4 light_color;
uniform float4 light_dir;

uniform float4 view_pos;
uniform float4 depth_parameters;

in float4 out_view_dir;
in float4 out_pos;
in float2 out_texcoord;
out float4 out_color { color(DIRECT_LIGHT_RT_INDEX) };
void main()
{
	float depth = subpassLoad(gbuffer_depth_texture).x;
	/*
	if(depth == 1.0)
	{
		discard;
	}
	*/

	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
	view_dir = normalize(view_dir);

	// Material params
	float3 albedo = subpassLoad(gbuffer_color_texture).xyz;
	float3 specular_color = subpassLoad(gbuffer_specular_texture).xyz;
	albedo = srgb_to_linear(albedo);
	specular_color = srgb_to_linear(specular_color);

	albedo = get_energy_conservative_albedo(albedo, specular_color);

	float4 tex_norm_gloss = subpassLoad(gbuffer_normal_texture);
	float3 normal = decode_normal(tex_norm_gloss.xyz);

	float roughness = 1.0 - tex_norm_gloss.w;

	half shadow = 1.0h;
#ifdef HAS_SHADOW
	shadow = get_sun_shadow(depth, world_pos);
#endif

	// Shading
	float3 diffuse = direct_diffuse(albedo, normal, light_dir.xyz, light_color.xyz, 1.0);
	float3 specular = direct_specular(specular_color, roughness, normal, -view_dir, light_dir.xyz, light_color.xyz, 1.0);

	float3 color = merge_direct_lighting(diffuse, specular, float(shadow));

	// Emissive pass
	//float4 emissive_color = subpassLoad(gbuffer_emissive_texture);
	//color += emissive_color.xyz * emissive_color.w * 16.0;

	out_color = float4(color, 1.0);
}