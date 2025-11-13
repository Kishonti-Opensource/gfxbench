/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;

in float2 texcoord;
out float3 out_color { color(0) };
void main()
{
	float4 color_hdr = textureLod(color_texture, texcoord, 0.0);

#ifdef SKY_FORMAT_RGBM
	float3 color_rgb = RGBMtoRGB(color_hdr);
#elif SKY_FORMAT_RGBE
	float3 color_rgb = RGBEtoRGB(color_hdr);
#elif SKY_FORMAT_RGB
	float3 color_rgb = color_hdr.xyz;
#else
	"ERROR! Sky format not set!"
#endif

	out_color = color_rgb;
}
