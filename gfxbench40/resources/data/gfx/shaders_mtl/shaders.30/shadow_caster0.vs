/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


vertex ShadowCaster0VertexOutput shader_main(VertexInput input [[ stage_in  ]],
                                constant MeshConsts* meshConst [[buffer(12)]])
{
	ShadowCaster0VertexOutput out;
	
    _float4 position = _float4(input.in_position.xyz, 1.0);
    out.position = meshConst[0].mvp * position;

    return out;
}

