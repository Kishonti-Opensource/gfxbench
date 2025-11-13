/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

fragment _float4 shader_main(PPVertexOutput input [[stage_in]],
							constant FrameConsts* frameConst [[buffer(10)]],
							texture2d<_float> unit0 [[texture(0)]],
							texture2d<_float> unit1 [[texture(1)]],
							texture2d<_float> unit2 [[texture(2)]],
							texture2d<_float> unit3 [[texture(3)]],
#ifdef HALF_SIZED_LIGHTING
							texture2d_depth<float> unit4 [[texture(4)]],
#endif
							sampler sam0 [[sampler(0)]],
							sampler sam1 [[sampler(1)]],
							sampler sam2 [[sampler(2)]],
							sampler sam3 [[sampler(3)]])
{
	_float4 frag_color;
	
	_float4 c0;
	_float4 c1;
	_float4 c2;
	_float4 c3;
	
	_float4 color_texel_a0 = unit0.sample(sam0, hfloat2(input.texcoord0));
	_float4 color_texel_a1 = unit1.sample(sam1, hfloat2(input.texcoord0));
	_float4 color_texel_a2 = unit2.sample(sam2, hfloat2(input.texcoord0));
	_float4 color_texel_a3 = unit3.sample(sam3, hfloat2(input.texcoord0));
	
#ifdef SV_31
	color_texel_a2 = RGBA10A2_DECODE(color_texel_a2) ;
#endif
	
#ifdef HALF_SIZED_LIGHTING
	vec<uint, 2> texcoord = vec<uint, 2>(input.texcoord0 / frameConst[0].inv_resolutionXY_pad.xy / 2.0);
	_float4 l0 = unit2.read(texcoord);
	_float4 l1 = unit2.read(texcoord + vec<uint,2>(  1,  0));
	_float4 l2 = unit2.read(texcoord + vec<uint,2>(  0,  1));
	_float4 l3 = unit2.read(texcoord + vec<uint,2>( -1,  0));
	_float4 l4 = unit2.read(texcoord + vec<uint,2>(  0, -1));

	_float d0 = unit4.read(texcoord).x;
	_float d1 = unit4.read(texcoord + vec<uint,2>(  1,  0)).x;
	_float d2 = unit4.read(texcoord + vec<uint,2>(  0,  1)).x;
	_float d3 = unit4.read(texcoord + vec<uint,2>( -1,  0)).x;
	_float d4 = unit4.read(texcoord + vec<uint,2>(  0, -1)).x;

	_float diff1 = fabs(d0 - d1);
	_float diff2 = fabs(d0 - d2);
	_float diff3 = fabs(d0 - d3);
	_float diff4 = fabs(d0 - d4);

	_float eps = 1.0f;
	_float w0 = 1.0f;
	_float w1 = 1.0f / (eps + diff1 * 10000.0);
	_float w2 = 1.0f / (eps + diff2 * 10000.0);
	_float w4 = 1.0f / (eps + diff4 * 10000.0);
	_float w3 = 1.0f / (eps + diff3 * 10000.0);
	
	_float tw = w0 + w1 + w2 + w3 + w4;
	
	color_texel_a2 = (w0 * l0 + w1 * l1 + w2 * l2 + w3 * l3 + w4 * l4) / tw;
#endif
	
	c0 = color_texel_a0;
	c1 = color_texel_a1;
	c2 = color_texel_a2;
	c3 = color_texel_a3;
	
#ifdef SV_30
	_float3 ambiLightCol = _float3(0.0/255.0, 30.0/255.0, 50.0/255.0);
#endif

#ifdef SV_31
	_float3 ambiLightCol = _float3(0.0/255.0, 15.0/255.0, 20.0/255.0);
#endif
	
#ifdef HALF_SIZED_LIGHTING
	
	c2 *= _float(3.0f);
	
	_float lum = dot(c2.xyz, _float3(0.3f, 0.59f, 0.11f)) + _float(0.001f); //avoid divide by zero
	_float3 diffChrominance = c2.xyz / lum; //reconstruct light color to modulate spec scalar for l-buffer
	_float3 diffCol = c2.xyz;
	_float3 specCol = c2.w * diffChrominance * _float(0.3f);
	_float3 matCol = c0.xyz;
	//vec4 final = vec4(matCol * (ambiLightCol + diffCol + specCol), 1.0f);
	_float4 final = float4(matCol * (ambiLightCol + diffCol) + specCol, 1.0f);
	
	final = mix( final, c3, c3.a); //final vs refl
	final = mix( final, c0, c1.a); //(final vs refl) vs boosted albedo (emissive)
	
	frag_color.rgb = final.xyz;
	frag_color.a = c1.a;
#else
	c2.xyz += c0.xyz * ambiLightCol;
	
	c2 = mix( c2, c3, c3.a);
	c2 = mix( c2, c0, c1.a);
	
	frag_color = c2;
	frag_color.a = c1.a;
#endif

#ifdef SV_31
	RGBA10A2_ENCODE(frag_color.xyz) ;
#endif
	
	return frag_color;
}


