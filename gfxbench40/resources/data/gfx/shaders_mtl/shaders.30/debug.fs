/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct DebugVertexOutput
{
	vec<float, 3> position [[position]];
};

fragment vec<float, 4> shader_main(	DebugVertexOutput vo [[stage_in] ])
{
     vec<float, 4> c;

    c.x = 1.0f;
    c.y = 0.0f;
    c.z = 0.0f;
    c.w = 1.0f;
    return c;
}
