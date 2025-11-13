/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"
#include "../include/cbuffer_emitter.hlsli"

Texture3D texture0 : register(t0);
SamplerState SampleType;

float4 main(ParticleVSOutput input) : SV_TARGET
{
	float texCol = texture0.Sample(SampleType, input.texcoord0).x;
	texCol *= input.visibility;
	float4 frag_color = texCol * emitter_color;
	frag_color.w *= 0.5;

	return frag_color;
}