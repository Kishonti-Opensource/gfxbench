/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(PPVertexOutput input [[stage_in]],
							texture2d<_float> unit0 [[texture(0)]],
							//texture2d<float> unit1 [[texture(1)]],
							sampler sam0 [[sampler(0)]])
							//sampler sam1 [[sampler(1)]])
{
	_float4 frag_color;
	
	//float3 c2 = unit1.sample(sam1, input.texcoord0, bias(15.0)).xyz;
	_float l2 = 0.0;//dot( c2, vec3( 0.2126 , 0.7152 , 0.0722 ));

	_float4 cc = unit0.sample(sam0, hfloat2(input.texcoord0) );
	_float3 c = cc.xyz;
	_float l = dot( c, _float3( 0.2126 , 0.7152 , 0.0722 )) - l2;
	
	l = clamp( cc.w + powr(l, _float(4.0) )* _float(128.0), _float(0.0), _float(1.0) );
	
	c = mix( _float3( 0.0, 0.0, 0.0), c, l);

	frag_color = _float4( c, 1.0);
	
	return frag_color;
}
