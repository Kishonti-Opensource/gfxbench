/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

LightVSOutput main(MinimalInput input)
{
	LightVSOutput output;

	float3 worldPos = mul(float4(input.position, 1.0), model).xyz;

	float3 viewDir = worldPos - view_pos.xyz;
	output.view_dir.xyz = viewDir;
	output.view_dir.w = dot(view_dir.xyz, viewDir);

	output.position = mul(float4(input.position, 1.0), mvp);

	output.screen_coord = output.position;
	return output;
}