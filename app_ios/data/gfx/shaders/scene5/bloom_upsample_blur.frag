/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<half> texture_unit0;
uniform sampler2D<half> input_texture;

uniform float4 inv_resolution;
uniform int gauss_lod_level; //lod level to sample

const float2 gauss_offsets[] = { PACKED_GAUSS_OFFSETS };

in float2 out_texcoord0;

out half4 out_color { color(0) };
void main()
{
	half3 color = half3(0.0);
	for (int i1 = 0; i1 < KS; i1++)
	{
		// TODO: This can be pre-multiplied
		float2 offset = gauss_offsets[i1] * inv_resolution.xy;
		color += textureLod( texture_unit0, out_texcoord0 + offset, float(gauss_lod_level)).xyz;
	}
	
	for (int i2 = 0; i2 < KS; i2++)
	{
		// TODO: This can be pre-multiplied
		float2 offset = gauss_offsets[KS+i2] * inv_resolution.zw;
		color += textureLod( input_texture, out_texcoord0 + offset, float(gauss_lod_level)).xyz;
	}
	out_color = half4(GAUSS_WEIGHT * color, 1.0h);
}