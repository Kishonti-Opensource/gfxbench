/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform sampler2D<PREC_TYPE> texture_unit0;

#if !LOOP_UNROLL
const PREC_TYPE gauss_weights[] = { PACKED_GAUSS_WEIGHTS };
const float gauss_offsets[] = { PACKED_GAUSS_OFFSETS };
#endif

BLUR_TYPE blur(float2 texcoord)
{
	BLUR_TYPE s = BLUR_TYPE(0.0);
#if LOOP_UNROLL
	UNROLLED_BLUR
#else
	for (int i = 0; i < KS; i++)
	{
#if defined HORIZONTAL
		float2 offset = float2(gauss_offsets[i], 0.0);
#elif defined VERTICAL
		float2 offset = float2(0.0, gauss_offsets[i]);
#endif
		s += gauss_weights[i] * INPUT_MASK(texture( texture_unit0, texcoord + offset));
	}
#endif

	return s;
}

#endif
