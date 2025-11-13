/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */


fragment _float4 shader_main(ShadowCaster0VertexOutput input [[stage_in]])
{
    _float4 outColor;

    outColor.x = 0.3;
    outColor.y = 0.3;
    outColor.z = 0.3;
    outColor.w = 0.0;
    
    return outColor;
}



