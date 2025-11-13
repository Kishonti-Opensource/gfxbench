/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec3 global_light_dir;
uniform vec3 global_light_color;
uniform vec3 view_dir;
uniform vec3 background_color;
uniform float time;
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;
uniform sampler2D texture_unit4;
uniform float diffuse_intensity;
uniform float specular_intensity;
uniform float specular_exponent;
uniform float reflect_intensity;
uniform samplerCube envmap0;
uniform samplerCube envmap1;
uniform float envmaps_interpolator;
uniform float transparency;
uniform sampler2D shadow_unit0;
uniform sampler2D shadow_unit1;
uniform samplerCube normalization_cubemap;
uniform vec2 inv_resolution;
uniform float mblur_mask;

varying vec2 out_texcoord0;
varying vec2 out_texcoord1;
varying vec2 out_texcoord4;
varying vec2 out_texcoord5;
varying float out_texcoord4z;
varying float out_texcoord5z;
varying vec3 out_view_dir;
varying vec3 out_normal;
varying vec3 out_tangent;
varying float fog_distance;
varying float dof;
varying vec2 out_texcoord01;
varying vec2 out_texcoord02;
varying vec2 out_texcoord03;
varying vec2 out_texcoord04;



#if defined RELIEF

void ray_intersect_rm_linear( inout vec3 p, inout vec3 v)
{
	const int linear_search_steps =	30;

	v /= float(linear_search_steps);

	for( int i=0; i<linear_search_steps-1; i++)
	{
		vec4 t = texture2D( texture_unit2, p.xy);
		
		if (p.z < t.x )
		{
			p += v;
		}
	}
}

void ray_intersect_rm_binary( inout vec3 p, inout vec3 v)
{
	const int binary_search_steps=4;
   
	for( int i=0; i<binary_search_steps; i++)
	{
		v *= 0.5;
		vec4 t = texture2D( texture_unit2, p.xy);

		if (p.z < t.x )
		{
			p += 2.0 * v;
		}
		p -= v;
	}
}


vec2 relief(vec2 uv, vec3 vd)
{
	vec3 v;
	vec3 t = normalize( out_tangent);
	vec3 n = normalize( out_normal);
	vec3 b = cross( n, t);

	t = -t;

	v.x = dot( vd, vec3( t.x, t.y, t.z));
	v.y = dot( vd, vec3( b.x, b.y, b.z));
	v.z = dot( vd, vec3( n.x, n.y, n.z));
	
	vec3 scale = vec3( 1.0, 1.0, 0.025);
	v *= scale.z / ( scale * v.z);
	
	vec3 p = vec3( uv, 0.0);
	
	ray_intersect_rm_linear( p, v);
	ray_intersect_rm_binary( p, v);
		
	return p.xy;
}

vec2 parallax(vec2 uv, vec3 vd)
{
	vec3 v;
	vec3 t = normalize( out_tangent);
	vec3 n = normalize( out_normal);
	vec3 b = cross( n, t);

	t = -t;

	v.x = dot( vd, vec3( t.x, t.y, t.z));
	v.y = dot( vd, vec3( b.x, b.y, b.z));
	v.z = dot( vd, vec3( n.x, n.y, n.z));
	
	float h = texture2D( texture_unit2, uv).x;
	
	//h -= 0.5;
	h *= 0.05;
	
	return uv + v.xy * h;
}


#endif


vec3 normalize2( vec3 v)
{
#ifdef USE_NORMALIZATION_CUBE_MAP
	vec3 r = textureCube( normalization_cubemap, v).xyz;
	r = r * 2.0 - 1.0;
	return r;
#else
	return normalize( v);
#endif
}


#ifndef ZPREPASS

#ifdef LIGHTMAP
vec3 lightmapping()
{
	return texture2D( texture_unit1, out_texcoord1).xyz * 1.5;
}
#endif




#if defined LIGHTING || defined REFLECTION || defined RELIEF || defined XRAY
vec3 calculate_normal( vec2 tc)
{
	vec4 ts_normal;

#ifdef ANIMATE_NORMAL
	vec4 ts_normal0 = texture2D( texture_unit3, out_texcoord01);
	vec4 ts_normal1 = texture2D( texture_unit3, out_texcoord02);
	vec4 ts_normal2 = texture2D( texture_unit3, out_texcoord03);
	vec4 ts_normal3 = texture2D( texture_unit3, out_texcoord04);

	ts_normal = (ts_normal0 + ts_normal1 + ts_normal2 + ts_normal3) / 4.0;
#else
	ts_normal = texture2D( texture_unit3, tc);
#endif

	ts_normal.xyz = (ts_normal.xyz * 2.0) - 1.0;

	ts_normal.xyz = normalize2( ts_normal.xyz);

	vec3 t = normalize2( out_tangent);
	vec3 n = normalize2( out_normal);
	vec3 b = cross( n, t);
	mat3 m = mat3( t, b, n);

	vec3 result = m * ts_normal.xyz;

	return result;
}
#endif


#if defined SHADOW_MAP && defined LIGHTING

float DecodeFloatRGB( vec3 rgba ) 
{
	return dot( rgba, vec3(1.0, 1.0/255.0, 1.0/65025.0) );
}

float shadow()
{
#if defined RGB_ENCODED
	float color0 = DecodeFloatRGB( texture2D( shadow_unit0, out_texcoord4.xy).xyz);
	float color1 = DecodeFloatRGB( texture2D( shadow_unit1, out_texcoord5.xy).xyz);
#else
	float color0 = texture2D( shadow_unit0, out_texcoord4.xy).x;
	float color1 = texture2D( shadow_unit1, out_texcoord5.xy).x;
#endif

    if( out_texcoord4z < 1.0 )
        if( out_texcoord4z > 0.0)
            if( color0 < out_texcoord4z )
				return 0.33;
    
    if( out_texcoord5z < 1.0 )
        if( out_texcoord5z > 0.0)
            if( color1 < out_texcoord5z )
				return 0.33;
    
    return 1.0;
    
}
#endif


#ifdef LIGHTING
vec3 lighting( vec3 color, vec3 normal, vec3 view_vector, vec3 light_dir, vec3 light_color, float specular_mask)
{
	vec3 half_vector = normalize2( view_vector + light_dir);

	float ambient = 0.1;
#if defined WIDE_DIFFUSE_CLAMP
	float diffuse = (dot( normal, light_dir) * 0.5 + 0.5) * diffuse_intensity;
#else
	float diffuse = clamp( dot( normal, light_dir), 0.0, 1.0) * diffuse_intensity;
#endif
	float specular = clamp( dot( normal, half_vector), 0.0, 1.0);
	specular = specular_intensity * pow( specular, specular_exponent) * specular_mask;

#if defined SHADOW_MAP
	float s = shadow();
	
	diffuse = diffuse * s;

	specular = specular * s;
#endif

	vec3 result = (color * (ambient + diffuse) + specular) * light_color;
	return result;
}

#endif



#if defined REFLECTION
vec3 reflection( vec3 color, vec3 normal, vec3 view_vector, float reflection_mask)
{
	vec3 reflect_vector = reflect( -view_vector, normal);
	vec4 reflection0 = textureCube( envmap0, reflect_vector);
	vec4 reflection1 = textureCube( envmap1, reflect_vector);
	
	vec3 reflection_color = mix( reflection1.xyz, reflection0.xyz, envmaps_interpolator);
	
	vec3 result = mix( color, reflection_color, reflect_intensity * reflection_mask);
	
	return result;
}
#endif

void main()
{   
#if defined RELIEF
	vec3 view_dir2 = normalize2( out_view_dir );
	vec2 tc = parallax( out_texcoord0, view_dir2 );
#else
	vec2 tc = out_texcoord0;
#endif


#if defined MASK

#ifdef ANIMATE_NORMAL
	vec4 mask0 = texture2D( texture_unit2, out_texcoord01);
	vec4 mask1 = texture2D( texture_unit2, out_texcoord02);
	vec4 mask2 = texture2D( texture_unit2, out_texcoord03);
	vec4 mask3 = texture2D( texture_unit2, out_texcoord04);

	vec4 mask = (mask0 + mask1 + mask2 + mask3) / 4.0;
#else
	vec4 mask = texture2D( texture_unit2, tc);
#endif

#else
	const vec4 mask = vec4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < 0.5)
	{
		discard;
	}
#endif

	vec3 color = texture2D( texture_unit0, tc).xyz;


	

#if defined LIGHTING || defined REFLECTION || defined RELIEF || defined XRAY
	vec3 view_dir = normalize2( out_view_dir);
	vec3 normal = calculate_normal( tc);
#endif


#ifdef LIGHTING
	color = lighting( color, normal, view_dir, global_light_dir, global_light_color, mask.y);	
#endif //LIGHTING

#ifdef LIGHTMAP
	color *= lightmapping();
#endif

	
#if defined REFLECTION
	color = reflection( color, normal, view_dir, mask.z);
#endif //REFLECTION


#if defined FOG
	color = mix( color, background_color, fog_distance * fog_distance);
#endif	

	
#if defined TRANSPARENCY
	float trp = mask.x * transparency;
	gl_FragColor = vec4( color, trp);
#else
	gl_FragColor = vec4( color, mblur_mask);
#endif

#if defined DOF
	gl_FragColor.w = dof;
#endif

#if defined XRAY
	float d = 1.0 - abs( dot( normalize( out_normal), normalize( -out_view_dir)));
	d = pow( d, reflect_intensity);
	//gl_FragColor = vec4( d );
	vec3 ghost = mix( vec3(0.0), color, d);
	gl_FragColor.xyz = ghost;
#endif
	
	gl_FragColor += texture2D(  texture_unit4, tc);
	
	float lum = dot( gl_FragColor.xyz, vec3( 0.5, 0.333, 0.333));
	gl_FragColor.w = lum;
	
	//gl_FragColor = vec4( normalize( out_normal), 1.0);
}

#else

void main()
{   
#if defined RELIEF
	vec3 view_dir2 = normalize2( out_view_dir );
	vec2 tc = parallax( out_texcoord0, view_dir2 );
#else
	vec2 tc = out_texcoord0;
#endif


#if defined MASK

#ifdef ANIMATE_NORMAL
	vec4 mask0 = texture2D( texture_unit2, out_texcoord01);
	vec4 mask1 = texture2D( texture_unit2, out_texcoord02);
	vec4 mask2 = texture2D( texture_unit2, out_texcoord03);
	vec4 mask3 = texture2D( texture_unit2, out_texcoord04);

	vec4 mask = (mask0 + mask1 + mask2 + mask3) / 4.0;
#else
	vec4 mask = texture2D( texture_unit2, tc);
#endif

#else
	const vec4 mask = vec4( 1.0, 1.0, 1.0, 1.0);
#endif

#if defined ALPHA_TEST
	if( mask.x < 0.5)
	{
		discard;
	}
#endif

	gl_FragColor = vec4(1.0);
}

#endif
