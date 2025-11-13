/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct Fill2InOut
{
	hfloat4 v_Position [[position]];
	v_float2 v_texCoord0;
	v_float2 v_texCoord1;
	v_float2 v_texCoord2;
};

#ifdef TYPE_vertex

struct VertexUniforms
{
	hfloat4 offsets; // pos_x-y offsets, u-v offsets
	hfloat rotation;
	hfloat aspectRatio;
	
	hfloat _pad1;
	hfloat _pad2;
};	

struct VertexInput
{
    hfloat4 in_vertex [[attribute(0)]];
};

_float2x2 rotateZ(_float alpha)
{
	return _float2x2(
	_float2(cos(alpha), -sin(alpha)),
	_float2(sin(alpha), cos(alpha)));
}

Fill2InOut vertex shader_main(         VertexInput     in [[ stage_in  ]],
							  constant VertexUniforms& vu [[ buffer(1) ]])
{
	Fill2InOut res;

#ifdef RESIZE_PASS
	res.v_texCoord0 = v_float2(in.in_vertex.z, 1.0 - in.in_vertex.w);
	res.v_Position = _float4(in.in_vertex.xy * 2.0, 0.0, 1.0);	
#else	
	#ifdef CUBE_DEPTH_PASS
		_float2 aspect_uv = in.in_vertex.zw * _float2(vu.aspectRatio, 1.0);			
		res.v_texCoord0 = v_float2(in.in_vertex.zw);
		
		_float2x2 rot1 = rotateZ(-vu.rotation);
		res.v_texCoord1 = v_float2(aspect_uv * rot1); 
		
		_float2x2 rot2 = rotateZ(-vu.rotation * 0.5);
		res.v_texCoord2 = v_float2(aspect_uv * rot2);
				
		res.v_Position = _float4(in.in_vertex.xy * 2.0, 0.0, 1.0);		
	#else		
		_float2 aspect_uv = in.in_vertex.zw * 0.5 + vu.offsets.zw;
		aspect_uv *= _float2(vu.aspectRatio, 1.0);
		
		res.v_texCoord0 = v_float2(aspect_uv + _float2(vu.rotation * -0.3, 0.0));		
		
		_float2x2 rot = rotateZ(-vu.rotation * 0.2);
		res.v_texCoord1 = v_float2(aspect_uv * rot);
		
		res.v_texCoord2 = v_float2(aspect_uv + _float2(vu.rotation * -0.1, 0.0));
		res.v_texCoord2 *= 0.25;
		
		res.v_Position = _float4(in.in_vertex.xy + vu.offsets.xy, 0.0, 1.0);
	#endif	
#endif

	return res;
}
#endif

#ifdef TYPE_fragment

fragment _float4 shader_main(Fill2InOut          in            [[ stage_in   ]],
							 texture2d<_float>   texture_unit0 [[ texture(0) ]],
							 sampler             sampler0      [[ sampler(0) ]],
#ifdef CUBE_DEPTH_PASS
							 texturecube<_float> envmap0       [[ texture(1) ]],
#else
							 texture2d<_float>   texture_unit1 [[ texture(1) ]],
#endif
                             sampler             sampler1      [[ sampler(1) ]],
#ifdef CUBE_DEPTH_PASS
							 texture2d<hfloat>   texture_unit2 [[ texture(2) ]],
#else
							 texture2d<_float>   texture_unit2 [[ texture(2) ]],
#endif
							 sampler             sampler2      [[ sampler(2) ]],
							 texture2d<_float>   texture_unit3 [[ texture(3) ]],
							 sampler             sampler3      [[ sampler(3) ]])
{
	_float4 frag_color;
#ifdef RESIZE_PASS
	frag_color = texture_unit0.sample(sampler0, hfloat2(in.v_texCoord0.x,  in.v_texCoord0.y));	
#elif defined CUBE_DEPTH_PASS
	_float4 sample0 = texture_unit0.sample(sampler0, hfloat2(in.v_texCoord0));
	_float4 sample1 = envmap0.sample(sampler1, hfloat3( hfloat2(in.v_texCoord1), 1.0 - in.v_texCoord1.x)); // cube map
	_float4 sample2 = _float4(texture_unit2.sample(sampler2, hfloat2(in.v_texCoord2)).x); // depth texture
	_float4 sample3 = texture_unit3.sample(sampler3, hfloat2(in.v_texCoord0));
	_float4 layers = sample1 * _float(0.02) + sample2 * _float(0.02);
	frag_color = _float4(sample0.xyz + layers.xyz, sample3.w);		
#else
	_float4 sample0 = texture_unit0.sample(sampler0, hfloat2(in.v_texCoord0));	
	_float4 sample1 = texture_unit1.sample(sampler1, hfloat2(in.v_texCoord0));	
	_float4 sample2 = texture_unit2.sample(sampler2, hfloat2(in.v_texCoord1));	
	_float4 sample3 = texture_unit3.sample(sampler3, hfloat2(in.v_texCoord2));	
	
	_float4 layers = mix(sample3, sample2,sample2.w);
	layers += layers * sample1 * _float(0.5) + sample0 * _float(0.01);
	frag_color = _float4(layers.xyz, 1.0);	
#endif
	return frag_color;
}
#endif
