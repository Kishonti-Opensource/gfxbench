/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_UBOs
	#include emitterRenderConsts;
#else
	uniform vec3 color;
#endif

uniform sampler2D texture_unit0;

in vec3 out_UVxy_Visibility;
out vec4 frag_color;

void main()
{
#ifdef USE_UBOs
	frag_color = texture(texture_unit0, out_UVxy_Visibility.xy) * out_UVxy_Visibility.z * vec4( color_pad.xyz, 1.0);
#else
	frag_color = texture(texture_unit0, out_UVxy_Visibility.xy) * out_UVxy_Visibility.z * vec4( color, 1.0);
#endif

#ifdef SV_31
	frag_color.xyz *= 3.0;
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
	frag_color = max(frag_color, vec4(0.0));
#endif		
}

