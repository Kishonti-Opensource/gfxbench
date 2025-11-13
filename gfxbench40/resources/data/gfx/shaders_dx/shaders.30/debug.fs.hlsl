/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
SamplerState SampleType;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

float4 main(VertexShaderOutput input) : SV_TARGET
{
	if (view_pos.w>3.0f)
	{
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (view_pos.w>2.0f)
	{
		return float4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else if (view_pos.w>1.0f)
	{
		return float4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (view_pos.w>0.0f)
	{
		return float4(1.0f, 0.0f, 0.0f, 1.0f);
	}

    return float4(input.normal, 1.0f);
}
