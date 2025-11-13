/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;
uniform float4      inv_resolution;

//uniform sampler2D<float> gbuffer_depth_texture; //full res depth tex
//uniform float4      tap_offsets; //float res tex offset (1/w, 1/h, 0, 0)

in float2 texcoord;
out float4 out_color { color(0) };
void main()
{
#if 1
	float2 uv = texcoord.xy + (0.5 * inv_resolution.zw);

	float2 fraction = fract(inv_resolution.xy * uv.xy);
	uv.xy -= fraction * inv_resolution.zw + (0.5 * inv_resolution.zw);

	//weights ignored since they create unwanted black borders...
	float4 hdrVol00 = textureLodOffset(color_texture, uv, 0.0, int2(0, 0));// * weights.z;
	float4 hdrVol10 = textureLodOffset(color_texture, uv, 0.0, int2(1, 0));// * weights.y;
	float4 hdrVol01 = textureLodOffset(color_texture, uv, 0.0, int2(0, 1));// * weights.w;
	float4 hdrVol11 = textureLodOffset(color_texture, uv, 0.0, int2(1, 1));// * weights.x;

	float4 sampleX0 = mix(hdrVol00, hdrVol10, fraction.x);
	float4 sampleX1 = mix(hdrVol01, hdrVol11, fraction.x);
	out_color = mix(sampleX0, sampleX1, fraction.y);



/*
	float depth  = texture(gbuffer_depth_texture,texcoord).x;

	float4 depth4Diff;
	depth4Diff.x = texture(input_texture,texcoord+tap_offsets.xy).x;
	depth4Diff.y = texture(input_texture,texcoord+tap_offsets.xw).x;
	depth4Diff.z = texture(input_texture,texcoord+tap_offsets.zw).x;
	depth4Diff.w = texture(input_texture,texcoord+tap_offsets.zy).x;

	depth4Diff = abs(depth4Diff - depth);
	*/

	/**/
	//new one
	//TODO target slope threshold
	//float tolerance = 100.0;
	//float4 weights = min(1.0/(depth4Diff*tolerance+0.001), 1.0);

	//out_color = float4(weights.x>0.0, weights.y>0.0, weights.z>0.0, weights.w>0.0);
	//out_color = float4(1.0, 0.0, 0.0, 1.0);
	//out_color = float4(depth4Diff.x>1.0, depth4Diff.y>1.0, depth4Diff.z>1.0, depth4Diff.w>1.0);
	//out_color = weights;
	//return;


	/**/

	/**
	//old one
	float weightMultiplier = 1000.0;
	float4 weights = 1.0/(depth4Diff*weightMultiplier+1.0);
	weights *= 1.0/dot(weights, float4(1.0));

	float4 upsampleUvs = (floor((texcoord.xyxy+tap_offsets)*inv_resolution.xyxy)+0.5) * inv_resolution.zwzw;

	float2 upsampleOffset = weights.x*upsampleUvs.xy + weights.y*upsampleUvs.xw + weights.z*upsampleUvs.zw + weights.w*upsampleUvs.zy;

	float4 hdrVol = texture(color_texture,upsampleOffset);
	/**/

#else
	out_color = texture(color_texture, texcoord);
#endif
}