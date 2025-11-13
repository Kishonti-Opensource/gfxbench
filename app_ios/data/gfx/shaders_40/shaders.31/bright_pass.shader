/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "sat_functions.h"
#include "rgbm_helper.h"

layout(binding = 0) uniform mediump sampler2D in_tex;

layout (OUT_TEX_TYPE, binding = OUT_TEX_LEVEL0_BIND) uniform writeonly mediump image2D out_level0_tex;

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = 1) in;
void main()
{
	ivec2 g_i = ivec2(gl_GlobalInvocationID.xy);	
	
    // first +0.5 pixelwise fetch
    // second +0.5 use texture sampler to calc the average
    vec2 uv = (vec2(2 * g_i) + 1.0) * STEP_UV;
#ifdef SV_40
	vec3 res = RGBDtoRGB_lightcombine ( texture( in_tex, uv) ) ;
#else // SV_31
	vec3 res = texture( in_tex, uv).xyz * 4.0; //decompress from GL_RGB10_A2
#endif
	
	res = bright_pass(res);

	imageStore(out_level0_tex, g_i, vec4(res,1.0));
}
