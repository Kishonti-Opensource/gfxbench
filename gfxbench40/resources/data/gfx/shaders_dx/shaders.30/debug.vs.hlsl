/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

VertexShaderOutput main(MinimalInput input)
{
	float4 pos;

    VertexShaderOutput output;

	
	pos = float4(input.position+(float3)view_pos, 1.0f);
    pos = mul(pos, mvp);

    output.position = pos;
    output.normal = float3(0.0,1.0,0.0);

    return output;
}
