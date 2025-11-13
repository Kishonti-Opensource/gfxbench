/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CONSTANTBUFFER_H
#define CONSTANTBUFFER_H


#include "d3d11/DX.h"


//DX
//XMFLOAT4X4A, XMFLOAT4A val assertet dobal
__declspec(align(16)) struct ConstantBuffer
{
	DirectX::XMFLOAT4X4		mvp;
	DirectX::XMFLOAT4X4		mv;
	DirectX::XMFLOAT4X4		model;
	DirectX::XMFLOAT4X4		inv_model;
	DirectX::XMFLOAT4X4		shadow_matrix0;
	DirectX::XMFLOAT4X4		shadow_matrix1;
	DirectX::XMFLOAT4X3		bones[32];
	DirectX::XMFLOAT4		color;
	DirectX::XMFLOAT4		global_light_dir;
	DirectX::XMFLOAT4		global_light_color;
	DirectX::XMFLOAT4		view_dir;
	DirectX::XMFLOAT4		view_pos;
	DirectX::XMFLOAT4		background_color;
	DirectX::XMFLOAT4		light_pos;
	DirectX::XMFLOAT4		offset_2d;
	DirectX::XMFLOAT4		translate_uv;
	float					time;
	float					fog_density;
	float					diffuse_intensity;
	float					specular_intensity;
	float					reflect_intensity;
	float					specular_exponent;
	float					transparency;
	float					envmaps_interpolator;
	float					camera_focus;
	float					alpha_threshold;
	float					mblur_mask;

	// Stuff needed for GFX 3.0 and higher.
	float					attenuation_parameter;
	DirectX::XMFLOAT4		light_color;
	DirectX::XMFLOAT2		screen_resolution;
	DirectX::XMFLOAT2		inv_screen_resolution;
	DirectX::XMFLOAT4		depth_parameters;
	DirectX::XMFLOAT4		light_x;
	DirectX::XMFLOAT4X4		inv_mv;
	DirectX::XMFLOAT4		fresnel_params;
	DirectX::XMFLOAT2		spot_cos;
};

__declspec(align(16)) struct ConstantBufferParticle
{
    DirectX::XMFLOAT4X4	mvp;
	DirectX::XMFLOAT4X4	mv;
    DirectX::XMFLOAT4X4	model;
    DirectX::XMFLOAT4X4	world_fit;
	DirectX::XMFLOAT4   particle_data[50];
	DirectX::XMFLOAT4   particle_color[50];

	// Stuff needed for GFX 3.0 and higher.
	DirectX::XMFLOAT4	color;
};

__declspec(align(16)) struct ConstantBufferMBlur
{
	DirectX::XMFLOAT4X4	mvp;
	DirectX::XMFLOAT4X4	mvp2;
	DirectX::XMFLOAT4X3	bones[32];
	DirectX::XMFLOAT4X3	prev_bones[32];
};

__declspec(align(16)) struct ConstantBufferFilter
{
	DirectX::XMFLOAT2		screen_resolution;
	DirectX::XMFLOAT2		inv_screen_resolution;
	DirectX::XMFLOAT4		depth_parameters;

	DirectX::XMFLOAT2		offset_2d;
	float		dof_strength;
	float		camera_focus;
};

__declspec(align(16)) struct ConstantBufferEmitter
{
	DirectX::XMFLOAT4X4 worldmat;
	DirectX::XMFLOAT4X4 mv;
	DirectX::XMFLOAT4X4 mvp;

	unsigned int startBirthIndex;
	unsigned int endBirthIndex;
	float begin_size;
	float end_size;

	DirectX::XMFLOAT3 aperture;
	float focusdist;

	DirectX::XMFLOAT3 min_freq;
	float min_speed;

	DirectX::XMFLOAT3 max_freq;
	float max_speed;

	DirectX::XMFLOAT3 min_amplitude;
	float min_accel;

	DirectX::XMFLOAT3 max_amplitude;
	float max_accel;

	DirectX::XMFLOAT4 color;

	DirectX::XMFLOAT3 externalVelocity;
	float gravityFactor;

	float maxlife;
};


#endif
