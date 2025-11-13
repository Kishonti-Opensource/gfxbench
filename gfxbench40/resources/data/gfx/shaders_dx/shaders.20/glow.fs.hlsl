/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);

SamplerState SampleType;
SamplerState SampleTypeClamp;

#include "../include/cbuffer.hlsl"

#include "../include/types.hlsl"

#include "../include/functions.hlsl"


float4 main(VertexShaderOutput input) : SV_TARGET
{
	float4 texel = texture0.Sample(SampleType, input.texcoord0) * float4( color.xyz, 1.0);

	return float4(texel.xyz, 0.0);
}
