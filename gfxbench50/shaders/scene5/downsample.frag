/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> input_texture;

#if LOD_MODE
uniform float lod_level;
#else
#define lod_level 0.0
#endif

in float2 texcoord;
out float4 out_color { color(0) };
void main()
{
	out_color = textureLod( input_texture, texcoord, lod_level);
}
