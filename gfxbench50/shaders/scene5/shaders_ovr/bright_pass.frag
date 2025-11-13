/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> input_texture;
uniform float4 bloom_parameters;

in float2 texcoord;
out float4 out_color { color(0) };
void main()
{
	float3 hdr_color = texture( input_texture, texcoord).xyz;

	// Color correction
	hdr_color = apply_color_correction(hdr_color);

	out_color = float4(tonemapper(hdr_color, exp2(hdr_exposure.x - bloom_parameters.x)), 1.0);
}
