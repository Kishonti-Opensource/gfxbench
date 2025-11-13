/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex OCVertexOutput shader_main(constant OCVertexInput* input [[buffer(0)]],
								constant MeshConsts* meshConst [[buffer(12)]],
								uint vid [[vertex_id]])
{
	OCVertexOutput out;
	_float4 p = _float4(input[vid].in_position.xyz, 1.0);
	out.position = meshConst[0].mvp * p;
	out.point_size = 6.0;
	
	return out;
}

