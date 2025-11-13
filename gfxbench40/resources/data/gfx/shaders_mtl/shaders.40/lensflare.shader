/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct VertexInput
{
	hfloat2 in_position [[attribute(0)]];
	hfloat2 in_texcoord0 [[attribute(1)]];
};

struct VertexOutput
{
	hfloat4 v_position [[position]];
	hfloat2 v_texcoord;
};

struct LensFlareConstants
{
	hfloat4x4 mvp;
	hfloat2 sun_dir;
	hfloat flare_size;
	hfloat flare_distance;
};

#ifdef TYPE_vertex


vertex VertexOutput shader_main(VertexInput input [[stage_in]],
								constant LensFlareConstants * lf_consts [[buffer(1)]],
								constant uint & _visibility [[buffer(2)]])
{
	VertexOutput output;

	// flare strength 0.0 .. 1.0
	_float strength = _float( _visibility )/_float(MAX_SAMPLE) ;

	// minimum size
	_float minimum_size = 0.6 ;
	
	strength = (1.0 - minimum_size )*strength + minimum_size ;
	
	_float2 vc = input.in_position.xy ;
	vc *= strength * lf_consts->flare_size ;
	vc += -lf_consts->flare_distance * lf_consts->sun_dir ;
	
#if VIEWPORT_HEIGHT > VIEWPORT_WIDTH
	vc = _float2(-vc.y, vc.x);
#endif

	_float4 out_pos = lf_consts->mvp * _float4( vc, 0.0, 1.0) ;
	
	output.v_position = out_pos ;
	output.v_texcoord = input.in_texcoord0;

	return output;
}

#endif


#ifdef TYPE_fragment

_float4 texture2DDistorted(texture2d<hfloat> Texture, _float2 TexCoord, _float2 Offset)
{
	constexpr sampler tex_sampler(coord::normalized, min_filter::nearest, mag_filter::linear, mip_filter::linear, address::repeat);

	return _float4(
		Texture.sample(tex_sampler, hfloat2(TexCoord + Offset * -1.0)).r,
		Texture.sample(tex_sampler, hfloat2(TexCoord + Offset * 0.0)).g,
		Texture.sample(tex_sampler, hfloat2(TexCoord + Offset * 1.0)).b,
		1.0
	);
}

fragment _float4 shader_main(VertexOutput input [[stage_in]],
							constant LensFlareConstants * lf_consts [[buffer(1)]],
							constant uint & _visibility [[buffer(2)]],
							texture2d<hfloat> color_tex [[texture(0)]],
							texture2d<hfloat> dirt_tex [[texture(1)]])
{
	_float4 color ;
	_float strength = _float( _visibility )/_float(MAX_SAMPLE) ;
	
	
	_float maximum_stength = 0.20 ;
	_float distortion_strength = 0.02 ;
	

	strength = clamp(strength,_float(0.0),_float(1.0)) ;
	strength = maximum_stength * strength ;
	
#if VIEWPORT_WIDTH > VIEWPORT_HEIGHT
	_float2 screen_coord = _float2(input.v_position.x/_float(VIEWPORT_WIDTH), 1.0 - input.v_position.y/_float(VIEWPORT_HEIGHT) ) ; // [AAPL] Y-FLIP
#else
	_float2 screen_coord = _float2(1.0 - input.v_position.y/_float(VIEWPORT_HEIGHT), 1.0 - input.v_position.x/_float(VIEWPORT_WIDTH) ) ;
#endif
	
	_float2 distortion = abs(2.0 * screen_coord - 1.0) ;
	
	distortion *= distortion_strength ;
	
	color = strength * texture2DDistorted(color_tex, _float2(input.v_texcoord), distortion) * _float4( 5.0 *texture2DDistorted(dirt_tex, screen_coord, distortion ).xyz + _float3(0.5) , 1.0) ;
	
	return _float4( color.xyz, 0.0 );
}

#endif
