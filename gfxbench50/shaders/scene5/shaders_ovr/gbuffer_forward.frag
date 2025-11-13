/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;
uniform sampler2D<float> normal_texture;
uniform sampler2D<float> specular_texture;
uniform sampler2D<float> ssao_texture;
uniform sampler2D<float> blurred_ssao_texture;
uniform float alpha_test_threshold;
uniform float4 view_pos;
uniform float4 view_dir;

uniform sampler2D<float> displacement_tex;
uniform float4 tessellation_factor; 
uniform float cam_near;
uniform float4 frustum_planes[6];
uniform float tessellation_multiplier;
uniform float4x4 mv;
uniform float4x4 model;
uniform float4x4 inv_model;
uniform float4x4 mvp;

uniform float4 light_dir;
uniform float4 light_color;
uniform sampler2D<float> environment_brdf;
uniform samplerCube<float> envmap0;
//uniform sampler2D<float> shadow_map;
uniform sampler2DShadow<float> shadow_map;
uniform float4x4 shadow_matrix;


//uniform float4 velocity_min_max_scale_factor;

#ifdef DEBUG_MIPMAP_LEVEL
uniform sampler2D<float> mip_texture;
in float2 texcoord_uv;
#endif

#ifdef TEXTURE_DENSITY
uniform float2 orig_texture_size;
uniform float4 min_ideal_max_texture_density;
#endif

in float2 texcoord;
in float3 world_position;
in float3 normal;
in float3 tangent;

half avg( half4 x )
{
	return (x.x + x.y + x.z + x.w) * 0.25h;
}

half manual_pcf_gather(sampler2DShadow<float> texture, float2 tex_size, float2 inv_tex_size, float3 shadow_coord)
{
	float2 f = fract(shadow_coord.xy*tex_size+0.5);
    float2 centroid_uv = floor(shadow_coord.xy*tex_size+0.5)*inv_tex_size;

	half4 samples = half4(textureGather(texture, centroid_uv, shadow_coord.z));

	half sampleX0 = mix(samples.w, samples.z, half(f.x));
	half sampleX1 = mix(samples.x, samples.y, half(f.x));
	half sampleXY = mix(sampleX0, sampleX1, half(f.y));

	return sampleXY;
}

half get_sun_shadow(float3 world_pos, sampler2DShadow<float> sm)
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
	return half(texture(sm, shadow_coord.xyz));
	/**/
}

half3 getHemiAmbient(half3 N)
{
	half3 sky_color = half3( 110.0h, 165.0h, 255.0h ) / 255.0h;
	half3 ground_color = half3( 0.25h );

	return mix(ground_color.xyz, sky_color.xyz, clamp(N.y,0.0h,1.0h));
}

out float4 out_color { color(0) };
//out float4 out_normal { color(1) };
//out float4 out_specular { color(2) };
//out float2 out_velocity { color(3) };
#if USE_SUBPASS && SHADER_METAL_IOS
out float out_depth { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#endif
void main()
{
#if USE_SUBPASS && SHADER_METAL_IOS
	out_depth = gl_FragCoord.z;
#endif

	// Color
	half4 color = half4(texture(color_texture, texcoord));
#ifdef ALPHA_TEST
	half alfa = color.w;
	if( alfa < half(alpha_test_threshold))
	{
		discard;
	}
#endif
	//out_color = float4(linear_to_srgb(color.xyz), color.w);

	// Normal + gloss
	float3 nn = normal;
#ifdef IS_TWO_SIDED
	if (!gl_FrontFacing)
	{
		nn *= -1.0;
	}
#endif
	half4 tex_norm_gloss = half4(texture(normal_texture, texcoord));
	//out_normal.xyz = encode_normal(calc_world_normal(tex_norm_gloss.xyz, nn, tangent));
	//out_normal.w = tex_norm_gloss.w;

	// Specular + translucency
	half4 specular_sample = half4(texture(specular_texture, texcoord));
	//out_specular = float4(linear_to_srgb(specular.xyz), specular.w);

	// Velocity
	//out_velocity = pack_velocity(velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor));
	//out_velocity = velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor);

	//////////////////////////////////////////////////////////////////////////////
	// LIGHTING CODE
	//////////////////////////////////////////////////////////////////////////////
	
	half4 albedo = color;
	half4 specular_color = specular_sample;
	float3 world_pos = world_position;
	
	// Energy conservation
	albedo.xyz = get_energy_conservative_albedo(albedo.xyz, specular_color.xyz);
	
	half roughness = 1.0h - tex_norm_gloss.w;
	roughness = max(roughness, 0.02h);
	
	float3 n = normalize(nn);
	half3 l = half3(normalize(light_dir.xyz));

	// Diffuse
	float3 diffuse_light = get_sh_irradiance( n );
	half3 diffuse_color = half3(diffuse_light) * albedo.xyz;

	// Specular
	half NdotV = half(dot(n, -view_dir.xyz));
	NdotV =  clamp(NdotV, 0.0h, 1.0h);
	half2 fresnel_scale_bias = half2(texture( environment_brdf, float2(float(roughness), float(NdotV))).xy);

	half mip_level = half(get_roughness_lod_level(float(roughness)));
	half3 R = half3(reflect( view_dir.xyz, n ));
	half3 prefiltered_color = half3(textureLod( envmap0, float3(R), float(mip_level) ).xyz);
	half3 reflected_color = prefiltered_color * (specular_color.xyz * fresnel_scale_bias.x + fresnel_scale_bias.y);

#if SSAO_ENABLED
	//float ao = texture(ssao_texture, texcoord).x;
	half ao = texture(blurred_ssao_texture, texcoord).x;
#else
	const half ao = 1.0h;
#endif

	half3 ibl_color = (diffuse_color + reflected_color);
	half3 const_ambient_color = half3(0.1h);
	half3 hemi_ambient_color = getHemiAmbient(half3(n));
	half3 ambient = (0.0h
#if ENABLE_IBL_AMBIENT
	+ ibl_color
#endif
#if ENABLE_CONSTANT_AMBIENT
	+ const_ambient_color * albedo.xyz
#endif
#if ENABLE_HEMISPHERIC_AMBIENT
	+ hemi_ambient_color * albedo.xyz
#endif
	) * half3(ao);
	
	// Shading
	half3 diffuse = half3(direct_diffuse(albedo.xyz, half3(n), l, half3(light_color.xyz), 1.0h));
	half3 specular = half3(direct_specular(float3(specular_color.xyz), float(roughness), float3(n), -view_dir.xyz, float3(l), light_color.xyz, 1.0));
#if ENABLE_SHADOW_MAPPING
	half shadow = get_sun_shadow(world_pos, shadow_map);
#else
	half shadow = 1.0h;
#endif
	
	half3 direct_light_color = half3(merge_direct_lighting(float3(diffuse), float3(specular), float3(specular_color.xyz), float(shadow)));

	out_color = float4(RGBMEncode(float3(ambient
#if ENABLE_DIRECT_LIGHTING
	+ direct_light_color
#endif
	)));
	
	//out_color = float4(direct_light_color, 1.0);
	
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	
	//---------------- DEBUG CODE -----------------

	//color = float4(1.0);
#ifdef DEBUG_MIPMAP_LEVEL
//original texture colors mean its a perfect match (1:1 texels to pixels ratio)
//more red = too much texture detail
//more blue = too little texture detail
	float4 mip = texture( mip_texture, texcoord_uv);
	float3 col = mix(linear_to_srgb(color.xyz), mip.xyz, mip.w);
	out_color.xyz = col;
	out_color.w = color.w;
#endif


#ifdef TEXTURE_DENSITY
	float2 texcoordDensity = max( abs(dFdx(texcoord)), abs(dFdy(texcoord)));
	float3 worldSpaceDensity = max( abs(dFdx(world_position)), abs(dFdy(world_position)));
	float2 texCoordDensityPerWorldUnit = texcoordDensity / worldSpaceDensity.xy;

	float WorldSpaceArea = length( cross( dFdx(world_position.xyz), dFdy(world_position.xyz) ) );
	WorldSpaceArea = max( WorldSpaceArea, 0.00000001 );
	float2 tt = texcoord * orig_texture_size;
	float2 A = dFdx(tt);
	float2 B = dFdy(tt);
	float2 C = A.xy * B.yx;
	float TexelArea = abs( C.x - C.y );
	float Density = 0.00000001;
	Density = max( Density, TexelArea / WorldSpaceArea );

	float MinDensity = min_ideal_max_texture_density.x;
	float IdealDensity = min_ideal_max_texture_density.y;
	float MaxDensity = min_ideal_max_texture_density.z;

	if ( Density > IdealDensity )
	{
		float Range = MaxDensity - IdealDensity;
		Density -= IdealDensity;
		out_color = float4( Density / Range, (Range - Density) / Range, 0.0, 1.0 );
	}
	else
	{
		float Range = IdealDensity - MinDensity;
		Density -= MinDensity;
		out_color = float4( 0.0, Density/Range, (Range-Density)/Range, 1.0 );
	}
	float rr = get_luminance(float4(linear_to_srgb(color.xyz), color.w).xyz);
	rr = 1.0 - rr * 0.5;
	out_color *= rr;
	//out_color = mix(float4(linear_to_srgb(color.xyz), color.w), out_color, out_normal.y);
#endif

	/*
#ifdef IS_TWO_SIDED
	float3 V = view_pos.xyz - world_position.xyz;
	V = normalize(V);

//	V = -view_dir.xyz;
	V = normalize(V);

	float flatness = (dot(V, normalize(nn)));

//	flatness = pow(flatness, 3.0);

//	out_color = float4(flatness);

	float alfa2 = texture(color_texture, texcoord, -2.0).w;
	if( alfa2 * flatness < 0.2)
	{
	//	out_color = float4(1.0, 0.0, 0.0, 1.0);
	//	out_color = float4(flatness, 0.0, 0.0, 1.0);
	//	discard;
	}
#endif
*/


}
