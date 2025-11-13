/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> input_texture;
uniform float4 bloom_parameters;

uniform float2 inv_resolution;
uniform int gauss_lod_level;

in float2 texcoord;
out float4 out_color { color(0) };
void main()
{
	//downsample to 1/4 res in this pass, hence the sampling
	float3 color = float3(0.0);
	color += apply_color_correction(RGBMDecode(textureLod( input_texture, texcoord + float2(-1.0, -1.0)*inv_resolution.xy, float(gauss_lod_level))).xyz);
	color += apply_color_correction(RGBMDecode(textureLod( input_texture, texcoord + float2( 1.0, -1.0)*inv_resolution.xy, float(gauss_lod_level))).xyz);
	color += apply_color_correction(RGBMDecode(textureLod( input_texture, texcoord + float2(-1.0,  1.0)*inv_resolution.xy, float(gauss_lod_level))).xyz);
	color += apply_color_correction(RGBMDecode(textureLod( input_texture, texcoord + float2( 1.0,  1.0)*inv_resolution.xy, float(gauss_lod_level))).xyz);

	out_color = float4(tonemapper(color*0.25, exp2(hdr_exposure.x - bloom_parameters.x)), 1.0);
}
