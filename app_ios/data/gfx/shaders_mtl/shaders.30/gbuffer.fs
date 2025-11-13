/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct FragOut
{
	_float4 color0 [[color(0)]];
	_float4 color1 [[color(1)]];
	_float4 color2 [[color(2)]];
	_float4 color3 [[color(3)]];
};

_float2 EncodeFloatRG( hfloat value, _float max_value)
{
	const hfloat max16int = 256.0 * 256.0 - 1.0;

	value = clamp( value / max_value, hfloat(0.0), hfloat(1.0) );

	value *= max16int;

	hfloat2 value2 = hfloat2(value, value);
	
	hfloat2 result = floor(value2 / hfloat2(256, 1.0));
	result.g -= result.r * 256.0;
	result /= 255.0;

	return _float2(result);
}

fragment FragOut shader_main(CommonVertexOutput input [[stage_in]],
                             constant FrameConsts* frameConst [[buffer(10)]],
                             constant MaterialConsts* matConst [[buffer(11)]],
                             constant MeshConsts* meshConst [[buffer(12)]],
                             texture2d<_float> texture_unit0 [[texture(0)]],
                             texture2d<_float> texture_unit2 [[texture(2)]],
                             texture2d<_float> texture_unit3 [[texture(3)]],
							 #ifdef EMISSION
                             texture2d<_float> texture_unit4 [[texture(4)]],
							 #endif
#ifdef TRANSITION_EFFECT
                             texture2d_array<_float> texture_array_unit0 [[texture(5)]],
#endif // TRANSITION_EFFECT
                             texturecube<_float> envmap0 [[texture(6)]],
                             texturecube<_float> envmap1 [[texture(7)]],
                             sampler sam0 [[sampler(0)]],
                             sampler sam2 [[sampler(2)]],
                             sampler sam3 [[sampler(3)]],
							 #ifdef EMISSION
                             sampler sam4 [[sampler(4)]],
							 #endif
#ifdef TRANSITION_EFFECT
                             sampler arrsam0 [[sampler(5)]],
#endif // TRANSITION_EFFECT
                             sampler envsam0 [[sampler(6)]],
                             sampler envsam1 [[sampler(7)]])
{
	FragOut out;
#ifdef MASK
	//vec4 mask = texture( texture_unit2, out_texcoord0);
	_float4 mask = texture_unit2.sample(sam2, hfloat2(input.texcoord0) );
#if defined ALPHA_TEST
	if( mask.x < _float(0.25))
	{
		discard_fragment();
	}
#endif
#elif defined ALPHA_TEST
	// The mask is always (0, 0, 0, 1) if no mask is defined, so (mask.x < 0.25) would always be hit.
	discard_fragment();
#else
	_float4 mask = _float4(0, 0, 0, 1);
#endif

#ifdef EMISSION
	//vec4 emission = texture( texture_unit4, out_texcoord0);
	_float4 emission = texture_unit4.sample(sam4, hfloat2(input.texcoord0) );
#endif
    //	vec3 normal = normalize(out_normal);
    //	vec3 vtx_normal = normal;
    //	vec3 tangent = normalize( out_tangent);
    //	vec3 bitangent = cross( tangent, normal);

	_float3 normal = normalize(input.normal);
	_float3 vtx_normal = normal;
	_float3 tangent = normalize(input.tangent);
	_float3 bitangent = cross( tangent, normal);

    //	vec3 texel_color = texture( texture_unit0, out_texcoord0).xyz;
    //	vec3 ts_normal = texture( texture_unit3, out_texcoord0).xyz;
    //

	_float3 texel_color = texture_unit0.sample(sam0, hfloat2(input.texcoord0) ).xyz;
	_float3 ts_normal = texture_unit3.sample(sam3, hfloat2(input.texcoord0) ).xyz;

	ts_normal.xyz = ts_normal.xyz * _float(2.0) - _float(1.0);

	_float3x3 mat = _float3x3( tangent, bitangent, normal);
	
//#if 0
    normal = mat * ts_normal;
//#else
//    float3 temp = normal;
//    temp = tangent  * ts_normal.xxx;
//    temp = bitangent * ts_normal.yyy + temp;
//    temp = normal * ts_normal.zzz + temp;
//    normal = temp;
//#endif
	
	_float3 reflect_vector = reflect( input.view_dir, normal);
    //	vec3 env_color0 = texture( envmap0, reflect_vector).xyz;
    //	vec3 env_color1 = texture( envmap1, reflect_vector).xyz;


	_float3 env_color0 = envmap0.sample(envsam0, hfloat3(reflect_vector) ).xyz;
	_float3 env_color1 = envmap1.sample(envsam1, hfloat3(reflect_vector) ).xyz;


	_float3 env_color = mix( env_color1.xyz, env_color0.xyz, _float(meshConst[0].envmapInterpolator.x) );

	_float3 normal_enc = normal * _float(0.5) + _float(0.5);

	out.color0 = _float4( texel_color.x,texel_color.y,texel_color.z, 0.0);
	out.color1 = _float4( normal_enc.x, normal_enc.y,normal_enc.z,0.0);
	out.color2 = _float4( env_color.x,env_color.y,env_color.z, mask.z);

	out.color3.xy = EncodeFloatRG( mask.y * _float(matConst[0].matparams_disiseri.y), 128.0);

	out.color3.zw = EncodeFloatRG( _float(matConst[0].matparams_disiseri.z), 4096.0);

#ifdef EMISSION
	out.color1.w = emission.x * _float(8.0);
#endif

#ifdef TRANSITION_EFFECT
	_float t = 0.0;

	if( _float(frameConst[0].view_posXYZ_time.w) > _float(0.37) )
	{
		t = ( _float(frameConst[0].view_posXYZ_time.w) - _float(0.37)) * _float(9.0);
	}

	t = clamp( t, _float(0.0), _float(1.0) );

	_float3 nnn = normalize( input.eye_space_normal);

	_float idx = fract( t * _float(10.0)) * _float(95.0);

	_float3 normal2 = normal * normal;
    //	vec3 secondary_color0 = texture( texture_array_unit0, vec3( out_world_pos.yz, idx)).xyz;
    //	vec3 secondary_color1 = texture( texture_array_unit0, vec3( out_world_pos.xz, idx)).xyz;
    //	vec3 secondary_color2 = texture( texture_array_unit0, vec3( out_world_pos.xy, idx)).xyz;

	_float3 secondary_color0 = texture_array_unit0.sample(arrsam0, hfloat2(input.world_pos.yz), idx).xyz;
	_float3 secondary_color1 = texture_array_unit0.sample(arrsam0, hfloat2(input.world_pos.xz), idx).xyz;
	_float3 secondary_color2 = texture_array_unit0.sample(arrsam0, hfloat2(input.world_pos.xy), idx).xyz;

	_float3 secondary_color = secondary_color0 * normal2.x + secondary_color1 * normal2.y + secondary_color2 * normal2.z;

	_float a = _float(2.5) * t + secondary_color.x;

	secondary_color *= _float3( _float(1.0) - nnn.z);
	secondary_color = _float(64.0) * powr( secondary_color, _float3( 2.0));

	out.color0.xyz = mix( texel_color, secondary_color * _float3( 0.2, 0.7, 1.0), (_float(1.0) - powr( t, _float(8.0) )));

	if(a < _float(1.0) )
	{
		discard_fragment();
	}


#ifdef EMISSION
	out.color1.w = mix( secondary_color.x * _float(128.0), emission.x, t);
#endif

#endif

#ifdef FRESNEL
	_float val = powr( _float(1.0)-dot(vtx_normal, _float3(-frameConst[0].view_dirXYZ_pad.xyz) ), _float(matConst[0].fresnelXYZ_transp.x) );

	_float texLum = dot(out.color0.xyz, _float3(0.3f, 0.59f, 0.11f));
	_float3 diffColNorm = out.color0.xyz / (texLum + hfloat(0.001f)); //avoid divide by zero

	val *= mask.x;

	out.color0.xyz = mix(out.color0.xyz, diffColNorm, _float(matConst[0].fresnelXYZ_transp.z) * clamp(val * _float(1.0),_float(0.0),_float(1.0) ));
	out.color1.w = val * _float(matConst[0].fresnelXYZ_transp.y); //emissive
#endif

	return out;
}



