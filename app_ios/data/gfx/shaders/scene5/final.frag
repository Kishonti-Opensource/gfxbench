/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> lighting_texture;
uniform sampler2D<float> gbuffer_depth_texture;
//uniform sampler2D<float> gbuffer_color_texture;
//uniform sampler2D<float> gbuffer_specular_texture;
//uniform sampler2D<float> gbuffer_normal_texture;

// Camera uniforms
uniform float4 view_pos;
uniform float4 depth_parameters;
uniform float4 inv_resolution;

// Bloom
uniform sampler2D<float> bloom_texture0;
#if !BLOOM_MOBILE
uniform sampler2D<float> bloom_texture1;
uniform sampler2D<float> bloom_texture2;
uniform sampler2D<float> bloom_texture3;
#endif
uniform float4 bloom_parameters;

// Color correction
uniform float4 sharpen_filter;

in float2 texcoord;
in float4 out_view_dir;
out float4 out_res { color(0) };
void main()
{
	float3 ldr_color;
	float3 linear_color = texture( lighting_texture, texcoord).xyz;

#if SHARPEN_FILTER
	float3 neighboors;
	neighboors  = texture( lighting_texture, texcoord + float2(inv_resolution.x, 0.0)).xyz;
	neighboors += texture( lighting_texture, texcoord - float2(inv_resolution.x, 0.0)).xyz;
	neighboors += texture( lighting_texture, texcoord + float2(0.0, inv_resolution.y)).xyz;
	neighboors += texture( lighting_texture, texcoord - float2(0.0, inv_resolution.y)).xyz;

	float3 weighted_color = linear_color * sharpen_filter.x - neighboors * sharpen_filter.y;
	linear_color = linear_color + clamp(weighted_color, -sharpen_filter.z, sharpen_filter.z);
#endif

	float fog_amount = 0.0;
	float depth = texture(gbuffer_depth_texture, texcoord).x;
	if( depth != 1.0)
	{
		float3 view_dir = out_view_dir.xyz / out_view_dir.w;
		float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
		view_dir = normalize(view_dir);

		float3 view_to_pos = world_pos - view_pos.xyz;
		float ray_length = length(view_to_pos);

		fog_amount = get_fog_amount(world_pos, view_pos.xyz, ray_length);
	}

	// Color correction
	linear_color = apply_color_correction(linear_color);

	// Tonemapping
	ldr_color = tonemapper(linear_color, hdr_exposure.y);

#if BLOOM_ENABLED
	// Apply bloom
	#if BLOOM_MOBILE
		float3 bloom = textureLod(bloom_texture0, texcoord, 0.0).xyz;
	#else
		float3 bloom0 = textureLod(bloom_texture0, texcoord, 0.0).xyz;
		float3 bloom1 = textureLod(bloom_texture1, texcoord, 0.0).xyz;
		float3 bloom2 = textureLod(bloom_texture2, texcoord, 0.0).xyz;
		float3 bloom3 = textureLod(bloom_texture3, texcoord, 0.0).xyz;
		float3 bloom = (bloom0 + bloom1 + bloom2 + bloom3) * 0.25;
	#endif

	bloom = bloom * bloom * bloom_parameters.y;
	ldr_color = ldr_color + bloom;
#endif

	// Apply fog
	ldr_color = mix(ldr_color, fog_color.xyz, fog_amount);

	// Gamma correction
#if GAMMA_ENABLED
	ldr_color = clamp(ldr_color, float3(0.0), float3(1.0));
	ldr_color = linear_to_srgb(ldr_color);
#endif

	out_res = float4(ldr_color, 1.0);
}
