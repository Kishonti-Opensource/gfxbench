/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform float4 hdr_abcd;
uniform float4 hdr_efw_tau;
uniform float hdr_tonemap_white;

//.x - exposure
//.y - exp2(exposure)
//.z - adaptive luminance
//.w - current frame average luminance
#if EXPOSURE_MANUAL
uniform float4 hdr_exposure;
#else
buffer float4 hdr_exposure;
#endif

// .x - saturation
// .y - contrast scale
// .z - contrast bias
uniform float4 color_correction;
float3 apply_color_correction(float3 color)
{
	// Saturation
	float middle_gray = dot(color, float3(0.3, 0.59, 0.11));
	color = mix(float3(middle_gray), color, color_correction.x);

	// Contrast
	color = color * color_correction.y + float3(color_correction.z);

	return color;
}

// Hejl / Burgess-Dawson: Filmic Tonemapping
// Default values are from John Hable (Uncharted 2)
float3 tonemap_filmic_func(float3 x)
{
	float A = hdr_abcd.x; // ShoulderStrength; 0.22
	float B = hdr_abcd.y; // LinearStrength; 0.3
	float C = hdr_abcd.z; // LinearAngle; 0.1
	float D = hdr_abcd.w; // ToeStrength; 0.2

	float E = hdr_efw_tau.x; // ToeNumerator; 0.01
	float F = hdr_efw_tau.y; // ToeDenominator; 0.3

	x = ((x * (A*x + C*B) + D*E) / (x * (A*x + B) + D*F)) - E/F;
	return x;
}


float3 tonemap_filmic(float3 linear_color, float linear_exposure)
{
	// Apply exposure
	float3 color = linear_exposure * linear_color;

	// Marmoset II applies this
	color = max(float3(0.0), color - float3(0.004));

	// Tonemap operator
	color = tonemap_filmic_func(color);

	// Apply white scale
	color = color * hdr_tonemap_white;

	return color;
}


float3 tonemap_reinhard(float3 linear_color, float linear_exposure)
{
	// Apply exposure
	float3 color = linear_exposure * linear_color;

	float luminance = get_luminance(linear_color);

	float tonemapped_luminance = luminance / (luminance + 1.0);
	return tonemapped_luminance * color / luminance;
}


float3 tonemap_linear(float3 linear_color, float linear_exposure)
{
	float3 color = linear_color;

	// Apply exposure
	//color = exposure * color;

	return color;
}


#if TONEMAPPER_FILMIC
	#define tonemapper tonemap_filmic
#elif TONEMAPPER_REINHARD
	#define tonemapper tonemap_reinhard
#else
	#define tonemapper tonemap_linear
#endif

#endif
