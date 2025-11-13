/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in float2 in_position;
in float2 in_texcoord0_;

out float2 texcoord;

void main() 
{
	gl_Position = float4( in_position, 0.0, 1.0);
	texcoord = in_texcoord0_;

#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	texcoord.y = 1.0- texcoord.y;
#endif

#ifdef NGL_ORIGIN_UPPER_LEFT
	texcoord.y = 1.0 - texcoord.y;
#endif

#if DOF_PORTRAIT_MODE
	float2 tmp = texcoord;
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	texcoord.x = 1.0-tmp.y;
#else
	texcoord.x = tmp.y;
#endif
#ifdef NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP
	texcoord.y = tmp.x;
#else
	texcoord.y = 1.0-tmp.x;
#endif
#endif
}
