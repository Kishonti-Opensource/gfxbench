/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<half> ssao_texture;

in float2 texcoord;
out half4 out_res { color(0) };
void main()
{
	half ssao = texture(ssao_texture, texcoord).x;
	out_res = half4(ssao, ssao, ssao, 1.0h);
}
