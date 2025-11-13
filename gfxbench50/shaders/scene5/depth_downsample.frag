/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> input_texture_lod;
//uniform float4 tap_offsets;
uniform float lod_level;
#if LINEARIZE_DEPTH
uniform float4 depth_parameters;
#endif

in float2 texcoord;
out float out_color { color(0) };
void main()
{
	// Color
	float depth00 = textureLodOffset(input_texture_lod, texcoord, lod_level, int2(0, 0)).r;
	float depth01 = textureLodOffset(input_texture_lod, texcoord, lod_level, int2(0, 1)).r;
	float depth10 = textureLodOffset(input_texture_lod, texcoord, lod_level, int2(1, 0)).r;
	float depth11 = textureLodOffset(input_texture_lod, texcoord, lod_level, int2(1, 1)).r;

	float max_depth = max(max(depth00, depth01), max(depth11, depth10));

#if LINEARIZE_DEPTH
	max_depth = depth_parameters.y / (max_depth - depth_parameters.x);
#endif

	out_color = max_depth;
}
