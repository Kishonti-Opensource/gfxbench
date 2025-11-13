/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_frame.h"
#include "ubo_instance.h"

#include "rgbm_helper.h"

// TODO: add these to UBOs
uniform mat4 view; //camera
uniform mat4 vp; //for instancing // camera
uniform mat4 prev_vp; //for instancing // camera
uniform vec2 view_port_size; //camera
uniform float near_far_ratio; // camera

uniform mat4 mvp; // mesh
uniform mat4 mvp2; //for motion blur, mesh
uniform mat4 mv; //mesh
uniform mat4 model; //mesh
uniform mat4 inv_model; //mesh
uniform mat4 inv_view;	//to convert world normals to view

uniform mat4 dpcam_view;

uniform vec3 light_color; // light
uniform vec3 light_pos; // light

uniform vec4 gamma_exp;

// Samplers
uniform sampler2D texture_unit0;
uniform sampler2D texture_unit1;
uniform sampler2D texture_unit2;
uniform sampler2D texture_unit3;

uniform sampler2D texture_unit4;
uniform sampler2D texture_unit5;
uniform sampler2D texture_unit6;
uniform sampler2D texture_unit7;

uniform sampler2D depth_unit0;

// Occlusion cull
uniform sampler2D hiz_texture;

#ifdef EDITOR_MODE
uniform int editor_mesh_selected;
const vec3 editor_select_color = vec3(1.2, 0.2, 0.2);
#endif

//uniform samplerCube envmap0;
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
uniform samplerCubeArray static_envmaps;
#endif

uniform sampler2DArray envmap1_dp;
uniform sampler2DArray envmap2_dp;


#define VELOCITY_BUFFER_TYPE vec4

#define PI 3.141592654

uniform mat4 car_ao_matrix0; 
uniform mat4 car_ao_matrix1;

// Scene bounding area is about 4000 meters, but shadow far is 220m, SSAO far is 500m
#define MAX_LINEAR_DEPTH 512.0 
#define MAX_BYTE 255.0

/*highp*/ vec2 EncodeLinearDepthToVec2(/*highp*/ float v )
{
	v /= MAX_LINEAR_DEPTH;
	
	v *= MAX_BYTE;
	
	/*highp*/ vec2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}

/*highp*/ float DecodeLinearDepthFromVec2(/*highp*/ vec2 v )
{
	/*highp*/ float r255 = v.x * MAX_BYTE + v.y;
	return MAX_LINEAR_DEPTH * r255 / MAX_BYTE;
}

#define MAX_FLOAT 4.0
vec2 packFloatToVec2(float v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) /2.0;
	
	v *= MAX_BYTE;
	
	vec2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}

highp vec2 packFloatToVec2Highp(highp float v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) /2.0;
	
	v *= MAX_BYTE;
	
	highp vec2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}


vec4 pack2FloatToVec4(vec2 v)
{
	vec2 ex = packFloatToVec2(v.x);
	vec2 ey = packFloatToVec2(v.y);
	
	return vec4(ex,ey);
}

highp vec4 pack2FloatToVec4Highp(highp vec2 v)
{
	highp vec2 ex = packFloatToVec2Highp(v.x);
	highp vec2 ey = packFloatToVec2Highp(v.y);
	
	return vec4(ex,ey);
}


highp float unpackFloatFromVec2Highp(highp vec2 v)
{
	highp float r255 = v.x*MAX_BYTE+v.y;
	highp float x = r255 / MAX_BYTE;
	return MAX_FLOAT * (  2.0*x-1.0  );
}


highp vec2 unpackVec2FFromVec4Highp(highp vec4 value)
{
	return vec2(unpackFloatFromVec2Highp(value.xy),unpackFloatFromVec2Highp(value.zw));
}


// The max velocity value(mb_velocity_min_max_sfactor_pad.y) will be never bigger than 0.05
#define MAX_VELOCITY 0.05
//#define MAX_VELOCITY mb_velocity_min_max_sfactor_pad.y
highp vec4 pack_velocity(highp vec2 velocity)
{
#if VELOCITY_BUFFER_RGBA8
	return pack2FloatToVec4(velocity);
#else
	return vec4(velocity / (2.0 * MAX_VELOCITY) + 0.5, 0.0, 0.0);
#endif
}
 
#if VELOCITY_BUFFER_RGBA8 
#define VELOCITY_PREC lowp
#else
#define VELOCITY_PREC mediump
#endif
highp vec2 unpack_velocity(VELOCITY_PREC sampler2D velocityMap, vec2 uv)
{
	vec4 v = texture(velocityMap, uv);
#if VELOCITY_BUFFER_RGBA8
	return unpackVec2FFromVec4Highp(v);
#else
	return (v.xy - 0.5) * (2.0 * MAX_VELOCITY);
#endif
}

vec2 velocityFunction(highp vec4 scPos,highp vec4 prevScPos)
{
	const float blur_strength = 0.5 ;

	highp vec2 a = scPos.xy / scPos.w ;
	highp vec2 b = prevScPos.xy / prevScPos.w ;

	highp vec2 qx = a-b;
	
	qx *= blur_strength ;
	
	#define EPS 0.0001
	highp float qx_length = length(qx) ;

	if (qx_length < EPS)
	{
		qx = vec2(0.0) ;
	}
	else
	{
		qx = ( qx * clamp( qx_length, mb_velocity_min_max_sfactor_pad.x, mb_velocity_min_max_sfactor_pad.y ) ) / ( qx_length + EPS) ;
	}
		
	vec2 res = qx ;
	
	return res ;
}

///////////////////////////////////////////////////////////////////////////////
//		CASCADED SHADOWS
///////////////////////////////////////////////////////////////////////////////

//TODO: use 3 + static shadow map, but fix top-down projections on overhanging rocks
//	to change:
//		- cascaded_shadow_matrices uniform upload @ glb_filter.cpp
//		- CASCADE_COUNT @ cascaded_shadowmap.h 
//		
#define CASCADE_COUNT 4

uniform highp mat4 cascaded_shadow_matrices[CASCADE_COUNT];
uniform highp vec4 cascaded_frustum_distances;
uniform highp sampler2DArrayShadow cascaded_shadow_texture_array;

#if (CASCADE_COUNT == 3)
int getCascadeIndex(float fragment_depth)
{	
	highp vec3 distances_h = vec3(fragment_depth) - cascaded_frustum_distances.xyz;
    vec3 distances = distances_h * 100000.0;
    distances = clamp(distances, vec3(0.0), vec3(1.0));

	return int(dot(distances, vec3(1.0)));
}
#else
int getCascadeIndex(float fragment_depth)
{	
	highp vec4 distances_h = vec4(fragment_depth) - cascaded_frustum_distances;
    vec4 distances = distances_h * 100000.0;
    distances = clamp(distances, vec4(0.0), vec4(1.0));

	return int(dot(distances, vec4(1.0)));
}
#endif

int getMapBasedCascadeIndex(highp vec4 world_pos, out highp vec4 shadow_coord)
{
    shadow_coord = cascaded_shadow_matrices[0] * world_pos;
    if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
    {
        return 0;
    }
    shadow_coord = cascaded_shadow_matrices[1] * world_pos;
    if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
    {
        return 1;
    }
    shadow_coord = cascaded_shadow_matrices[2] * world_pos;
    if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
    {
        return 2;
    }
    shadow_coord = cascaded_shadow_matrices[3] * world_pos;
    if ( min(shadow_coord.x, shadow_coord.y) >= 0.0 && max(shadow_coord.x, shadow_coord.y) < 1.0)
    {
        return 3;
    }
    return 4;
}

float shadowStatic(vec4 world_pos)
{
    highp vec2 uv = world_pos.xz / 1950.0;
    uv += vec2(-0.5, 0.5);

    float baked_shadow = texture(texture_unit7, uv).y;
    //baked_ao = clamp( baked_ao + 0.0, 0.0, 1.0);    
    return baked_shadow;
}

float shadowCascaded(highp float fragment_depth, highp vec4 world_pos)
{    
    /*
    // Distance based selection
	const float bias_values[4] = float[4]
	(
        -0.0011,
        -0.0011,
        -0.0011,
		0.0001	
	);
    */
    // Map based selection
    const float bias_values[4] = float[4]
	(
        0.000,
        0.000,
		0.0001,
		0.0001
	);

    highp vec4 shadow_coord;
    
    // Distance based selection
    //int index = getCascadeIndex(fragment_depth);
    // Project to light projection space
    //shadow_coord = cascaded_shadow_matrices[index] * world_pos;
    
    // Map bases selection
    int index = getMapBasedCascadeIndex(world_pos, shadow_coord);
#if (CASCADE_COUNT == 3)    
    index = clamp(index, 0, 3);
    if (index > 2)
    {
       return shadowStatic(world_pos);
    }
#else    
    if (index > 3)
    {
        return 1.0;        
    }
#endif
		
    // Apply the bias and sample the shadow map
	highp float z = shadow_coord.z - bias_values[index];
	shadow_coord.z = float(index);
	shadow_coord.w = z;
    
	float shadow_dist = 0.0;
	//PCF
	//shadow_dist = texture(cascaded_shadow_texture_array, shadow_coord.xyzw);
	
	//PCFx4
	vec4 gather = textureGather(cascaded_shadow_texture_array, shadow_coord.xyz, z);
	shadow_dist = dot(gather, vec4(0.25));
	
	//PCFx16
	/*
	vec4 gather1 = textureGather(cascaded_shadow_texture_array, shadow_coord.xyz, z);
	vec4 gather2 = textureGather(cascaded_shadow_texture_array, shadow_coord.xyz + vec3(0.0 / 1024.0, 1.0 / 1024.0, 0.0), z);
	vec4 gather3 = textureGather(cascaded_shadow_texture_array, shadow_coord.xyz + vec3(1.0 / 1024.0, 0.0 / 1024.0, 0.0), z);
	vec4 gather4 = textureGather(cascaded_shadow_texture_array, shadow_coord.xyz + vec3(1.0 / 1024.0, 1.0 / 1024.0, 0.0), z);
	shadow_dist = dot(gather1, vec4(0.25) * 0.25);
	shadow_dist += dot(gather2, vec4(0.25) * 0.25);
	shadow_dist += dot(gather3, vec4(0.25) * 0.25);
	shadow_dist += dot(gather4, vec4(0.25) * 0.25);
	*/	
    
	// Shadow attenuation
#if (CASCADE_COUNT == 3)
    if (index > 1)
    {
        float border = cascaded_frustum_distances.z - 0.0001;
        float attenuation = clamp((fragment_depth - border) / 0.0001, 0.0, 1.0);
        shadow_dist = mix(shadow_dist, shadowStatic(world_pos), attenuation);
    }
#else
	float border = cascaded_frustum_distances.w - 0.00001;
	float attenuation = clamp((fragment_depth - border) / 0.00001, 0.0, 1.0);
	shadow_dist = mix(shadow_dist, 1.0, attenuation);
#endif
	
	return shadow_dist;
}

///////////////////////////////////////////////////////////////////////////////
//		POST & SCREEN-SPACE EFFECTS
///////////////////////////////////////////////////////////////////////////////
uniform vec4 depth_parameters;
uniform vec3 view_pos;
uniform highp vec3 corners[4];

highp float rand(highp vec2 co)
{
	//screen-space
	return (fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453));
}

highp vec4 encodeNormal(highp vec3 N)
{
#if NORMAL_BUFFER_RGBA8
	highp float temp = 8.0*(N.z)+8.0;
	
	highp float val = sqrt(abs(temp)); //abs: calling sqrt on <0 is undefined
	return pack2FloatToVec4Highp(N.xy / vec2(val) + vec2(0.5));
#else
	return vec4(0.5 * N + 0.5, 1.0);
#endif
}

#if NORMAL_BUFFER_RGBA8 
#define NORMAL_PREC lowp
#else
#define NORMAL_PREC mediump
#endif
highp vec3 decodeNormal(NORMAL_PREC sampler2D normalMap, vec2 uv)
{
	highp vec4 g_normal = texture(normalMap, uv);

#if NORMAL_BUFFER_RGBA8 
	highp vec2 a = unpackVec2FFromVec4Highp(g_normal)*vec2(4.0)-vec2(2.0);
	highp float b = dot(a,a);
	highp float val = 1.0-b/4.0;
	highp float c = sqrt(abs(val)); //abs: calling sqrt on <0 is undefined
	
	highp vec3 N;
	N.xy = a*vec2(c);
	N.z = 1.0-b/2.0;
	return N;
#else
	return 2.0 * g_normal.xyz - 1.0;
#endif
}

highp float getLinearDepth(highp sampler2D depthTex, highp vec2 uv)
{
	highp float d = texture( depthTex, uv).x;
	d = depth_parameters.y / (d - depth_parameters.x);

	return d;
}

highp float getLinearDepth(highp float depth)
{
	return depth_parameters.y / (depth - depth_parameters.x);
}

highp vec3 getPositionVS(highp float linear_depth, highp vec2 uv)
{
	highp vec3 dir0;
	highp vec3 dir1;
	highp vec3 dir2;
	
	dir0 = mix( corners[0].xyz, corners[1].xyz, uv.x);
	dir1 = mix( corners[2].xyz, corners[3].xyz, uv.x);
	
	dir2 = mix( dir0, dir1, uv.y);
	return linear_depth * dir2;
}

highp vec3 getPositionVS(highp sampler2D depthTex, highp vec2 uv) 
{
	highp float d  = getLinearDepth(depthTex, uv); 
	return getPositionVS(d,uv);
}

highp vec3 getPositionWS(highp sampler2D depthTex, highp vec2 uv) 
{
	//NOTE: corners[4] are different between VS and WS case
	return getPositionVS(depthTex, uv) + view_pos;
}

highp vec3 getPositionWS(highp float linear_depth, highp vec2 uv) 
{
	//NOTE: corners[4] are different between VS and WS case
	return getPositionVS(linear_depth, uv) + view_pos;
}

///////////////////////////////////////////////////////////////////////////////
//		SHADING
///////////////////////////////////////////////////////////////////////////////
float getLum(vec3 inCol)
{
	return dot(vec3(0.212671, 0.71516, 0.072169), inCol);
}

vec3 RGBMtoRGB( vec4 rgbm ) 
{
	vec3 dec = 6.0 * rgbm.rgb * rgbm.a;
	return dec*dec; //from "gamma" to linear
}

vec3 RGBEtoRGB( vec4 rgbe) 
{ 
	float e = (rgbe.a - 0.5) * 256.0;
	e = exp2( e);
	vec3 origVal = rgbe.rgb * vec3( e);
	//float origLum = getLum(origVal);
	//float boostedLum = origLum + pow(max(origLum - 3.0, 0.0), 3.0); //brightness bright pixels even more (10 -> 1000)
	//vec3 boostedVal =  origVal * boostedLum / origLum;
	return origVal;
} 


void decodeFromByteVec3(inout vec3 myVec)
{
#ifdef UBYTE_NORMAL_TANGENT
	myVec = 2.0 * myVec - 1.0;
#endif
} 

mat3 calcTBN(vec3 normal, highp vec3 tangent)
{
	vec3 n = normalize( normal);
	vec3 t = normalize( tangent);
	vec3 b = normalize( cross( t, n));
	return mat3( t, b, n);
}


vec3 calcWorldNormal(vec3 texNorm, vec3 normal, vec3 tangent)
{
	vec3 ts_normal = texNorm * 2.0 - 1.0;
	
	mat3 tbn = calcTBN(normal,tangent) ;

	return normalize( tbn * ts_normal);
}

float shadow( highp sampler2DShadow s, vec4 tc)
{
	return textureProj( s, tc) * 1.0 + 0.0;
}

float shadowBias( highp sampler2DShadow s, vec4 tc, float bias)
{
	tc.z -= bias;
	return textureProj( s, tc) * 1.0 + 0.0;
}

///////////////////////////////////////////////////////////////////////////////
//		LIGHTING
///////////////////////////////////////////////////////////////////////////////

//Physically-based specular
float getFresnel(float HdotL, float specular_color) //Schlick Fresnel
{
	float base = 1.0 - HdotL;
	float exponential = pow( base, 5.0 );
	float fresnel_term = specular_color + ( 1.0 - specular_color ) * exponential;
	return fresnel_term;
}

float getSpecular(vec3 N, vec3 L, vec3 V, float NdotL_01, float specular_color, float roughness)
{
	float specExp = pow (2.0, (1.0 - roughness) * 10.0 + 2.0);

	vec3 H = normalize(V + L);
	float NdotH = clamp(dot(N, H),0.0,1.0);
	float HdotL = dot(H, L); //can never go above 90, no need to clamp

	float normalisation_term = ( specExp + 2.0 ) / 8.0;
	float blinn_phong = pow( NdotH, specExp );
	
	float specular_term = normalisation_term * blinn_phong;	
	float cosine_term = NdotL_01;
	
	float fresnel_term = getFresnel(HdotL, specular_color);

	float specular = specular_term * cosine_term * fresnel_term; // visibility_term, but it is factor out as too costy
	return specular;
}


highp vec3 applyFog( in vec3  origCol,
               in highp float dist,
               in highp vec3  rayDir)
{
    highp float fogAmount = exp(-(dist * dist) * 0.000001);
	highp float fogTintFactor = pow( clamp(dot(-rayDir, global_light_dir.xyz),0.0,1.0),4.0);
	highp vec3 finalFogCol = mix(fogCol.xyz, global_light_color.xyz, fogTintFactor);
    return mix(finalFogCol, origCol, clamp(fogAmount, 0.0, 1.0) );
}

vec3 getHemiAmbient(vec3 N)
{
	return mix(ground_color.xyz, sky_color.xyz, clamp(N.y,0.0,1.0));
}

vec3 getAmbientCubeColor(vec3 N, float cubeIdx)
{
	int offset = int(cubeIdx) * 6;

	vec3 nSquared = N * N;
	int isNegativeX = N.x < 0.0 ? 1 : 0;
	int isNegativeY = N.y < 0.0 ? 1 : 0;
	int isNegativeZ = N.z < 0.0 ? 1 : 0;
	vec3 linearColor = 
        nSquared.x * ambient_colors[offset + isNegativeX].xyz + 
        nSquared.y * ambient_colors[offset + isNegativeY+2].xyz + 
        nSquared.z * ambient_colors[offset + isNegativeZ+4].xyz;
	return linearColor; 

	//return mix(ground_color.xyz, sky_color.xyz, clamp(N.y,0.0,1.0));
}

float getEnvLodLevel(float roughness, vec3 view_dir, vec3 N)
{
	float view_length = length(view_dir);
		
	float level = dot(view_dir / view_length, N);
	level = clamp(level, 0.0, 1.0);
	
	level = 1.0 - level;
	
	level = pow(level, 64.0);
		
	view_length = clamp(view_length * 0.001, 0.0, 1.0);
	
	level += roughness;
	level = mix(level, 1.0, view_length);
	level *= 10.0; // max lod level
	
	return level;
}

//#define STATIC_ENVPROBE_CAPTURE

//NOTE: until more lights need to be combined with the IBL sun, uses IBL specular only
//		until convoluted diffuse envmaps are available (even a single one with sky), uses analytic diffuse/ambient
vec3 getPBRambient(vec3 N, float cubeIdx)
{
	return getHemiAmbient(N);
}

//simple version for realtime cubemap render pass
vec3 getPBRambient(vec3 N)
{
	return getHemiAmbient(N);
}

vec3 getPBRdiffuse(vec3 N, vec3 L, vec3 V, float two_sided_lighting, float shadow,highp vec3 world_pos)
{
	//NOTE: diffuse is almost zero for pure metals
	float translucent_addition = 0.0;
	if(two_sided_lighting > 0.5)
	{
		highp vec2 uv = world_pos.xz / 1950.0;
		uv += vec2(-0.5, 0.5);
		
		float baked_shadow = texture(texture_unit7, uv).y;
	
		translucent_addition = (two_sided_lighting - 0.5) * 2.0 * (clamp(dot(-N,-V),0.0,1.0)) * (baked_shadow + 1.0) * 0.5;
	}
	else if(two_sided_lighting > 0.0)//billboard
	{
		translucent_addition = two_sided_lighting * 2.0 * (1.0 - clamp(dot(N, L),0.0,1.0)); //brightens if facing against the sun
	}
	
	return global_light_color.xyz * 2.0 * (clamp(dot(N, L),0.0,1.0) * shadow + translucent_addition) / PI; ////global_light_color.xyz * 2.0 * sat(dot(N, L)) / PI * (1.0 + two_sided_lighting * sat(dot(V,L)));
}

//reflections are stronger when surface is looked upon in a steep angle
float FresnelSchlickWithRoughness(float specCol, vec3 V, vec3 N, float roughness)
{
	return specCol * (1.0 + 1.0 * (max(roughness, specCol) - specCol) * pow(1.0 - clamp(dot(V, N), 0.0, 1.0), 5.0));
}

vec4 getParaboloid(in lowp sampler2DArray sampler, vec3 R, float envLod)
{
	//transform to DP-map space (z = fwd, y = up)
	
	//vec3 normalWS = normalize(inv_view * vec4(normalVS,0.0)).xyz;
	
	vec3 Rdp = normalize(dpcam_view * vec4(R, 0.0)).xyz;

	if(Rdp.z>0.0)
	{
		vec2 frontUV = (Rdp.xy / (2.0*(1.0 + Rdp.z))) + 0.5;
		return textureLod( sampler, vec3(frontUV, 0.0), envLod );
	}
	else
	{
		vec2 backUV = (Rdp.xy / (2.0*(1.0 - Rdp.z))) + 0.5;
		return textureLod( sampler, vec3(backUV, 1.0), envLod );
	}	

}

vec3 PBRhelper(float roughness, float specular_color, float cube_index, vec3 view_dir, highp vec3 N, highp vec3 V, vec3 diffuse, vec3 ambient, float shadow, float ssao, vec3 albedo, vec3 emissive)
{
	//NOTE: specular is almost zero for non-metals
	
	highp vec3 R = reflect(-V, N);	
	vec3 result = vec3(0.0);
	vec3 specular = vec3(0.0);

	roughness = clamp(roughness, 0.08, 1.0);

	float envLod = getEnvLodLevel(roughness, view_dir, N);
	float mipIntensityCorrection = (1.0 + clamp(envLod, 0.0, 10.0) / 10.0 * 1.0);
	float viewDepBrighten = FresnelSchlickWithRoughness(specular_color, V, N, roughness);
	
	float specStrength = specular_color;
	float shadow2 = mix(shadow, 1.0, pow(specStrength, 0.5));
	
	float cube_expanded = cube_index;
	if(cube_expanded < 100.0)
	{
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
		vec4 reflect_color = textureLod(static_envmaps, vec4( R, cube_expanded), envLod);
		specular = RGBMtoRGB(reflect_color).xyz;
#else
		// EXT_texture_cube_map_array is not supported
		specular = vec3(0.5);
#endif
		
/*		if(abs(cube_expanded - 7.0) < 0.4) //HACK: 7th location is the garage, sample dyn cube of car to fake light coming from the opening door
		{
			specular = textureLod(envmap2, R, envLod).xyz;
		}
*/		
#ifdef STATIC_ENVPROBE_CAPTURE
		specular = vec3(0.0);
#endif	
		specular *= mipIntensityCorrection; //glGenerateMips darkens the image
		specular *= viewDepBrighten;		

		//specular = mix(getSpecular(N, global_light_dir, V, sat(dot(N, global_light_dir)), specular_color, roughness).xxx, specular, specular_color);
				
		//NOTE: shadow2 for specular dimming is not needed if we have enough cubemaps
		//NOTE: because cubemap just approximates the general environment and is not correctly respecting self-occlusion and reflection, ao is used to darken it
		result = specular * shadow2 * ssao;
		
		result += (diffuse + emissive + ambient) * albedo;
	}
	else if (cube_expanded < 101.0)
	{
		//Water shading
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
		vec4 reflect_color = texture(static_envmaps, vec4( R, 0.0)); //Sky cube map
		specular = RGBMtoRGB(reflect_color).xyz;		
#else
		// EXT_texture_cube_map_array is not supported
		specular = vec3(0.5);
#endif
		
#ifdef STATIC_ENVPROBE_CAPTURE
		specular = vec3(0.0);
#endif	
		//specular *= mipIntensityCorrection; //glGenerateMips darkens the image
		specular *= viewDepBrighten;		
		
		result = specular;// * shadow2 * ssao;
		result += (diffuse + emissive + ambient) * albedo;
	}
	else //dynamic cubemaps
	{	
		float reflFix = 1.0 - pow(1.0 - clamp(dot(V, N), 0.0, 1.0), 50.0); //HACK to correct car mesh / normal issues
	
		float isPaint = 0.0;
		if(cube_expanded < 253.5) //car 0, 252 or 253
		{			
			isPaint = cube_expanded > 252.5 ? 1.0 : 0.0;
			
			vec3 envmap1_color = RGBMtoRGB_cubemap( getParaboloid(envmap1_dp, R, envLod) ) ;
			specular = envmap1_color * mipIntensityCorrection; //NOTE: clamp is here to reduce reflection vector noise at edges, and white spots from envmap reads
#ifdef STATIC_ENVPROBE_CAPTURE
			specular = vec3(0.0);
#endif			
			
			specular *= viewDepBrighten;
			//specular *= ssao;
		}
		else //car 1, 254 or 255
		{
			isPaint = cube_expanded > 254.5 ? 1.0 : 0.0;

			vec3 envmap2_color = RGBMtoRGB_cubemap(  getParaboloid(envmap2_dp, R, envLod) ) ;		
			specular = envmap2_color * mipIntensityCorrection;
#ifdef STATIC_ENVPROBE_CAPTURE
			specular = vec3(0.0);
#endif						
			specular *= viewDepBrighten;
			//specular *= ssao;
		}
		
		specular *=  reflFix;
		
		//decide if we are shading car paint - based on G-Buffer encoded data
		if(isPaint > 0.5)
		{
			float  fresnel1 = clamp( dot( N, V ),0.0,1.0);
			float  fresnel2 = clamp( dot( N, V ),0.0,1.0); 
		   

			float dist = 1.0 - clamp(length(view_dir)/4.0, 0.0, 1.0);
			float flakeDim = 0.1 * mix(0.5, 1.0, dist);
			float  fresnel1Sq = mix(fresnel1 * fresnel1, 0.0, clamp(dot(V, global_light_dir.xyz),0.0,1.0));

			albedo = 		/*(fresnel1) * */albedo /*color*/ + 
								fresnel1Sq * albedo * vec3( 1.15) +
								fresnel1Sq * fresnel1Sq * albedo * vec3( 1.17);// +
								//pow( fresnel2, 16.0 ) * flakeLayerColor * flakeDim;		
		
		
			float envContrib = pow(clamp(dot(N, V),0.0,1.0),0.1);
			result = mix(specular * 1.0, (diffuse + emissive + ambient) * albedo, /*0.5 + 0.5 * */envContrib);
		}
		else
		{
			result = (diffuse + emissive + ambient) * albedo + specular * pow(ssao, 2.0);
		}
	}
			
	//TODO IBL based diffuse, analytic specular for extra lights (computed via MicrofacetBRDF)
	
	// Analytical specular
	//specular += MicrofacetBRDF(roughness, specular_color, N, L_extra, V);
	
	return result;
}

vec3 getPBRlighting(highp vec3 view_dir, highp vec3 N, float roughness, float cube_index, float specular_color, vec3 albedo, float emissive_translucency, float bakedao, float ssao, float shadow, vec3 originWS)
{
	highp vec3 V = normalize(view_dir);
	vec3 L = global_light_dir.xyz;

	float emissive = 0.0;
	float two_sided_lighting = 0.0;
	if(cube_index > 49.5 && cube_index < 99.5)
	{
		//add two-sided lighting
		two_sided_lighting = emissive_translucency;
	}
	else
	{
		emissive = pow(emissive_translucency * emissive_translucency, 3.0) * 20.0; //first decompress from G-buffer encode, then make emissive source data nonlinear, then give range for bloom
	}
	
	vec3 diffuse = getPBRdiffuse(N, L, V, two_sided_lighting, shadow, originWS);
	vec3 ambient = getPBRambient(N, cube_index);
	
	//vec3 result_color = PBRhelper(roughness, specular_color, cube_index, view_dir, N, V, albedo * diffuse * shadow, vec3(emissive) + albedo * ambient * ao, shadow, ao);
	float ao = min(bakedao, ssao);
	highp vec3 result_color = PBRhelper(roughness, specular_color, cube_index, view_dir, N, V, diffuse, ambient * ao, shadow, ssao, albedo, vec3(emissive)); //ssao dims reflections

	//NOTE: 1. metals: 		the color is largely given by specular, shadow does not affect it
	//		2. non-metals:	specular is affected by diffuse
	
	return result_color;
}


struct _bezier_patch
{
	mat4 Px;
	mat4 Py;
	mat4 Pz;
};

const mat4 BT = mat4(
	vec4( -1.0, 3.0,-3.0, 1.0),
	vec4(  3.0,-6.0, 3.0, 0.0),
	vec4( -3.0, 3.0, 0.0, 0.0),
	vec4(  1.0, 0.0, 0.0, 0.0)
);
