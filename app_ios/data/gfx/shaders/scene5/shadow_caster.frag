/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef ALPHA_TEST
uniform sampler2D<float> color_texture;
uniform float alpha_test_threshold;
in float2 texcoord0;
#endif

#if PARABOLOID
in float v_side;
#endif

void main()
{
#if PARABOLOID
	if(v_side < 0.0)
		discard;
#endif

#ifdef ALPHA_TEST
	float alpha = texture(color_texture, texcoord0).w;
	if( alpha < alpha_test_threshold)
	{
		discard;
	}
#endif
}
