/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> color_texture;
uniform sampler2D<float> normal_texture;
uniform sampler2D<float> specular_texture;
uniform float alpha_test_threshold;
uniform float4 view_pos;
uniform float4 view_dir;

uniform sampler2D<float> displacement_tex;
uniform float4 tessellation_factor; 
uniform float cam_near;
uniform float4 frustum_planes[6];
uniform float tessellation_multiplier;
uniform float4x4 mv;
uniform float4x4 model;
uniform float4x4 inv_model;
uniform float4x4 mvp;

//uniform float4 velocity_min_max_scale_factor;

#ifdef DEBUG_MIPMAP_LEVEL
uniform sampler2D<float> mip_texture;
in float2 texcoord_uv;
#endif

#ifdef TEXTURE_DENSITY
uniform float2 orig_texture_size;
uniform float4 min_ideal_max_texture_density;
#endif

in float2 texcoord;
in float3 world_position;
in float3 normal;
in float3 tangent;

out float4 out_color { color(0) };
out float4 out_normal { color(1) };
out float4 out_specular { color(2) };
//out float2 out_velocity { color(3) };
#if USE_SUBPASS && SHADER_METAL_IOS
out float out_depth { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#endif
void main()
{
#if USE_SUBPASS && SHADER_METAL_IOS
	out_depth = gl_FragCoord.z;
#endif

	// Color
	float4 color = texture(color_texture, texcoord);
#ifdef ALPHA_TEST
	float alfa = color.w;
	if( alfa < alpha_test_threshold)
	{
		discard;
	}
#endif
	out_color = float4(linear_to_srgb(color.xyz), color.w);

	// Normal + gloss
	float3 nn = normal;
#ifdef IS_TWO_SIDED
	if (!gl_FrontFacing)
	{
		nn *= -1.0;
	}
#endif
	float4 tex_norm_gloss = texture(normal_texture, texcoord);
	out_normal.xyz = encode_normal(calc_world_normal(tex_norm_gloss.xyz, nn, tangent));
	out_normal.w = tex_norm_gloss.w;

	// Specular + translucency
	float4 specular = texture(specular_texture, texcoord);
	out_specular = float4(linear_to_srgb(specular.xyz), specular.w);

	// Velocity
	//out_velocity = pack_velocity(velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor));
	//out_velocity = velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor);

	//---------------- DEBUG CODE -----------------

	color = float4(1.0);
#ifdef DEBUG_MIPMAP_LEVEL
//original texture colors mean its a perfect match (1:1 texels to pixels ratio)
//more red = too much texture detail
//more blue = too little texture detail
	float4 mip = texture( mip_texture, texcoord_uv);
	float3 col = mix(linear_to_srgb(color.xyz), mip.xyz, mip.w);
	out_color.xyz = col;
	out_color.w = color.w;
#endif


#ifdef TEXTURE_DENSITY
	float2 texcoordDensity = max( abs(dFdx(texcoord)), abs(dFdy(texcoord)));
	float3 worldSpaceDensity = max( abs(dFdx(world_position)), abs(dFdy(world_position)));
	float2 texCoordDensityPerWorldUnit = texcoordDensity / worldSpaceDensity.xy;

	float WorldSpaceArea = length( cross( dFdx(world_position.xyz), dFdy(world_position.xyz) ) );
	WorldSpaceArea = max( WorldSpaceArea, 0.00000001 );
	float2 tt = texcoord * orig_texture_size;
	float2 A = dFdx(tt);
	float2 B = dFdy(tt);
	float2 C = A.xy * B.yx;
	float TexelArea = abs( C.x - C.y );
	float Density = 0.00000001;
	Density = max( Density, TexelArea / WorldSpaceArea );

	float MinDensity = min_ideal_max_texture_density.x;
	float IdealDensity = min_ideal_max_texture_density.y;
	float MaxDensity = min_ideal_max_texture_density.z;

	if ( Density > IdealDensity )
	{
		float Range = MaxDensity - IdealDensity;
		Density -= IdealDensity;
		out_color = float4( Density / Range, (Range - Density) / Range, 0.0, 1.0 );
	}
	else
	{
		float Range = IdealDensity - MinDensity;
		Density -= MinDensity;
		out_color = float4( 0.0, Density/Range, (Range-Density)/Range, 1.0 );
	}
	float rr = get_luminance(float4(linear_to_srgb(color.xyz), color.w).xyz);
	rr = 1.0 - rr * 0.5;
	out_color *= rr;
	//out_color = mix(float4(linear_to_srgb(color.xyz), color.w), out_color, out_normal.y);
#endif

	/*
#ifdef IS_TWO_SIDED
	float3 V = view_pos.xyz - world_position.xyz;
	V = normalize(V);

//	V = -view_dir.xyz;
	V = normalize(V);

	float flatness = (dot(V, normalize(nn)));

//	flatness = pow(flatness, 3.0);

//	out_color = float4(flatness);

	float alfa2 = texture(color_texture, texcoord, -2.0).w;
	if( alfa2 * flatness < 0.2)
	{
	//	out_color = float4(1.0, 0.0, 0.0, 1.0);
	//	out_color = float4(flatness, 0.0, 0.0, 1.0);
	//	discard;
	}
#endif
*/


}
