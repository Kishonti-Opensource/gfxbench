/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex PPVertexOutput shader_main(	PPVertexInput input [[ stage_in ]] )
{
	PPVertexOutput out;

	out.position = _float4(input.in_pos, 0.0, 1.0);
	out.texcoord0.xy = v_float2( input.in_uv );
	return out;
}
