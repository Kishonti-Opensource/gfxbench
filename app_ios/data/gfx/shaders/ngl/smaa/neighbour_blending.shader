/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef SMAA_ONLY_COMPILE_VS
in vec3 in_position;
in vec2 in_texcoord;
out vec2 texcoord;
out vec4 offset;
void main()
{
	gl_Position = vec4(in_position, 1.0);
	texcoord = in_texcoord;
	offset = mad(SMAA_RT_METRICS.xyxy, float4( 1.0, 0.0, 0.0,  1.0), texcoord.xyxy);
}
#endif


#ifdef SMAA_ONLY_COMPILE_PS
uniform sampler2D albedo_tex;
uniform sampler2D blend_tex;
in vec2 texcoord;
in vec4 offset;
void main()
{
	gl_FragColor = SMAANeighborhoodBlendingPS(texcoord, offset, albedo_tex, blend_tex);
	//gl_FragColor = texture(albedo_tex, texcoord);
}
#endif
