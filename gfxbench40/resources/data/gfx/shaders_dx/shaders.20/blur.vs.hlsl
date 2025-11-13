/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

VertexShaderOutput main(MinimalInput input)
{
    VertexShaderOutput output;
		
	output.position  = float4( input.position, 1.0f);
	output.animatecoord0 = (float2)input.position * 0.5f + 0.5f;
	output.animatecoord1 = output.animatecoord0 + (float2)offset_2d;
	output.animatecoord2 = output.animatecoord0 - (float2)offset_2d;

    return output;
}
