/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(ColorBlurVertexOutput input [[stage_in]],
							texture2d<_float> unit0 [[texture(0)]],
							sampler sam0 [[sampler(0)]])
{
	_float4 c = unit0.sample(sam0, hfloat2(input.texcoord0) );
	c += unit0.sample(sam0, hfloat2(input.texcoord1) );
	c += unit0.sample(sam0, hfloat2(input.texcoord2) );
	c *= _float(0.3333333);
	
	return c;
}


