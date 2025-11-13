/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(BillboardVertexOutput input [[stage_in]],
							constant hfloat4* color [[buffer(3)]],
							texture2d<_float> texUnit0 [[texture(0)]],
							sampler sam [[sampler(0)]])
							
{
    _float4 color4 = _float4( _float3(color[0].xyz), 1.0);
	_float4 texel = texUnit0.sample( sam, hfloat2(input.texcoord0) ) * color4;

	return _float4(texel.xyz, 0.0);
}

