/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined(TYPE_fragment) || defined(TYPE_compute)

#define M_PI	3.14159265358
#define M_PI_HALF 3.14159265358h

float3 get_sh_irradiance(float3 normal)
{
	float3 coeffL0;
	float3 coeffL1[3];
	float3 coeffL2[5];

	// sky cloudy free hdr
	coeffL0 = float3(0.47617,0.481866,0.502918);
	coeffL1[0] = float3(0.367666, 0.38842, 0.490323);
	coeffL1[1] = float3(0.079516, 0.0699033, 0.0649335);
	coeffL1[2] = float3(0.234263, 0.213554, 0.210651);

	coeffL2[0] = float3(0.203956,0.193104, 0.189979);
	coeffL2[1] = float3(0.0721589, 0.0646777, 0.0605624);
	coeffL2[2] = float3(-0.0235643, -0.0221851,-0.0234574);
	coeffL2[3] = float3(0.0662994,0.0593324, 0.053132);
	coeffL2[4] = float3(0.0591019, 0.0454807,0.0288728);

	//l = 0 band
	float3 light = coeffL0;

	//l = 1 band
	light += coeffL1[0].xyz * normal.y;
	light += coeffL1[1].xyz * normal.z;
	light += coeffL1[2].xyz * normal.x;

	//l = 2 band
	float3 swz = normal.yyz * normal.xzx;
	light += coeffL2[0].xyz * swz.x;
	light += coeffL2[1].xyz * swz.y;
	light += coeffL2[2].xyz * swz.z;

	float3 sqr = normal * normal;
	light += coeffL2[3].xyz * ( 3.0*sqr.z - 1.0 );
	light += coeffL2[4].xyz * ( sqr.x - sqr.y );

	return light;
}

#define REFLECTION_CAPTURE_ROUGHEST_MIP 1.0
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE 1.2

#define MAX_LOD_LEVEL 10

float get_roughness_lod_level(float roughness)
{
	float LevelFrom1x1 = REFLECTION_CAPTURE_ROUGHEST_MIP - REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE * log2(roughness + 0.0001);
	const float HardcodedNumCaptureArrayMips = float(MAX_LOD_LEVEL);
	float level = HardcodedNumCaptureArrayMips - 1.0 - LevelFrom1x1;
	return level;
}

float4 RGBMEncode( float3 color )
{
  float4 rgbm;
  color *= 1.0 / 6.0;
  rgbm.a = clamp( max( max( color.r, color.g ), max( color.b, 0.000001 ) ), 0.0, 1.0 );
  rgbm.a = ceil( rgbm.a * 255.0 ) / 255.0;
  rgbm.rgb = color / rgbm.a;
  return rgbm;
}

float3 RGBMDecode( float4 rgbm )
{
  return 6.0 * rgbm.rgb * rgbm.a;
}

float linear_to_srgb_correct(float x)
{
	if (x < 0.0031308)
	{
		return 12.92 * x;
	}
	else
	{
		return 1.055 * pow(x, 0.41666) - 0.055;
	}
}


float srgb_to_linear_correct(float x)
{
	if (x <= 0.04045)
	{
		return x / 12.92;
	}
	else
	{
		return pow((x + 0.055) / 1.055, 2.4);
	}
}


float3 linear_to_srgb(float3 color)
{
#if GAMMA_CORRECTION_FAST
	return pow(color, float3(0.45));
#else
	return float3(
		linear_to_srgb_correct(color.x),
		linear_to_srgb_correct(color.y),
		linear_to_srgb_correct(color.z));
#endif
}


float3 srgb_to_linear(float3 color)
{
#if GAMMA_CORRECTION_FAST
	return pow(color, float3(2.2));
#else
	return float3(
		srgb_to_linear_correct(color.x),
		srgb_to_linear_correct(color.y),
		srgb_to_linear_correct(color.z));
#endif
}


half3 srgb_to_linear_half(half3 color)
{
	return pow(color, half3(2.2));
}


float3x3 calc_tbn(float3 normal, float3 tangent)
{
	float3 n = normalize( normal);
	float3 t = normalize( tangent);
	float3 b = cross( n, t);
	b = normalize( b);

	return float3x3( t, b, n);

	// ALPHA 1 Content used like this:
	// b = cross( t, n);
	// return float3x3( -t, -b, n);
}


half3 encode_normal(half3 normal)
{
	return 0.5h * normal + half3(0.5h);
}


half3 decode_normal(half3 normal)
{
	return normalize(2.0h * normal - half3(1.0h));
}

float3 decode_normal_highp(float3 normal)
{
	return normalize(2.0 * normal - float3(1.0));
}


half3 calc_world_normal(half3 tex_norm, half3 normal, half3 tangent)
{
#if NORMAL_MAPPING_ENABLED
	half3 ts_normal = decode_normal(tex_norm);

	// PREC_TODO
	// half3x3 tbn = calc_tbn( normal, tangent);
	half3x3 tbn = half3x3(calc_tbn( float3(normal), float3(tangent)));

	half3 calc_normal = tbn * ts_normal;

	return half3(normalize( calc_normal));
#else
	return half3(normal);
#endif
}


float get_luminance(float3 color)
{
	return dot(color, float3(0.299, 0.587, 0.114));
}


float3 get_world_pos(float depth, float4 depth_params, float3 view_dir, float3 view_pos)
{
	float d = depth_params.y / (depth - depth_params.x);
	return d * view_dir + view_pos;
}


float get_linear_depth(float depth, float4 depth_params)
{
	return depth_params.y / (depth - depth_params.x);
}


float3 RGBMtoRGB(float4 rgbm )
{
	float3 dec = 6.0 * rgbm.rgb * rgbm.a;
	return dec*dec; //from "gamma" to linear
}


float3 RGBEtoRGB(float4 rgbe )
{
	float exponent = rgbe.a * 255.0 - 128.0;
	return rgbe.rgb * exp2(exponent);
}


float get_reflectivity(float3 specular_color)
{
	float specular_strength = max(specular_color.x, max(specular_color.y, specular_color.z));
	return 1.0 - specular_strength;
}


half3 get_energy_conservative_albedo(half3 albedo, half3 specular_color)
{
	return albedo * (half3(1.0h) - specular_color);
}


// Lambertian diffuse
half3 direct_diffuse(half3 albedo, half3 N, half3 L, half3 light_intensity, half attenuation)
{
	half NdotL = clamp(dot(N, L), 0.0h, 1.0h);
	return (1.0h / M_PI_HALF) * albedo * NdotL * light_intensity * attenuation;
}


// PBR specular model is based on [Real shading in Unreal Engine 4, Brian Karis]
// N, L, V should be normalized
float3 direct_specular(float3 specular_color, float roughness, float3 N, float3 V, float3 L, float3 light_intensity, float attenuation)
{
	float a = max( roughness * roughness, 0.001); // Disney
	float a2 = a * a;

	float3 H = normalize(L + V);
	float NdotL = clamp(dot(N, L),	0.0, 1.0);
	float NdotH = clamp(dot(N, H),	0.0, 1.0);
	float NdotV = clamp(dot(N, V),	0.0, 1.0);
	float VdotH = clamp(dot(V, H),	0.0, 1.0);

	// Cook-Torrance
	// f(L, V) = D * F * G / (4 * NdotL * NdotV)
	// f(L, V) = D * F * Vis

	// Visibility term
	// Vis = G_Smith / (4.0 * NdotL * NdotV)
	float k = a * 0.5;
	float G_Smith1 = NdotL * (1.0 - k) + k;
	float G_Smith2 = NdotV * (1.0 - k) + k;
	float Vis = 0.25 / (G_Smith1 * G_Smith2);

	// Distribution term
	// GGX/Trowsbridge-Reitz, with Disney's parametrization
	float denom = NdotH * NdotH * (a2 - 1.0 ) + 1.0;
	float D = a2 / (M_PI * denom * denom);

	// Fresnel term
	// float Fc = pow(1.0 - VdotH, 5.0);
	// Schlick's approximation with Spherical Gaussian approximation
	float Fc = exp2( (-5.55473 * VdotH - 6.98316) * VdotH );
	float3 F = Fc + (1.0 - Fc) * specular_color;

	// Apply BRDF
	float3 incoming_light = light_intensity * attenuation * NdotL;
	return D * F * Vis * incoming_light;
}


float3 merge_direct_lighting(float3 diffuse, float3 specular, float3 specular_color, float shadow)
{
	return (diffuse + specular) * shadow;
}


float3 get_debug_color(int index)
{
	if (index == 0)
	{
		return float3(1.0, 0.0, 0.0);
	}
	else if (index == 1)
	{
		return float3(0.0, 1.0, 0.0);
	}
	else if (index == 2)
	{
		return float3(0.0, 0.0, 1.0);
	}
	else if (index == 3)
	{
		return float3(1.0, 1.0, 0.0);
	}
	else if (index == 4)
	{
		return float3(1.0, 0.0, 1.0);
	}
	else if (index == 5)
	{
		return float3(0.0, 1.0, 1.0);
	}
	else if (index == 6)
	{
		return float3(1.0, 1.0, 1.0);
	}

	return float3(0.0, 0.0, 0.0);
}

#endif
