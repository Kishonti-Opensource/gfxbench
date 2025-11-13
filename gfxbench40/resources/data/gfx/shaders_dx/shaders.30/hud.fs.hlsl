/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);
SamplerState SampleType;

float4 main(in float2 texCoord : TEXCOORD0) : SV_TARGET
{
	return float4(texture0.Sample(SampleType, texCoord).rgb, 1.0);
}