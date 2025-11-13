/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

//#define per_pixel_random_offset

#ifdef per_pixel_random_offset
constant const _float offsets[16] = {
	0.52, 0.37, 0.28, 0.68,
	0.19, 0.03, 0.33, 0.17,
	0.38, 0.54, 0.83, 0.90,
	0.78, 0.31, 0.18, 0.47
};
#endif


fragment _float4 shader_main(FogVertexOutput input [[stage_in]],
							constant FrameConsts* frame [[buffer(10)]],
							constant LightConsts* light [[buffer(13)]],
							texture2d<_float> unit0 [[texture(0)]],
							texture2d<hfloat> unit1 [[texture(1)]],
							sampler sam0 [[sampler(0)]],
							sampler sam1 [[sampler(1)]])
{
	//frag_color = vec4(0.0, 1.0, 0.0, 0.9);
	//return;
#if 0
    return _float4(1.0, 0.0, 0.0, 1.0);
#else
    _float2 depth_texcoord = (_float2(input.out_pos_hom.xy) / _float(input.out_pos_hom.w) * _float(0.5) ) + _float2(0.5,0.5);
    
    hfloat d = unit1.sample(sam1, hfloat2(depth_texcoord) ).x;
    hfloat z_view_depth = frame[0].depth_parameters.y / (d - frame[0].depth_parameters.x);
    
	_float t = _float(frame[0].view_posXYZ_time.w) * _float(8.0);
	_float final_atten = 0.0;
	
	_float v = 0.0;
	_float v2 = 0.0;
	
	_float prev_fov_cos = -1000.0; // cos(tracing point cone angle) of previous step;
	_float prev_atten_distance = -1000.0;  // previous axial cone attenuation value
	
	_float3 world_pos = _float3(input.out_pos.xyz); // world position for following texture fetch from shadow_texture
	_float3 light_dir = _float3(input.out_pos.xyz) - _float3(light[0].light_posXYZ_pad.xyz);
	hfloat3 ray_dir   = input.out_pos.xyz - hfloat3(frame[0].view_posXYZ_time.xyz);
	_float3 ray_dir_norm = _float3(normalize(ray_dir));
	_float3 ray_dir_step = ray_dir_norm; // scale up if needed
	
	hfloat start_depth = dot(ray_dir, hfloat3(frame[0].view_dirXYZ_pad.xyz) );
	
	_float offset = 0.0;
#ifdef per_pixel_random_offset
#error
	// add random offset of up 1.0 length to starting vector position in 4x4 grid.
	vec<uint, 2> off = vec<uint, 2>(fract(_float2( input.position.xy * 0.25 ))*4.0);
	//offset = offsets[off.x << 2 + off.y];
	//TODO: this may be a bug - << has lower precedence than '+', which do they mean to happen first?
	offset = _float( _float(1.0)*offsets[(off.x << 2) + off.y]);
	// * fract(z_view_depth); // adds even more randomness per-pixel
	light_dir -= offset * ray_dir_step;
	world_pos -= offset * ray_dir_step;
#endif
	
	hfloat max_iter = (z_view_depth - start_depth);
	if (max_iter > hfloat(1024.0))
	{
		max_iter = hfloat(1024.0);
	}
	
	hfloat i;
	
	for (i= hfloat(0.0); i<max_iter; i+=hfloat(1.0)) // trace only distance to opaque object
	{
		_float sq_distance = dot(light_dir, light_dir);
		//vec3 light_dir_norm = light_dir * inversesqrt(sq_distance);
		_float3 light_dir_norm = light_dir / sqrt(sq_distance);
		
		_float atten = light[0].spotcosXY_attenZ_pad.z * sq_distance + 1.0;
		
		// check for exit due distance (exit via bottom of cone)
		if ((atten < _float(0.0)) && (prev_atten_distance > atten)) {
			v2 = _float(1.0);
			break;
		}
		prev_atten_distance = atten; // if we're going inside cone - previous distance will be smaller vs attenuated distance (it's going from 0.0 to 1.0 near light source)
		// if ((atten > _float(1.1) ) break;
		
		atten = clamp( atten, _float(0.0), _float(1.0) ); //todo: saturate?
		
		_float fov_atten = dot(light_dir_norm, _float3(light[0].light_x.xyz) );
		
		fov_atten = fov_atten - _float(light[0].spotcosXY_attenZ_pad.x);
		
		// check for exit due angle (exit via side of cone)
		if ((fov_atten < 0.0) && (prev_fov_cos > fov_atten)) {
			v = 1.0;
			break;
		}
		prev_fov_cos = fov_atten; // if we're going inside cone - previous cos() will be smaller vs new cos() value of angle
		
		fov_atten *= _float(light[0].spotcosXY_attenZ_pad.y);
		
		atten *= fov_atten;
		
		if (atten > _float(1.0)/_float(255.0) ) // if we're above threshold - modulate by noise texture
		{
			//TODO why even calculate the shadow coords in the vertex shader if you aren't ever going to use them?

            _float4 world_pos4 =  _float4(world_pos, 1.0);
			_float4 shadow_texcoord_f = _float4x4(light[0].shadowMatrix0) * world_pos4;
			
			_float3 out_texcoord0 = shadow_texcoord_f.xyz / shadow_texcoord_f.w;
			
			_float shadow_depth = unit0.sample(sam0, hfloat2(out_texcoord0.xy + _float2( t, 0.0))  ).x;
			atten *= shadow_depth;
		}
		
		final_atten += atten * (_float(1.0) - final_atten);
		
		// advance our positions by raymarching step
		light_dir += ray_dir_step;
		world_pos += ray_dir_step;
	}

    _float4 retVal = _float4( _float3(light[0].light_colorXYZ_pad.xyz), final_atten) * _float(0.5);
    
#ifdef SV_31
	RGBA10A2_ENCODE(retVal.xyz) ;
	retVal = max(retVal, _float4(0.0)) ;
#endif
    
	return retVal;
	
	//frag_color.yw = vec2(i/_float(60.0), _float(1.0)); //debug - visualize number of steps
	//if (v > 0) frag_color.xw = vec2(v); //debug - color red pixels exited via cone sides
	//if (v2 > 0) frag_color.zw = vec2(v2); //debug - color blue pixels exited via cone bottom
	//frag_color = vec4( final_atten, 0, 0, 1); // visualize just attenuation
	
	//frag_color = vec4(1.0);
	
	// depth visualization checks
	//frag_color = vec4(z_view_depth/200, start_depth, 0, 1.0);
	//frag_color = vec4(0, (z_view_depth - start_depth)/200, 0, 1.0);
#endif
}



