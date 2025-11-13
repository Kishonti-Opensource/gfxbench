/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include"LowLevel_Common.hlsli"

cbuffer ConstantBufferUI : register(b0)
{
	float2 c_scale;
	float2 c_offset;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	output.position = float4(input.position.xy * c_scale + c_offset, input.position.z, 1.0);
	output.texCoord0 = input.texCoord0;
	output.color0 = input.color0;

	return output;
}