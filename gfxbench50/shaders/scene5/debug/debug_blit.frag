/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> texture_unit0;

in float2 texcoord;
out float4 out_res { color(0) };
void main()
{
	out_res = texture( texture_unit0, texcoord);
}

