/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in float2 out_texcoord0;
out float4 out_color { color(0) };
void main()
{
	out_color.OUTPUT_SWIZZLE = blur(out_texcoord0);
}
