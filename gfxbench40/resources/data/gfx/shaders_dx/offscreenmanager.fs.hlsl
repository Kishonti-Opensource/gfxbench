/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
Texture2D texture0 : register(t0);

SamplerState SampleType;

struct VertexShaderOutput
{
    float4 position  : SV_POSITION;
    float2 vTexCoord : TEX_COORD0;
};

float4 main(VertexShaderOutput input) : SV_TARGET
{
	return texture0.Sample(SampleType, input.vTexCoord);
}