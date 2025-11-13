/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct VertexShaderInput
{
    float2 myVertex    : POSITION;
    float2 myTexCoord  : TEX_COORD0;
};
		
struct VertexShaderOutput
{
    float4 position  : SV_POSITION;
    float2 vTexCoord : TEX_COORD0;
};
		
VertexShaderOutput main( VertexShaderInput input )
{
    VertexShaderOutput output;
	output.position = float4(input.myVertex.x, input.myVertex.y, 0.0, 1.0);
	output.vTexCoord = float2(1.0-input.myTexCoord.y, 1.0-input.myTexCoord.x);
	return output;
}