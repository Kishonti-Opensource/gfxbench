/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_filter.hlsli"

Texture2D texture_unit0 : register(t0);
Texture2D texture_unit1 : register(t1);
Texture2D texture_unit2 : register(t2);

SamplerState SampleType;


float4 main(ScreenVSOutput input) : SV_TARGET
{
	float d = texture_unit2.Sample(SampleType, input.texcoord0).x;
	float depth = depth_parameters.y / (d - depth_parameters.x);

	float d_s = (dof_strength * 0.1);

	float dof = clamp( abs(log(depth/camera_focus)) / d_s, 0.0, 1.0);
	dof = pow ( dof, 0.6);

	float4 c1 = texture_unit1.Sample(SampleType, input.texcoord0);

	float f = dof * 3.0;

	float4 final = texture_unit0.SampleLevel(SampleType, input.texcoord0, f);
	final += c1;

	return final;
}
