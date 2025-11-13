/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

#include "hdr_common.h"
#include "rgbm_helper.h"

layout(binding = 0) uniform mediump sampler2D texture_unit0;

in vec2 out_texcoord0;
out vec4 frag_color;
void main()
{
	vec3 result = RGBDtoRGB_lightcombine(texture(texture_unit0, out_texcoord0));
	result = bright_pass(result);
    frag_color = vec4(result, 1.0);
}

#endif

#ifdef TYPE_vertex
in vec3 in_position;
out vec2 out_texcoord0;
void main()
{
	gl_Position = vec4(in_position, 1.0);
	out_texcoord0 = in_position.xy * 0.5 + 0.5;
}
#endif
