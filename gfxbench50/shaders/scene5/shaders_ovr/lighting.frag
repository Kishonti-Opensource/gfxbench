/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
out float4 out_color { color(0) };

uniform sampler2D<float> gbuffer_color_texture;
uniform sampler2D<float> gbuffer_normal_texture;
uniform sampler2D<float> gbuffer_specular_texture;
uniform sampler2D<float> gbuffer_depth_texture;
uniform sampler2D<float> ssao_texture;
uniform sampler2D<float> blurred_ssao_texture;
//uniform sampler2D<float> shadow_map;
uniform sampler2DShadow<float> shadow_map;
uniform sampler2D<float> environment_brdf;
uniform samplerCube<float> envmap0;

uniform float4 light_dir;
uniform float4 light_color;

uniform float4 view_pos;
uniform float4 depth_parameters;

uniform float4x4 shadow_matrix;

in float2 texcoord;
in float4 out_view_dir;

float avg( float4 x )
{
	return (x.x + x.y + x.z + x.w) * 0.25;
}

/* float manual_pcf_gather(sampler2DShadow<float> texture, float2 tex_size, float2 inv_tex_size, float3 shadow_coord)
{
	float2 f = fract(shadow_coord.xy*tex_size+0.5);
    float2 centroid_uv = floor(shadow_coord.xy*tex_size+0.5)*inv_tex_size;

	float4 samples = textureGather(texture, centroid_uv, shadow_coord.z);

	float sampleX0 = mix(samples.w, samples.z, f.x);
	float sampleX1 = mix(samples.x, samples.y, f.x);
	float sampleXY = mix(sampleX0, sampleX1, f.y);

	return sampleXY;
}*/

float get_sun_shadow(float3 world_pos, sampler2DShadow<float> sm)
{
	// Simple shadow map
	float4 shadow_coord = shadow_matrix * float4(world_pos, 1.0);
	
	shadow_coord.z -= 
#if SHADOW_MAP_SIZE == 2048
	0.001
#elif SHADOW_MAP_SIZE == 1024
	0.0075
#elif SHADOW_MAP_SIZE == 512
	0.05
#endif
	;
	
	float2 tex_size = float2(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	float2 uv = shadow_coord.xy;
	float2 inv_tex_size = 1.0 / tex_size;
	
	/**
	float accum = 0.0;
	int size = 1;
	int numsamples = (size * 2 + 1);
	for(int y = -size; y <= size; ++y)
		for(int x = -size; x <= size; ++x)
		{
			accum += manual_pcf_gather(sm, tex_size, inv_tex_size, float3(shadow_coord.xy + float2(x,y)*2.0*inv_tex_size, shadow_coord.z));
		}			
	return accum * (1.0 / float(numsamples*numsamples));
	/**/
	
	/**
	return manual_pcf_gather(sm, tex_size, inv_tex_size, shadow_coord.xyz);
	/**/
	
	/**/
	return texture(sm, shadow_coord.xyz);
	/**/
}

float3 getHemiAmbient(float3 N)
{
	float3 sky_color = float3( 110, 165, 255 ) / 255.0;
	float3 ground_color = float3( 0.25 );

	return mix(ground_color.xyz, sky_color.xyz, clamp(N.y,0.0,1.0));
}


void main()
{
	float depth = texture(gbuffer_depth_texture, texcoord).x;
	if(depth == 1.0)
	{
		discard;
	}
	
	// Color
	float4 albedo = texture(gbuffer_color_texture, texcoord);
	
	float3 view_dir = out_view_dir.xyz / out_view_dir.w;
	float3 world_pos = get_world_pos(depth, depth_parameters, view_dir, view_pos.xyz);
	view_dir = normalize(view_dir);
	
	// Specular + translucency
	float4 specular_color = texture(gbuffer_specular_texture, texcoord);
	float4 tex_norm_gloss = texture(gbuffer_normal_texture, texcoord);
	float3 normal = decode_normal(tex_norm_gloss.xyz);
	
	albedo.xyz = srgb_to_linear(albedo.xyz);
	specular_color.xyz = srgb_to_linear(specular_color.xyz);
	
	// Energy conservation
	albedo.xyz = get_energy_conservative_albedo(albedo.xyz, specular_color.xyz);
	
	float roughness = 1.0 - tex_norm_gloss.w;
	roughness = max(roughness, 0.02);
	
	float3 n = normalize(normal);
	float3 l = normalize(light_dir.xyz);

	// Diffuse
	float3 diffuse_light = get_sh_irradiance( n );
	float3 diffuse_color = diffuse_light * albedo.xyz;

	// Specular
	float NdotV = dot(n, -view_dir.xyz);
	NdotV =  clamp(NdotV, 0.0, 1.0);
	float2 fresnel_scale_bias = texture( environment_brdf, float2(roughness, NdotV)).xy;

	float mip_level = get_roughness_lod_level(roughness);
	float3 R = reflect( view_dir.xyz, n );
	float3 prefiltered_color = textureLod( envmap0, R, mip_level ).xyz;
	float3 reflected_color = prefiltered_color * (specular_color.xyz * fresnel_scale_bias.x + fresnel_scale_bias.y);

#if SSAO_ENABLED
	//float ao = texture(ssao_texture, texcoord).x;
	float ao = texture(blurred_ssao_texture, texcoord).x;
#else
	const float ao = 1.0;
#endif

	float3 ibl_color = (diffuse_color + reflected_color);
	float3 const_ambient_color = float3(0.1);
	float3 hemi_ambient_color = getHemiAmbient(n);
	float3 ambient = (0.0
#if ENABLE_IBL_AMBIENT
	+ ibl_color
#endif
#if ENABLE_CONSTANT_AMBIENT
	+ const_ambient_color * albedo.xyz
#endif
#if ENABLE_HEMISPHERIC_AMBIENT
	+ hemi_ambient_color * albedo.xyz
#endif
	) * float3(ao);
	
	// Shading
	float3 diffuse = direct_diffuse(albedo.xyz, n, l, light_color.xyz, 1.0);
	float3 specular = direct_specular(specular_color.xyz, roughness, n, -view_dir.xyz, l, light_color.xyz, 1.0);
#if ENABLE_SHADOW_MAPPING
	float shadow = get_sun_shadow(world_pos, shadow_map);
#else
	float shadow = 1.0;
#endif
	
	float3 direct_light_color = merge_direct_lighting(diffuse, specular, specular_color.xyz, shadow);

	out_color = RGBMEncode(ambient
#if ENABLE_DIRECT_LIGHTING
	+ direct_light_color
#endif
	);
}
