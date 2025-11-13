/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
vertex SkyInOut shader_main(VertexInput    vi  [[ stage_in  ]],
						    constant VertexUniforms *vu  [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]])
{
    SkyInOut r ;
    
    r.out_position = vu->mvp * _float4( vi.in_position.xyz, 1.0);
	r.out_position.z = r.out_position.w;
    r.out_texcoord0 = v_float2(vi.in_texcoord0.xy) ;
    
    return r;
}

