/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

typedef vec<uint,2> ivec2;
typedef vec<uint,3> ivec3;
typedef vec<uint,4> ivec4;


fragment _float4 shader_main(CommonVertexOutput input [[stage_in]],
							 constant FrameConsts* frame [[buffer(10)]],
							 constant MaterialConsts* material [[buffer(11)]],
							 constant MeshConsts* mesh [[buffer(12)]],
							 texture2d<_float> unit0 [[texture(0)]],
							 texture2d<_float> unit1 [[texture(1)]],
							 texture2d<_float> unit2 [[texture(2)]],
							 texture2d<_float> unit3 [[texture(3)]],
							 texture2d<_float> unit4 [[texture(4)]],
							 //never used?
							 //texture2d_array<float> texture_array_unit0 [[texture(5)]],
							 texturecube<_float> envmap0 [[texture(6)]],
							 texturecube<_float> envmap1 [[texture(7)]],
							 sampler sam [[sampler(0)]],
							 sampler cube [[sampler(1)]])
{
	_float4 frag_color;
	_float3 normal = normalize( input.normal);
	_float3 tangent = normalize( input.tangent);
	_float3 bitangent = cross( tangent, normal);
	
	_float3 texel_color = unit0.sample(sam, hfloat2(input.texcoord0) ).xyz;
	_float3 ts_normal = unit3.sample(sam, hfloat2(input.texcoord0) ).xyz;
#ifdef MASK
	_float4 mask = unit2.sample(sam, hfloat2(input.texcoord0) );
#else
	_float4 mask = _float4( 1.0, 0.0, 0.0, 1.0);
#endif
	
#ifdef EMISSION
	_float4 emission = unit4.sample(sam, hfloat2(input.texcoord0) );
#endif
	
#if defined ALPHA_TEST
	if( mask.x < _float(0.25) )
	{
		discard_fragment();
	}
#endif
	
	ts_normal.xyz = ts_normal.xyz * _float(2.0) - _float(1.0);
	
	_float3x3 mat = _float3x3( tangent, bitangent, normal);
	
	normal = mat * ts_normal;
	
	_float3 reflect_vector = reflect( input.view_dir, normal);
	_float3 env_color0 = envmap0.sample(cube, hfloat3(reflect_vector) ).xyz;
	_float3 env_color1 = envmap1.sample(cube, hfloat3(reflect_vector) ).xyz;
	
	_float3 env_color = mix(env_color1.xyz, env_color0.xyz, _float(mesh[0].envmapInterpolator.x) );
	
	_float3 color = mix(texel_color, env_color, _float(material[0].matparams_disiseri.w) * mask.z);
	
#ifdef EMISSION
	//color = emission.x * _float(8.0);
#endif
	
	frag_color = _float4( color, mask.x * _float(material[0].fresnelXYZ_transp.w) );
	//frag_color = vec4( color, transparency);
	//frag_color = vec4( color, _float(0.5));
	
    
#ifdef SV_31
	RGBA10A2_ENCODE(frag_color.xyz) ;
#endif
	
	return frag_color;

}



