/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct DebugVertexOutput
{
    vec<float,4> position [[position]];
    
};


vertex VertexOutput shader_main(DebugVertexInput* input [[buffer(0)]],
                                constant MeshConsts *meshConsts [[ buffer(12) ]])
{
    DebugVertexOutput out;
    uint vid = get_vertex_id();
    vec<float,4> p_d = vec<float,4>(input[vid].in_position);
    out.position = p_d.xxxx * meshConsts->mvp[0];
    out.position += p_d.yyyy * meshConsts->mvp[1];
    out.position += p_d.zzzz * meshConsts->mvp[2];
    out.position += meshConsts->mvp[3];

    return out;
}
