/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef SMAA_ONLY_COMPILE_VS
	in vec3 in_position;
	in vec2 in_texcoord;
	out vec2 texcoord;
	out vec4 offset[3];
	void main()
	{
		gl_Position = vec4(in_position, 1.0);
		texcoord = in_texcoord;
		offset[0] = mad(SMAA_RT_METRICS.xyxy, float4(-1.0, 0.0, 0.0, -1.0), texcoord.xyxy);
		offset[1] = mad(SMAA_RT_METRICS.xyxy, float4( 1.0, 0.0, 0.0,  1.0), texcoord.xyxy);
		offset[2] = mad(SMAA_RT_METRICS.xyxy, float4(-2.0, 0.0, 0.0, -2.0), texcoord.xyxy);
	}
#endif


#ifdef SMAA_ONLY_COMPILE_PS
uniform sampler2D albedo_tex;
in vec2 texcoord;
in vec4 offset[3];
void main()
{
#if SMAA_PREDICATION == 1
	gl_FragColor = SMAAColorEdgeDetectionPS(texcoord, offset, albedo_tex, depthTex);
#else
	gl_FragColor.xy = SMAAColorEdgeDetectionPS(texcoord, offset, albedo_tex);
#endif
	gl_FragColor.zw = vec2(0.0);
}
#endif

