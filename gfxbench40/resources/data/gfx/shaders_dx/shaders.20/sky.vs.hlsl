/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

VertexShaderOutput main(VertexShaderInput input)
{
	float4 pos;
    VertexShaderOutput output;
	
	pos = float4(input.position, 1.0f) ;
    pos = mul(pos, mvp);
	pos.z = pos.w;

    output.position = pos;
    output.texcoord0 = input.texcoord0;

    return output;
}
