/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform sampler2D<float> texture_unit0;
uniform int gauss_lod_level;

uniform float2 inv_resolution;

#if !LOOP_UNROLL
const PREC_TYPE gauss_weights[] = { PACKED_GAUSS_WEIGHTS };
const float gauss_offsets[] = { PACKED_GAUSS_OFFSETS };
#endif

INPUT_TYPE blur(float2 texcoord)
{
	INPUT_TYPE color = INPUT_TYPE(0.0);
#if LOOP_UNROLL
	UNROLLED_BLUR
#else
	for (int i = 0; i < KS; i++)
	{
		// TODO: This can be pre-multiplied
#if defined HORIZONTAL
		float2 offset = float2(inv_resolution.x * gauss_offsets[i], 0.0);
#elif defined VERTICAL
		float2 offset = float2(0.0, inv_resolution.y * gauss_offsets[i]);
#endif
		color += gauss_weights[i] * textureLod( texture_unit0, texcoord + offset, float(gauss_lod_level)).INPUT_SWIZZLE;
	}
#endif
	return color;
}

#endif
