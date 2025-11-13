/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"

Texture2D texture0 : register(t0);
SamplerState SampleType;

float4 main(MultiTexCoordVertexStruct input) : SV_TARGET
{   
	float4 c0 = texture0.Sample(SampleType, input.texcoord0);
	float4 c1 = texture0.Sample(SampleType, input.texcoord1);
	float4 c2 = texture0.Sample(SampleType, input.texcoord2);

	float4 c = c0 + c1 + c2;

	return c * 0.3333333;
}