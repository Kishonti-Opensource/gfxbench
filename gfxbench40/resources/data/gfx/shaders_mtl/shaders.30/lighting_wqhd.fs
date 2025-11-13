/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

hfloat DecodeFloatRG(_float2 value, _float max_value)
{
	const hfloat max16int = 256.0 * 256.0 - 1.0;
	hfloat result = hfloat(255.0) * dot( hfloat2(value), hfloat2(256.0, 1.0)) / max16int;
	
	result *= max_value;
	
	return result;
}


hfloat3 get_world_pos(texture2d<hfloat> tex, sampler sam, _float2 out_texcoord, hfloat4 view_dir, constant FrameConsts* frameConsts)
{
	hfloat d = tex.sample(sam, hfloat2(out_texcoord) ).x;
	
	d = frameConsts->depth_parameters.y / (d - frameConsts->depth_parameters.x);
	
	return d * view_dir.xyz / view_dir.w + hfloat3(frameConsts->view_posXYZ_time.xyz);
}

fragment _float4 shader_main(LightingVertexOutput input [[stage_in]],
							constant FrameConsts* frameConst [[buffer(10)]],
#ifdef INSTANCING
							constant CameraConsts* cameraConst [[buffer(12)]],
							constant InstancedLightConsts* instancedLightConst [[buffer(13)]],
#else
							constant MeshConsts* meshConst [[buffer(12)]],
							constant LightConsts* lightConst [[buffer(13)]],
#endif
							texture2d<_float> unit0 [[texture(0)]],
							texture2d<_float> unit1 [[texture(1)]],
							texture2d<hfloat> unit3 [[texture(2)]],
							texture2d<_float> unit4 [[texture(3)]],
							depth2d<hfloat> shadow_unit0 [[texture(4)]],
							sampler sam0 [[sampler(0)]],
							sampler sam1 [[sampler(1)]],
							sampler sam3 [[sampler(2)]],
							sampler sam4 [[sampler(3)]],
							sampler shadowSampler [[sampler(4)]])
{
#ifdef INSTANCING
	LightConsts lightConst[1] ;

	
	//  struct LightConsts
	//  {
	//     hfloat4 light_colorXYZ_pad;
	//     hfloat4 light_posXYZ_pad;
	//     hfloat4 light_x;
	//     hfloat4 spotcosXY_attenZ_pad;
	//     hfloat4x4   shadowMatrix0;
	//  };
    //
	//  vec3  light_color           = lights[instanceID].light_color.xyz;
	//  vec3  light_pos             = lights[instanceID].light_pos.xyz;
	//  float attenuation_parameter = lights[instanceID].attenuation_parameter.x;
	//  vec4  light_x               = lights[instanceID].light_x;
	//  vec2  spot_cos              = lights[instanceID].spot_cos.xy;
	//  vec3  view_pos              = view_posXYZ_normalized_time.xyz;


	lightConst[0].light_colorXYZ_pad   = hfloat4( instancedLightConst[input.instanceID].light_color.xyz, 1.0 ) ;
	lightConst[0].light_posXYZ_pad     = hfloat4( instancedLightConst[input.instanceID].light_pos.xyz,   1.0 ) ;
	lightConst[0].light_x              =          instancedLightConst[input.instanceID].light_x ;
	lightConst[0].spotcosXY_attenZ_pad = hfloat4( instancedLightConst[input.instanceID].spot_cos.xy,
												  instancedLightConst[input.instanceID].attenuation_parameter.x,
												  1.0) ;

#endif

	_float4 frag_color;
	_float2 out_texcoord = (input.out_pos.xy / input.out_pos.w * 0.5) + _float2(0.5,0.5);
#if 1
    out_texcoord.y = 1.0 - out_texcoord.y;
#endif
	_float shadow = 1.0;
	
	_float3 view_dir = normalize( input.view_dir.xyz);
	
#if defined (POINT_LIGHT) || defined (SPOT_LIGHT)

    hfloat3 position = get_world_pos(unit3, sam3,out_texcoord, hfloat4(input.view_dir), frameConst);
    
	_float3 light_dir = _float3(lightConst[0].light_posXYZ_pad.xyz) - _float3(position);
	_float sq_distance = dot( light_dir, light_dir);

	_float atten = lightConst[0].spotcosXY_attenZ_pad.z * sq_distance + 1.0;
	
	if( atten <= _float(0.0) )
	{
#ifdef OSX
		discard_fragment();
#else
		return _float4(0.0,0.0,0.0,0.0);
#endif
	}

	atten = clamp( atten, _float(0.0), _float(1.0) );

	light_dir = light_dir / sqrt( sq_distance);
#endif

#if defined POINT_LIGHT

#if defined SHADOW_MAP
	
//shadows don't ever seem turned on here.
#error Should not hit this!
	
	_float4 p = _float4( position, 1.0);
	_float4 shadow_texcoord = lightConst[0].shadowMatrix0 * p;
	
	_float4 projectedShadow = shadow_texcoord / shadow_texcoord.w;
	
	shadow_texcoord.xy = shadow_texcoord.xy * 0.5 + _float2(0.5);
	
	//flip y in shadow lookup?
	shadow_texcoord.y = _float(1.0f) - shadow_texcoord.y;
	
	_float refvalue = clamp(shadow_texcoord.z, 0.0f, 1.0f);
	
	shadow = shadow_unit0.sample_compare(shadowSampler, projectedShadow.xy, refvalue);
	
	if( shadow == 0.0)
	{
#ifdef OSX
		discard_fragment();
#else
		return _float4(0.0,0.0,0.0,0.0);
#endif
	}
	
	atten *= shadow;
	
#endif
	
#elif defined SPOT_LIGHT
	_float fov_atten = dot( light_dir, _float3(-lightConst[0].light_x.xyz) );
	
	fov_atten = fov_atten - lightConst[0].spotcosXY_attenZ_pad.x;
	
	if( fov_atten <= _float(0.0) )
	{
#ifdef OSX
		discard_fragment();
#else
		return _float4(0.0,0.0,0.0,0.0);
#endif
	}
	
	//fov_atten /= (1.0 - 0.9);

	fov_atten *= _float(lightConst[0].spotcosXY_attenZ_pad.y);
	
	atten *= fov_atten;
	
#if defined SHADOW_MAP
	
	//NOTE: same as above
#error Shadows Don't work!
	
	_float4 p = float4(position, 1.0);
	_float4 shadow_texcoord = lightConst[0].shadowMatrix0 * p;
	
	_float4 projectedShadow = shadow_texcoord / shadow_texcoord.w;
	
	shadow_texcoord.xy = shadow_texcoord.xy * float(0.5) + float2(0.5);
	
	//flip y in shadow lookup?
	shadow_texcoord.y = 1.0f - shadow_texcoord.y;
	
	_float refvalue = clamp(shadow_texcoord.z, 0.0f, 1.0f);
	
	shadow = shadow_unit0.sample_compare(shadowSampler, projectedShadow.xy, refvalue);
	
	if( shadow == 0.0)
	{
#ifdef OSX
		discard_fragment();
#else
		return _float4(0.0,0.0,0.0,0.0);
#endif
	}
	
	atten *= shadow;
	
#endif
	
	
#else
	_float3 light_dir = lightConst[0].light_x.xyz;
	const float atten = 1.0;
#endif
	
	_float3 half_vector = normalize( light_dir - view_dir);
	
	_float4 normal = unit1.sample(sam1, hfloat2(out_texcoord) );
	normal.xyz = normal.xyz * _float(2.0) - _float(1.0);
	normal.xyz = normalize( normal.xyz);
	_float diffuse = dot( normal.xyz, light_dir);
	
#ifdef SPECIAL_DIFFUSE_CLAMP
	diffuse = diffuse * _float(0.5) + _float(0.5);
#else
	diffuse = clamp( diffuse, _float(0.0), _float(1.0) );
#endif
	
	_float specular = dot( normal.xyz, half_vector);
	specular = clamp( specular, _float(0.0), _float(1.0) );
	
	_float4 float_params = unit4.sample(sam4, hfloat2(out_texcoord) );
	_float A = DecodeFloatRG( float_params.rg, 128.0);
	_float B = DecodeFloatRG( float_params.ba, 4096.0);
	
	specular = A * powr( specular, B);
	//specular = A * powr( specular, B);
	//float specpower = exp2(B * log2(specular));
	
#ifdef HALF_SIZED_LIGHTING
#error half sized light?
	frag_color = _float4( diffuse * lightConst[0].light_colorXYZ_pad.xyz * atten, diffuse * specular * atten);
#else
	_float4 texel_color = unit0.sample(sam0, hfloat2(out_texcoord) );
	_float3 c = (texel_color.xyz * diffuse * _float(3.0) + specular) * _float3(lightConst[0].light_colorXYZ_pad.xyz) * atten;
	
	frag_color = _float4( c, 1.0);
#endif

#ifdef SV_31
	RGBA10A2_ENCODE(frag_color.xyz) ;
#endif
    
    return frag_color;
}



