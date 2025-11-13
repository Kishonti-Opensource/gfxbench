/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include"LowLevel_Common.hlsli"

cbuffer ConstantBufferCPU : register(b0)
{
	float2 c_position;
	float2 c_scale;
	float2 c_screenResolution;
	float2 c_matrixSize;
	float4 c_color0;
	float4 c_color1;
	float4 c_color2;
	float4 c_color3;
	float4 c_rotation;
}

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 color01 = lerp(c_color0, c_color1, input.texCoord0.x);
	float4 color23 = lerp(c_color2, c_color3, input.texCoord0.x);
	float2x2 rot = float2x2(c_rotation);
	float2 localCoord = mul(input.position.xy * c_scale * 10.0, rot) / c_screenResolution;
	float2 offset = 2.0 * (c_position / c_matrixSize) - float2(1.0, 1.0);
	float2 scrCoord = localCoord + offset;

	output.position = float4(scrCoord, input.position.z, 1.0);
	output.texCoord0 = input.texCoord0;
	output.color0 = lerp(color01, color23, input.texCoord0.y);

	return output;
}