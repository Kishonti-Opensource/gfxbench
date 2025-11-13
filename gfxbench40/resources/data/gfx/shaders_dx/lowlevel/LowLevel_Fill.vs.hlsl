/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include"LowLevel_Common.hlsli"

cbuffer ConstantBufferALU : register(b0)
{
	float c_aspectRatio;
	float c_rotation;
}

float2x2 rotateZ(float alpha)
{
	float ca = cos(alpha);
	float sa = sin(alpha);
	return float2x2(
		ca, -sa,
		sa, ca);
}

FillPixelShaderInput main(VertexShaderInput input)
{
	FillPixelShaderInput output;
	output.position = float4(input.position, 1.0);

	float2x2 rot0 = rotateZ(c_rotation);
	output.texCoord0 = mul(rot0, (input.texCoord0 - float2(0.5, 0.5)) * float2(c_aspectRatio * 2.0, 2.0)) + float2(0.5, 0.5);

	float2x2 rot1 = rotateZ(-c_rotation);
	output.texCoord1 = mul(input.texCoord0 * float2(c_aspectRatio, 1.0), rot1);

	float2x2 rot2 = rotateZ(-c_rotation * 0.5);
	output.texCoord2 = mul(input.texCoord0 * float2(c_aspectRatio, 1.0), rot2);

	float2x2 rot3 = rotateZ(c_rotation * 0.5);
	output.texCoord3 = mul(input.texCoord0 * float2(c_aspectRatio, 1.0), rot3);

	return output;
}