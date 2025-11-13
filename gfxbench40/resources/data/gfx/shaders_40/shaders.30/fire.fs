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

uniform sampler3D texture_3D_unit0;


in vec4 out_UVxyz_Visibility;

out vec4 frag_color;

void main()
{
	float texCol = texture(texture_3D_unit0, out_UVxyz_Visibility.xyz).x;
	texCol *= out_UVxyz_Visibility.w;
#ifdef USE_UBOs
	frag_color.xyz = texCol * color_pad.xyz;
#else
	frag_color.xyz = texCol * color;
#endif
	
	frag_color.w = texCol;
	
#ifdef SV_31
	frag_color.xyz *= 4.0;
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
	frag_color = max(frag_color, vec4(0.0));
#endif			
}

