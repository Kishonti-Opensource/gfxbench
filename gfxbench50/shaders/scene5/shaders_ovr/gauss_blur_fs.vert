/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
in float2 in_position;
in float2 in_texcoord0_;

out float2 out_texcoord0;
void main()
{
	float4 in_pos = float4( in_position, 0.0, 1.0);

	gl_Position = in_pos;
	out_texcoord0 = in_texcoord0_;
}