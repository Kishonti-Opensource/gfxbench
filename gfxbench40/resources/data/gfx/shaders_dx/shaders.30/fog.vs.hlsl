/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

float in_scrollspeed = 0.0;

FogStruct main(MinimalInput input)
{
	FogStruct output;

	output.position = mul(float4(input.position, 1.0), mvp);
	output.world_pos = input.position;
	output.shadow_texcoord = mul(float4(output.world_pos, 1.0), shadow_matrix0);

	output.screen_coord = output.position;
	return output;
}