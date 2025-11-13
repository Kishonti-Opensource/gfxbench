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

uniform float4 view_pos;
uniform float4 depth_parameters;

uniform float4 light_pos;
uniform float4 light_dir;
uniform float4 light_color;
uniform float2 spot_cos;
uniform float4 attenuation_parameters;

#ifdef HAS_SHADOW
uniform float4x4 shadow_matrix;
uniform sampler2DShadow<float> shadow_map;
#endif

in float4 out_view_dir;
in float4 out_pos;
out float4 out_color { color(LIGHTING_RT_INDEX) };
void main()
{
	float2 tc = (out_pos.xy / out_pos.w * 0.5) + float2(0.5, 0.5);

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

	float3 L = light_pos.xyz - world_pos;
	float L_length2 = dot(L, L);

	// Distance attenuation
	float atten = 1.0 - L_length2 * attenuation_parameters.x;
	if(atten <= 0.0)
	{
		discard;
	}

	L = L / sqrt(L_length2);

	atten = pow(atten, attenuation_parameters.y);

	// FOV attenuation
	float fov_atten = dot(L, light_dir.xyz);
	fov_atten = fov_atten - spot_cos.x;
	if(fov_atten <= 0.0)
	{
		discard;
	}

	fov_atten = clamp( fov_atten, 0.0, 1.0);
	fov_atten *= spot_cos.y;
	fov_atten = pow(fov_atten, attenuation_parameters.z);
	atten *= fov_atten;

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
	float4 shadow_coords = shadow_matrix * float4(world_pos, 1.0);
	shadow_coords.xyz /= shadow_coords.w;
	shadow = get_linear_shadow(shadow_map, shadow_coords.xyz);
#endif

	// Shading
	float3 diffuse = direct_diffuse(albedo, normal, L, light_color.xyz, atten);
	float3 specular = direct_specular(specular_color, roughness, normal, -view_dir, L, light_color.xyz, atten);

	float3 color = merge_direct_lighting(diffuse, specular, float(shadow));

	out_color = float4(color, 1.0);
}
