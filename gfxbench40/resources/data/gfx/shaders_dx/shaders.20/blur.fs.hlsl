/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);

SamplerState SampleType;

#include "../include/types.hlsl"


float4 main(VertexShaderOutput input) : SV_TARGET
{
	float c = texture0.Sample(SampleType, input.animatecoord0).x * 0.333333;
	c += texture0.Sample(SampleType, input.animatecoord1).x * 0.333333;
	c += texture0.Sample(SampleType, input.animatecoord2).x * 0.333333;

    return float4( c, c, c, 1.0f);
}
