/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "../include/types.hlsl"

ScreenVSOutput main(MinimalInput input)
{
    ScreenVSOutput output;
    output.position = float4(input.position.xy, 0.0f, 1.0f);
    output.texcoord0 = input.position.xy * 0.5 + 0.5;
	output.texcoord0.y = 1 - output.texcoord0.y;
    return output;
}
