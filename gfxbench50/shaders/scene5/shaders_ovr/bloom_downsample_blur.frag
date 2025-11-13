/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<half> texture_unit0;

uniform float2 inv_resolution;
uniform int gauss_lod_level; //lod level to sample

const float2 gauss_offsets[] = { PACKED_GAUSS_OFFSETS };

in float2 out_texcoord0;

out half4 out_color { color(0) };
void main()
{
	half3 color = half3(0.0);
	for (int i = 0; i < KS; i++)
	{
		// TODO: This can be pre-multiplied
		float2 offset = gauss_offsets[i] * inv_resolution.xy;
		color += textureLod( texture_unit0, out_texcoord0 + offset, float(gauss_lod_level)).xyz;
	}
	out_color = half4(GAUSS_WEIGHT * color, 1.0h);
}