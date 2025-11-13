/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex PostProcessInOut shader_main( PPVertexInput input [[ stage_in ]] )
{
    PostProcessInOut r ;
    
    r.out_position = _float4(input.in_pos, 0.0, 1.0);
    r.out_texcoord0 = v_float2( input.in_uv );
    
    return r;
}

