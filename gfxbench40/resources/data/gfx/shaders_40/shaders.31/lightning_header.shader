/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
float pack_w(float intensity, float end_point_distance)
{
	return float(packHalf2x16(vec2(intensity, end_point_distance)));
}

vec2 unpack_w(in float w)
{
    // .x - intensity
    // .y - endpoint distance
    return unpackHalf2x16(uint(w));	
}
