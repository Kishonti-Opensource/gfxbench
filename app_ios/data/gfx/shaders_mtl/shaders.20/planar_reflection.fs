/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
_float3 normalize2( _float3 v)
{
#ifdef USE_NORMALIZATION_CUBE_MAP
	_float3 r = textureCube( normalization_cubemap, v).xyz;
	r = r * _float(2.0) - _float(1.0);
	return r;
#else
	return normalize( v);
#endif
}


_float3 planar_reflection2( _float3 color, texture2d<_float> planar_reflection, sampler planar_sampler,
						 texture2d<_float> texture_unit3, sampler sampler3,
						 _float2 out_texcoord0, _float2 screen_position)
{
	_float3 reflection_color;

	_float3 ts_normal = texture_unit3.sample(sampler3, hfloat2(out_texcoord0) ).xyz * _float(0.04) - _float(0.02);
	
	_float2 tc = screen_position + ts_normal.xy;
	
	reflection_color = planar_reflection.sample(planar_sampler, hfloat2(tc) ).xyz;
	
	_float3 result = reflection_color;
		
	return result;
}


fragment _float4 shader_main(PlanarReflectionInOut  inFrag    [[ stage_in ]],
							constant FragmentUniforms *fu [[ buffer(FRAGMENT_UNIFORM_DATA_INDEX) ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							texture2d<_float> texture_unit2 [[texture(2)]],
							texture2d<_float> texture_unit3 [[texture(3)]],
							texture2d<_float> planar_reflection [[texture(8)]],
							sampler sampler0 [[sampler(0)]],
							sampler sampler2 [[sampler(2)]],
							sampler sampler3 [[sampler(3)]],
							sampler planar_sampler [[sampler(8)]] )
{
	_float4 result ;	

#if defined MASK
	_float4 mask = texture_unit2.sample(sampler2, hfloat2(inFrag.out_texcoord0) );
#else
	const _float4 mask = vec4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < _float(0.6) )
	{
		discard_fragment();
	}
#endif

	_float3 color = texture_unit0.sample(sampler0, hfloat2(inFrag.out_texcoord0) ).xyz;
	
	_float2 screen_pos = _float2(inFrag.out_position.xy) * _float2(fu->inv_resolution.xy) ;

	color = planar_reflection2( color, planar_reflection, planar_sampler,
								texture_unit3, sampler3,
								inFrag.out_texcoord0, screen_pos);

    
#if defined FOG
	color = mix( color, fu->background_color, inFrag.fog_distance * inFrag.fog_distance);
#endif	

    
#if defined TRANSPARENCY
	_float trp = mask.z * _float(fu->transparency);
	result = _float4( color, trp);
#else
	result = _float4( color, 1.0);
#endif


	return result ;
}

