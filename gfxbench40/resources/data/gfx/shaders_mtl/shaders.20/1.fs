/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#if defined RELIEF

void ray_intersect_rm_linear(texture2d<_float> texture_unit2, sampler sampler2, thread _float3 &p, thread _float3 &v)
{
	const int linear_search_steps =	30;

	v /= _float(linear_search_steps);

	for( int i=0; i<linear_search_steps-1; i++)
	{
		_float4 t = texture_unit2.sample(sampler2, hfloat2(p.xy) ) ;
		
		if (p.z < t.x )
		{
			p += v;
		}
	}
}

void ray_intersect_rm_binary(texture2d<_float> texture_unit2, sampler sampler2, thread _float3 &p, thread _float3 &v)
{
	const int binary_search_steps=4;
   
	for( int i=0; i<binary_search_steps; i++)
	{
		v *= _float(0.5);
		_float4 t = texture_unit2.sample(sampler2, hfloat2(p.xy) ) ;

		if (p.z < t.x )
		{
			p += _float(2.0) * v;
		}
		p -= v;
	}
}


_float2 relief(texture2d<_float> texture_unit2, sampler sampler2, _float2 uv, _float3 vd, _float3 out_tangent, _float3 out_normal)
{
	_float3 v;
	_float3 t = normalize( out_tangent);
	_float3 n = normalize( out_normal);
	_float3 b = cross( n, t);

	t = -t;

	v.x = dot( vd, _float3( t.x, t.y, t.z));
	v.y = dot( vd, _float3( b.x, b.y, b.z));
	v.z = dot( vd, _float3( n.x, n.y, n.z));
	
	_float3 scale = _float3( 1.0, 1.0, 0.025);
	v *= scale.z / ( scale * v.z);
	
	_float3 p = _float3( uv, 0.0);
	
	ray_intersect_rm_linear(texture_unit2, sampler2, p, v);
	ray_intersect_rm_binary(texture_unit2, sampler2, p, v);
		
	return p.xy;
}

_float2 parallax(texture2d<_float> texture_unit2, sampler sampler2, _float2 uv, _float3 vd, _float3 out_tangent, _float3 out_normal)
{
	_float3 v;
	_float3 t = normalize( out_tangent);
	_float3 n = normalize( out_normal);
	_float3 b = cross( n, t);

	t = -t;

	v.x = dot( vd, _float3( t.x, t.y, t.z));
	v.y = dot( vd, _float3( b.x, b.y, b.z));
	v.z = dot( vd, _float3( n.x, n.y, n.z));
	
	_float h = texture_unit2.sample(sampler2, hfloat2(uv) ).x ;
	
	//h -= 0.5;
	h *= _float(0.05);
	
	return uv + v.xy * h;
}


#endif


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


#ifndef ZPREPASS

#ifdef LIGHTMAP
_float3 lightmapping(texture2d<_float> texture_unit1, sampler sampler1, _float2 out_texcoord1)
{
	return texture_unit1.sample(sampler1, hfloat2(out_texcoord1) ).xyz * _float(1.5);
}
#endif




#if defined LIGHTING || defined REFLECTION || defined RELIEF
_float3 calculate_normal(texture2d<_float> texture_unit3, sampler sampler3, _float2 tc, _float3 out_tangent, _float3 out_normal)
{
	_float4 ts_normal;

#ifdef ANIMATE_NORMAL
	_float4 ts_normal0 = texture_unit3.sample(sampler3, hfloat2(out_texcoord01) );
	_float4 ts_normal1 = texture_unit3.sample(sampler3, hfloat2(out_texcoord02) );
	_float4 ts_normal2 = texture_unit3.sample(sampler3, hfloat2(out_texcoord03) );
	_float4 ts_normal3 = texture_unit3.sample(sampler3, hfloat2(out_texcoord04) );

	ts_normal = (ts_normal0 + ts_normal1 + ts_normal2 + ts_normal3) / _float(4.0);
#else
	ts_normal = texture_unit3.sample(sampler3, hfloat2(tc) );
#endif

	ts_normal.xyz = (ts_normal.xyz * _float(2.0)) - _float(1.0);

	ts_normal.xyz = normalize2( ts_normal.xyz);

	_float3 t = normalize2( out_tangent);
	_float3 n = normalize2( out_normal);
	_float3 b = cross( n, t);
	_float3x3 m = _float3x3( t, b, n);

	_float3 result = m * ts_normal.xyz;

	return result;
}
#endif


#if defined SHADOW_MAP && defined LIGHTING

_float DecodeFloatRGB( _float3 rgba ) 
{
	return dot( rgba, _float3(1.0, _float(1.0)/_float(255.0), _float(1.0)/_float(65025.0) ) );
}

_float shadow(texture2d<_float> shadow_unit0, sampler shadow_sampler0,
			 texture2d<_float> shadow_unit1, sampler shadow_sampler1,
			 _float2 out_texcoord4, _float2 out_texcoord5,
			 _float out_texcoord4z, _float out_texcoord5z)
{
#if defined RGB_ENCODED
	_float color0 = DecodeFloatRGB( shadow_unit0.sample(shadow_sampler0, hfloat2(out_texcoord4.xy) ).xyz);
	_float color1 = DecodeFloatRGB( shadow_unit1.sample(shadow_sampler1, hfloat2(out_texcoord5.xy) ).xyz);
#else
	_float color0 = shadow_unit0.sample(shadow_sampler0, hfloat2(out_texcoord4.xy) ).x;
	_float color1 = shadow_unit1.sample(shadow_sampler1, hfloat2(out_texcoord5.xy) ).x;
#endif

    if( out_texcoord4z < _float(1.0) )
        if( out_texcoord4z > _float(0.0) )
            if( color0 < out_texcoord4z )
				return _float(0.33);
    
    if( out_texcoord5z < _float(1.0) )
        if( out_texcoord5z > _float(0.0) )
            if( color1 < out_texcoord5z )
				return _float(0.33);				
    
    return _float(1.0);
    
}
#endif


#ifdef LIGHTING
_float3 lighting( _float3 color, _float3 normal, _float3 view_vector, _float3 light_dir, _float3 light_color, _float specular_mask,
			   _float diffuse_intensity, _float specular_intensity, _float specular_exponent,
			   
			   texture2d<_float> shadow_unit0, sampler shadow_sampler0,
			   texture2d<_float> shadow_unit1, sampler shadow_sampler1,
			   _float2 texcoord4, _float2 texcoord5, _float texcoord4z, _float texcoord5z)
{
	_float3 half_vector = normalize2( view_vector + light_dir);

	_float ambient = 0.1;
#if defined WIDE_DIFFUSE_CLAMP
	_float diffuse = (dot( normal, light_dir) * _float(0.5) + _float(0.5)) * diffuse_intensity;
#else
	_float diffuse = clamp( dot( normal, light_dir), _float(0.0), _float(1.0) ) * diffuse_intensity;
#endif
	_float specular = clamp( dot( normal, half_vector), _float(0.0), _float(1.0) );
	specular = specular_intensity * powr( specular, specular_exponent) * specular_mask;

#if defined SHADOW_MAP
	_float s = shadow(shadow_unit0,shadow_sampler0,shadow_unit1,shadow_sampler1,
					 texcoord4, texcoord5, texcoord4z, texcoord5z);
	
	diffuse = diffuse * s;

	specular = specular * s;
#endif

	_float3 result = (color * (ambient + diffuse) + specular) * light_color;
	
	return result;
}

#endif



#if defined REFLECTION
_float3 reflection( _float3 color, _float3 normal, _float3 view_vector, _float reflection_mask, _float envmaps_interpolator,
			_float reflect_intensity, 
			texturecube<_float> envmap0, sampler env_sampler0,
			texturecube<_float> envmap1, sampler env_sampler1)
{
	_float3 reflect_vector = reflect( -view_vector, normal);
	
	_float4 reflection0 = envmap0.sample(env_sampler0, hfloat3(reflect_vector) );
	_float4 reflection1 = envmap1.sample(env_sampler1, hfloat3(reflect_vector) );
	
	_float3 reflection_color = mix( reflection1.xyz, reflection0.xyz, envmaps_interpolator);
	
	_float3 result = mix( color, reflection_color, reflect_intensity * reflection_mask);
	
	return result;
}
#endif





fragment _float4 shader_main(_1_InOut  inFrag    [[ stage_in ]],
							constant FragmentUniforms *fu [[ buffer(FRAGMENT_UNIFORM_DATA_INDEX) ]],
							texture2d<_float> texture_unit0 [[texture(0)]],
							texture2d<_float> texture_unit1 [[texture(1)]],
							texture2d<_float> texture_unit2 [[texture(2)]],
							texture2d<_float> texture_unit3 [[texture(3)]],
							texture2d<_float> shadow_unit0 [[texture(4)]],
							texture2d<_float> shadow_unit1 [[texture(5)]],
							texturecube<_float> envmap0 [[texture(6)]],
                            texturecube<_float> envmap1 [[texture(7)]],
                            sampler sampler0 [[sampler(0)]],
                            sampler sampler1 [[sampler(1)]],
                            sampler sampler2 [[sampler(2)]],
                            sampler sampler3 [[sampler(3)]],
                            sampler shadow_sampler0 [[sampler(4)]],
                            sampler shadow_sampler1 [[sampler(5)]],
                            sampler env_sampler0 [[sampler(6)]],
                            sampler env_sampler1 [[sampler(7)]] )
{
	_float4 r ;
	  
#if defined RELIEF
	_float3 view_dir2 = normalize2( inFrag.out_view_dir );
	_float2 tc = parallax(texture_unit2, sampler2, inFrag.out_texcoord0, view_dir2, inFrag.out_tangent, inFrag.out_normal );
#else
	_float2 tc = inFrag.out_texcoord0;
#endif


#if defined MASK

#ifdef ANIMATE_NORMAL
	_float4 mask0 = texture_unit2.sample(sampler2, out_texcoord01);
	_float4 mask1 = texture_unit2.sample(sampler2, out_texcoord02);
	_float4 mask2 = texture_unit2.sample(sampler2, out_texcoord03);
	_float4 mask3 = texture_unit2.sample(sampler2, out_texcoord04);

	_float4 mask = (mask0 + mask1 + mask2 + mask3) / _float(4.0);
#else
	_float4 mask = texture_unit2.sample(sampler2, hfloat2(tc) );
#endif

#else
	const _float4 mask = _float4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < _float(0.5))
	{
		discard_fragment();
	}
#endif

	_float3 color = texture_unit0.sample(sampler0, hfloat2(tc) ).xyz;

	
#if defined LIGHTING || defined REFLECTION || defined RELIEF
	_float3 view_dir = normalize2( inFrag.out_view_dir);
	_float3 normal = calculate_normal(texture_unit3, sampler3, tc, inFrag.out_tangent, inFrag.out_normal);
#endif


#ifdef LIGHTING
	color = lighting( color, normal, view_dir, _float3(fu->global_light_dir.xyz), _float3(fu->global_light_color.xyz), mask.y,
					  fu->diffuse_intensity, fu->specular_intensity,fu->specular_exponent,
					  
					  shadow_unit0, shadow_sampler0, shadow_unit1, shadow_sampler1,
					  inFrag.out_texcoord4, inFrag.out_texcoord5, inFrag.out_texcoord4z, inFrag.out_texcoord5z);	
#endif //LIGHTING


#ifdef LIGHTMAP
	color *= lightmapping(texture_unit1, sampler1, inFrag.out_texcoord1);
#endif


#if defined REFLECTION
	color = reflection( color, normal, view_dir, mask.z,fu->envmaps_interpolator,fu->reflect_intensity,
					    envmap0, env_sampler0, envmap1, env_sampler1);
#endif //REFLECTION

	
#if defined FOG
	color = mix( color, _float3(fu->background_color.xyz), inFrag.fog_distance * inFrag.fog_distance);
#endif	

	
#if defined TRANSPARENCY
	_float trp = mask.x * fu->transparency;
	r = _float4( color, trp);
#else
	r = _float4( color, fu->mblur_mask);
#endif


#if defined DOF
	r.w = dof;
#endif

	return r ;

}

#else

fragment _float4 shader_main(_1_InOut  inFrag    [[ stage_in ]],
							constant FragmentUniforms *fu [[ buffer(FRAGMENT_UNIFORM_DATA_INDEX) ]],
							texture2d<_float> texture_unit2 [[texture(2)]],
							sampler sampler2 [[sampler(2)]])
{	
#if defined RELIEF
	_float3 view_dir2 = normalize2( inFrag.out_view_dir );
	_float2 tc = parallax(texture_unit2, sampler2, inFrag.out_texcoord0, view_dir2, inFrag.out_tangent, inFrag.out_normal );
#else
	_float2 tc = inFrag.out_texcoord0;
#endif


#if defined MASK

#ifdef ANIMATE_NORMAL
	_float4 mask0 = texture_unit2.sample(sampler2, out_texcoord01);
	_float4 mask1 = texture_unit2.sample(sampler2, out_texcoord02);
	_float4 mask2 = texture_unit2.sample(sampler2, out_texcoord03);
	_float4 mask3 = texture_unit2.sample(sampler2, out_texcoord04);

	_float4 mask = (mask0 + mask1 + mask2 + mask3) / _float(4.0);
#else
	_float4 mask = texture_unit2.sample(sampler2, hfloat2(tc) );
#endif

#else
	const _float4 mask = _float4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < _float(0.5) )
	{
		discard_fragment();
	}
#endif	
	
	return _float4(1.0);
}

#endif

