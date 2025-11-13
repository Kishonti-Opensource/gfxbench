/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef SMAA_ONLY_COMPILE_VS
in vec3 in_position;
in vec2 in_texcoord;
out vec2 texcoord;
out vec2 pixcoord;
out vec4 offset[3];
void main()
{
	gl_Position = vec4(in_position, 1.0);
	texcoord = in_texcoord;
	pixcoord = texcoord * SMAA_RT_METRICS.zw;
	// We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
	offset[0] = mad(SMAA_RT_METRICS.xyxy, float4(-0.25, -0.125, 1.25, -0.125), texcoord.xyxy);
	offset[1] = mad(SMAA_RT_METRICS.xyxy, float4(-0.125, -0.25, -0.125, 1.25), texcoord.xyxy);
	// And these for the searches, they indicate the ends of the loops:
	offset[2] = mad(SMAA_RT_METRICS.xxyy, float4(-2.0, 2.0, -2.0, 2.0) * float(SMAA_MAX_SEARCH_STEPS), float4(offset[0].xz, offset[1].yw));
}
#endif

#ifdef SMAA_ONLY_COMPILE_PS
uniform sampler2D edge_tex;
uniform sampler2D area_tex;
uniform sampler2D search_tex;
in vec2 texcoord;
in vec2 pixcoord;
in vec4 offset[3];
void main()
{
	gl_FragColor = SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edge_tex, area_tex, search_tex, ivec4(0));
}
#endif
