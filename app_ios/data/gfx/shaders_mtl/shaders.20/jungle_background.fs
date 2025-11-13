/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(JungleBackGroundInOut  inFrag    [[ stage_in ]],
						constant FragmentUniforms *fu [[ buffer(FRAGMENT_UNIFORM_DATA_INDEX) ]],
						texture2d<_float> texture_unit0 [[texture(0)]],
						texture2d<_float> texture_unit2 [[texture(2)]],
						sampler sampler0 [[sampler(0)]],
						sampler sampler2 [[sampler(2)]]
#if !defined( ZPREPASS ) && defined IOS
                        , _float4 framebuffer [[ color(0) ]]
#endif
                             )
{
	_float4 result ;

#if defined MASK
	_float4 mask = texture_unit2.sample(sampler2, hfloat2(inFrag.out_texcoord0) );
#else
	const vec4 mask = _float4( 1.0, 1.0, 1.0, 1.0);
#endif

	_float3 color = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) ).xyz;

#if defined ALPHA_TEST
	if( mask.x < _float(0.6))
	{
#if !defined( ZPREPASS ) && defined IOS
        return _float4( framebuffer.xyz, 1.0);
#else
		discard_fragment();
#endif
	}
#endif


#if defined FOG
	color = mix( color, _float3(fu->background_color.xyz), inFrag.fog_distance * inFrag.fog_distance);
#endif	

	
#if defined TRANSPARENCY
	result = _float4( color, transparency * mask.x);
#else
	result = _float4( color, 1.0);
#endif

	return result ;
}


