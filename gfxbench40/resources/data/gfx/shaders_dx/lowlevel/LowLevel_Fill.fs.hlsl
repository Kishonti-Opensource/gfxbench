/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include"LowLevel_Common.hlsli"

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
SamplerState SampleType;

float4 main(FillPixelShaderInput input) : SV_TARGET
{
	float4 color = texture0.Sample(SampleType, input.texCoord0);
	float4 light1 = texture1.Sample(SampleType, input.texCoord1);
	float4 light2 = texture1.Sample(SampleType, input.texCoord2);
	float4 light3 = texture1.Sample(SampleType, input.texCoord3);

	float4 light = light1 * 0.5 + light2 * 0.3 + light3 * 0.2;
	return color * light;
}