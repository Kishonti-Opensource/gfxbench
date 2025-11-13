/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

cbuffer ShaderConstantBuffer : register(b0)
{
    float4x4	mvp;
	float4x4	mv;
	float4x4	model;
	float4x4	inv_model;
	float4x4	shadow_matrix0;
	float4x4	shadow_matrix1;
	float4x3	bones[32];
	float4		color;
	float4		global_light_dir;
	float4		global_light_color;
	float4		view_dir;
	float4		view_pos;
	float4		background_color;
	float4		light_pos;
	float4		offset_2d;
	float4		translate_uv;
	float		time;					// Start of 16 bytes...
	float		fog_density;
	float		diffuse_intensity;
	float		specular_intensity;
	float		reflect_intensity;		// Start of 16 bytes...
	float		specular_exponent;
	float		transparency;
	float		envmaps_interpolator;
	float		camera_focus;			// Start of 16 bytes...
	float		alpha_threshold;
	float		mblur_mask;

	// Stuff needed for GFX 3.0 and higher.
	float		attenuation_parameter;
	float4		light_color;			// Start of 16 bytes...
	float2		screen_resolution;
	float2		inv_screen_resolution;
	float4		depth_parameters;
	float4		light_x;
	float4x4	inv_mv;
	float4		fresnel_params;
	float2		spot_cos;
};
