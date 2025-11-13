/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer_filter.hlsli"
#include "../include/types.hlsl"

MultiTexCoordVertexStruct main(MinimalInput input)
{
    MultiTexCoordVertexStruct output;
    output.position = float4(input.position.xy, 0.0f, 1.0f);

    output.texcoord0 = input.position.xy * 0.5 + 0.5;
	output.texcoord0.y = 1.0 - output.texcoord0.y;

	output.texcoord1 = output.texcoord0 + offset_2d;
	output.texcoord2 = output.texcoord0 - offset_2d;

    return output;
}