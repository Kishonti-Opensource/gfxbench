/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef KSL_COMPILER

#define sampler2D hsampler2D
#define vec2 float2
out float4 res { color(0) };

#endif

uniform sampler2D color_texture;

in vec2 texcoord0;
void main()
{
#ifdef ALPHA_TEST
	float alpha = texture(color_texture, texcoord0).w;
	if( alpha < 0.5)
	{
		discard;
	}
#endif

#ifdef KSL_COMPILER
	res = float4(1.0, 1.0, 1.0, 1.0);
#else
	gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
#endif
}
