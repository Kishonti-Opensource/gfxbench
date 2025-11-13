/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;
uniform float alpha_test_threshold;

in float2 texcoord0;
out float4 out_color { color(0) };
void main()
{
#ifdef ALPHA_TEST
	float alpha = texture(color_texture, texcoord0).w;
	if( alpha < alpha_test_threshold)
	{
		discard;
	}
#endif

	out_color = float4(1.0, 1.0, 1.0, 1.0);
}
