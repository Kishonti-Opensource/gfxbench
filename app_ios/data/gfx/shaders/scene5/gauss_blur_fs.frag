/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in float2 out_texcoord0;
out TEXTURE_TYPE out_color { color(0) };
void main()
{
	OUTPUT_MASK(out_color) = blur(out_texcoord0);
}
