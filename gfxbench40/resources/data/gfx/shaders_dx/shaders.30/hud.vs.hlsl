/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
float4 main(in float4 position : SV_POSITION,
			in float2 inTexCoord : TEXCOORD0,
			out float2 outTexCoord : TEXCOORD0) : SV_POSITION
{
	outTexCoord = inTexCoord;
	return float4(position.xy * 2 - 1, 0, 1);
}