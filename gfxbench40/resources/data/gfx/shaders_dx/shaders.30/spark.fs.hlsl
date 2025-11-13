/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_emitter.hlsli"

Texture2D texture0 : register(t0);
SamplerState SampleType;

float4 main(ParticleVSOutput input) : SV_TARGET
{
	return texture0.Sample(SampleType, input.texcoord0.xy) * input.visibility * emitter_color;
}