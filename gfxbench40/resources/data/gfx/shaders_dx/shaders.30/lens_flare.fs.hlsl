/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/cbuffer.hlsl"
#include "../include/types.hlsl"

Texture2D texture0 : register(t0);
SamplerState SampleType;

float4 main(ScreenVSOutput In) : SV_TARGET
{
	return texture0.Sample(SampleType, In.texcoord0) * light_color * 0.1;
}