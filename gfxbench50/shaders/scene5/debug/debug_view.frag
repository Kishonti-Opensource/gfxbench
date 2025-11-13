/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> gbuffer_color_texture;
uniform sampler2D<float> gbuffer_specular_texture;
uniform sampler2D<float> gbuffer_emissive_texture;
uniform sampler2D<float> gbuffer_normal_texture;
uniform sampler2D<float> gbuffer_velocity_texture;
uniform sampler2D<float> gbuffer_depth_texture;

uniform sampler2D<float> depth_downsample_texture;
uniform sampler2D<float> ssao_texture;
uniform sampler2D<float> bright_texture0;
uniform sampler2D<float> bright_texture1;
uniform sampler2D<float> bright_texture2;
uniform sampler2D<float> bright_texture3;
uniform sampler2D<float> bloom_texture0;
uniform sampler2D<float> bloom_texture1;
uniform sampler2D<float> bloom_texture2;
uniform sampler2D<float> bloom_texture3;
uniform sampler2D<float> neighbor_max_texture;
uniform sampler2D<float> motion_blur_texture;
uniform sampler2D<float> dof_input_texture;
uniform sampler2D<float> dof_blur_texture;
uniform sampler2D<float> half_res_transparent_texture;

uniform float4 depth_parameters;

#if DEBUG_VIEW_DIRECT_SHADOW
#if DEBUG_SHADOW_ARRAY
uniform sampler2DArray<float> shadow_map;
#elif DEBUG_SHADOW_CUBE
uniform samplerCube<float> shadow_map;
#else
uniform sampler2D<float> shadow_map;
#endif
#endif


float sample_shadow_map(sampler2D<float> shadow_texture, float2 uv)
{
	return texture(shadow_texture, uv).x;
}


float sample_shadow_map(sampler2DArray<float> shadow_texture, float2 uv)
{
	if (uv.x < 0.5 && uv.y < 0.5)
	{
		uv = float2(uv.x * 2.0, uv.y * 2.0);
		return texture(shadow_texture, float3(uv, 0.0)).x;
	}

	if (uv.x >= 0.5 && uv.y < 0.5)
	{
		uv = float2(uv.x * 2.0 - 1.0, uv.y * 2.0);
		return texture(shadow_texture, float3(uv, 1.0)).x;
	}

	if (uv.x < 0.5 && uv.y >= 0.5)
	{
		uv = float2(uv.x * 2.0, uv.y * 2.0 - 1.0);
		return texture(shadow_texture, float3(uv, 2.0)).x;
	}

	uv = float2(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0);
	return texture(shadow_texture, float3(uv, 3.0)).x;
}


bool in_region(float4 r, float2 v)
{
	if (v.x < r.x) return false;
	if (v.x > r.y) return false;
	if (v.y < r.z) return false;
	if (v.y > r.w) return false;
	return true;
}


float2 map_region(float4 r, float2 v)
{
	float2 a = float2( (v.x - r.x) / (r.y - r.x), (v.y - r.z)/ (r.w - r.z));
	a *= 2.0;
	a -= 1.0;
	return a;
}


float sample_shadow_map(samplerCube<float> shadow_texture, float2 uv)
{
	float4 r1 = float4(0.0,     1.0/3.0, 0.0,     1.0/2.0);
	float4 r2 = float4(0.0,     1.0/3.0, 1.0/2.0, 2.0/2.0);

	float4 r3 = float4(1.0/3.0, 2.0/3.0, 0.0,     1.0/2.0);
	float4 r4 = float4(1.0/3.0, 2.0/3.0, 1.0/2.0, 2.0/2.0);

	float4 r5 = float4(2.0/3.0, 3.0/3.0, 0.0,     1.0/2.0);
	float4 r6 = float4(2.0/3.0, 3.0/3.0, 1.0/2.0, 2.0/2.0);

	float d = 0.0;

	if (in_region(r1,uv))
	{
		float2 tc = map_region(r1,uv);
		d = texture(shadow_texture, float3(tc,-1.0) ).x;
	}

	if (in_region(r2,uv))
	{
		float2 tc = map_region(r2,uv);
		d = texture(shadow_texture, float3(tc,1.0) ).x;
	}

	if (in_region(r3,uv))
	{
		float2 tc = map_region(r3,uv);
		d = texture(shadow_texture, float3(tc.x,-1.0,tc.y) ).x;
	}

	if (in_region(r4,uv))
	{
		float2 tc = map_region(r4,uv);
		d = texture(shadow_texture, float3(tc.x,1.0,tc.y) ).x;
	}

	if (in_region(r5,uv))
	{
		float2 tc = map_region(r5,uv);
		d = texture(shadow_texture, float3(-1.0,tc) ).x;
	}

	if (in_region(r6,uv))
	{
		float2 tc = map_region(r6,uv);
		d = texture(shadow_texture, float3(1.0,tc) ).x;
	}

	return d;
}

float4 sample_bloom_layers(sampler2D<float> t0, sampler2D<float> t1, sampler2D<float> t2, sampler2D<float> t3, float2 uv)
{
	bool a1 = (uv.x < 0.5) && (uv.y < 0.5);
	bool a2 = (uv.x > 0.5) && (uv.y < 0.5);
	bool a3 = (uv.x < 0.5) && (uv.y > 0.5);
	if (a1)
	{
		float2 tc = 2.0 * uv;
		return textureLod( t0, tc, 0.0);
	}
	else if(a2)
	{
		float2 tc = 2.0 * (uv + float2(-0.5, 0));
		return textureLod( t1, tc, 0.0);
	}
	else if (a3)
	{
		float2 tc = 2.0 * (uv + float2(0, -0.5));
		return textureLod( t2, tc, 0.0);
	}

	float2 tc = 2.0 * (uv + float2(-0.5, -0.5));
	return textureLod( t3, tc, 0.0);
}

in float2 texcoord;
out float4 out_color { color(0) };
void main()
{
	float3 albedo = texture(gbuffer_color_texture, texcoord).xyz;
	float alpha = texture(gbuffer_color_texture, texcoord).w;

	float3 specular_color = texture(gbuffer_specular_texture, texcoord).xyz;
	float3 emissive_color = texture(gbuffer_emissive_texture, texcoord).xyz;

	float3 tex_normal = texture(gbuffer_normal_texture, texcoord).xyz;
	float glossness = texture(gbuffer_normal_texture, texcoord).w;

	float2 velocity = texture(gbuffer_velocity_texture, texcoord).xy;

	float depth = texture(gbuffer_depth_texture, texcoord).x;

	/*
	float world_shadow = sample_world_shadow_map(texcoord);
	float tanya_shadow = sample_shadow_map(shadow_map1, texcoord);
	float golem_shadow = sample_shadow_map(shadow_map2, texcoord);
	float indirect_shadow = sample_shadow_map(shadow_map_indirect_debug, texcoord);
	*/

	float4 depth_downsample = textureLod(depth_downsample_texture, texcoord, 0.0) / depth_parameters.w;
	float4 ssao = texture(ssao_texture, texcoord);
	float4 bright = sample_bloom_layers(bright_texture0, bright_texture1, bright_texture2, bright_texture3, texcoord);
	float4 bloom = sample_bloom_layers(bloom_texture0, bloom_texture1, bloom_texture2, bloom_texture3, texcoord);
	float4 neighbor_max = texture(neighbor_max_texture, texcoord);
	float4 motion_blur = texture(motion_blur_texture, texcoord);
	float4 dof_input = texture(dof_input_texture, texcoord);
	float4 dof = texture(dof_blur_texture, texcoord);
	float4 half_res_transparent = texture(half_res_transparent_texture, texcoord);

	albedo = srgb_to_linear(albedo);
	specular_color = srgb_to_linear(specular_color);

	float roughness = 1.0 - glossness;
	float3 normal = decode_normal(tex_normal);

	float3 debug_color = float3(0.0);

	float linear_depth = get_linear_depth(depth, depth_parameters);

	bool do_gamma_correction = false;
	bool black_on_sky = false;

#if DEBUG_VIEW_ALBEDO

	debug_color = albedo;
	do_gamma_correction = true;
	black_on_sky = true;

#elif DEBUG_VIEW_ALPHA

	debug_color = float3(alpha);
	black_on_sky = true;

#elif DEBUG_VIEW_SPECULAR

	debug_color = specular_color;
	do_gamma_correction = true;
	black_on_sky = true;

#elif DEBUG_VIEW_EMISSIVE

	debug_color = emissive_color;
	do_gamma_correction = true;
	black_on_sky = true;

#elif DEBUG_VIEW_GLOSSNESS

	debug_color = float3(glossness);
	black_on_sky = true;

#elif DEBUG_VIEW_ROUGHNESS

	debug_color = float3(roughness);
	black_on_sky = true;

#elif DEBUG_VIEW_NORMALS

	debug_color = normal;
	black_on_sky = true;

#elif DEBUG_VIEW_VELOCITY

	debug_color = float3(velocity.xy, 0.0);
	black_on_sky = true;

#elif DEBUG_VIEW_DEPTH

	debug_color = float3(depth);

#elif DEBUG_VIEW_DEPTH_LINEAR

	debug_color = float3(linear_depth) / depth_parameters.w;

#elif DEBUG_VIEW_DEPTH_DOWNSAMPLE

	debug_color = float3(depth_downsample.x);

#elif DEBUG_VIEW_DIRECT_SHADOW

	//golem_shadow = 1.0;
	//world_shadow = 1.0;
	//tanya_shadow = 1.0;
	float shadow = sample_shadow_map(shadow_map, texcoord);
	debug_color = float3(shadow);

#elif DEBUG_VIEW_INDIRECT_SHADOW

	//debug_color = float3(indirect_shadow);

#elif DEBUG_VIEW_SSAO

	debug_color = ssao.xxx;
	do_gamma_correction = true;

#elif DEBUG_VIEW_BRIGHT

	debug_color = bright.xyz;

#elif DEBUG_VIEW_BLOOM

	debug_color = bloom.xyz;

#elif DEBUG_VIEW_NEIGHBOR_MAX

	debug_color = neighbor_max.xyz;

#elif DEBUG_VIEW_MOTION_BLUR

	debug_color = motion_blur.xyz;

#elif DEBUG_VIEW_DOF

	debug_color = dof.xyz;

#elif DEBUG_VIEW_DOF_INPUT

	debug_color = dof_input.xyz;

#elif DEBUG_VIEW_HALF_RES_TRANSPARENT

	debug_color = half_res_transparent.xyz;
	//debug_color = float3(half_res_transparent.w);

#elif DEBUG_VIEW_ENERGY_CONSERVATIVITY_CHECK

	debug_color = float3(0.0);
	float3 diff = albedo + specular_color - float3(1.0);

	float max_diff = max( max( diff.x, diff.y), diff.z);

	if (max_diff > 0.0)
	{
		debug_color = float3(0.5 + max_diff * 1.5, 0.0, 0.0);
	}

#endif

#ifdef GAMMA_ENABLED
	// Gamma correction
	if (do_gamma_correction)
	{
		debug_color = clamp(debug_color, float3(0.0), float3(1.0));
		debug_color = linear_to_srgb(debug_color);
	}

	out_color = float4(debug_color, 1.0);
#endif

	if (black_on_sky && depth == 1.0)
	{
		out_color = float4(0.0, 0.0, 0.0, 0.0);
	}
}
