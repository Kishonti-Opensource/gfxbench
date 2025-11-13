/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<half> color_texture;
uniform sampler2D<half> normal_texture;
uniform sampler2D<half> specular_texture;

#ifdef ALPHA_TEST
uniform float alpha_test_threshold;
#endif

uniform float4 velocity_min_max_scale_factor;

#ifdef DEBUG_MIPMAP_LEVEL
uniform sampler2D<float> mip_texture;
in float2 texcoord_uv;
#endif

#ifdef TEXTURE_DENSITY
uniform float2 orig_texture_size;
uniform float4 min_ideal_max_texture_density;
#endif

#if COLORIZE_SHADOW_CASTERS
uniform float4 debug_frusum_ids[8];
#endif

#if COLORIZE_LOD_LEVELS
uniform int debug_mesh_lod_index;
#endif

#if WIREFRAME
uniform float debug_wireframe_mode;
#endif

in float2 texcoord;

#ifdef TEXTURE_DENSITY
in float3 world_position;
#endif

// PREC_TODO: half?
in float3 normal;
in float3 tangent;
in float4 clip_space_pos;
in float4 clip_space_prev_pos;

out half4 out_color { color(0) };
out half4 out_normal { color(1) };
out half4 out_specular { color(2) };
out float2 out_velocity { color(3) };
#if defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
out float out_depth { color(GBUFFER_COLOR_DEPTH_RT_INDEX) };
#endif
void main()
{
#if defined(GBUFFER_COLOR_DEPTH_RT_INDEX)
	out_depth = gl_FragCoord.z;
#endif

//#ifdef DITHERED
#if 0
	if (dither_value > 0.0)
	{
		float random = cos(dot(floor(gl_FragCoord.xy) , float2(347.83451793,3343.28371963)));
		random = fract(random * 1024.0);

		if(dither_value > random)
		{
			discard;
		}
	}
#endif
//#endif

	// Color
	half4 color = texture(color_texture, texcoord);
#ifdef ALPHA_TEST
	if( color.w < half(alpha_test_threshold))
	{
		discard;
	}
#endif
	out_color = color;

	// Normal + gloss
	half3 nn = half3(normal);
#ifdef IS_TWO_SIDED
	if (!gl_FrontFacing)
	{
		nn *= -1.0h;
	}
#endif
	half4 tex_norm_gloss = texture(normal_texture, texcoord);
	out_normal.xyz = encode_normal(calc_world_normal(tex_norm_gloss.xyz, nn, half3(tangent)));
	out_normal.w = tex_norm_gloss.w;

	// Specular + translucency
	out_specular = texture(specular_texture, texcoord);

	// Velocity
	//out_velocity = pack_velocity(velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor));
	out_velocity = velocity_function(clip_space_pos, clip_space_prev_pos, velocity_min_max_scale_factor);

	//---------------- DEBUG CODE -----------------
#ifdef DEBUG_MIPMAP_LEVEL
//original texture colors mean its a perfect match (1:1 texels to pixels ratio)
//more red = too much texture detail
//more blue = too little texture detail
	float4 mip = texture( mip_texture, texcoord_uv);
	float3 col = mix(float3(1.0), mip.xyz, mip.w);
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
	float rr = get_luminance((color.xyz));
	rr = 1.0 - rr * 0.6;
	out_color *= rr;
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

#if COLORIZE_SHADOW_CASTERS
	{
		float3 debug_color = float3(0.0);
		for (int i = 0; i < 6; i++)
		{
			if (debug_frusum_ids[i].x < 0.5)
			{
				continue;
			}
			debug_color += get_debug_color(i) * 0.5;
		}
		out_color.xyz += debug_color;
	}
#endif


#if COLORIZE_LOD_LEVELS
	if (debug_mesh_lod_index > 0)
	{
		float3 debug_color = get_debug_color(2 - debug_mesh_lod_index) * 0.5;
		out_color.xyz += debug_color;
	}
#endif

#if WIREFRAME
	if (debug_wireframe_mode > 0.5)
	{
		out_color = float4(1.0);
		out_specular = float4(1.0, 1.0, 1.0, 0.0);
	}
#endif
}
