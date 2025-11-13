/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "rgbm_helper.h"



#define VERTEX_UNIFORMS_SLOT		1
#define INSTANCE_UNIFORMS_SLOT		2
#define TESSELLATION_UNIFORMS_SLOT	3
#define USER_PER_PATCH_SLOT			4
#define INDEX_BUFFER_SLOT			6
#define VERTEX_BUFFER_SLOT			7
#define TESS_FACTORS_SLOT			8
#define CONTROL_DATA_SLOT			9
#define ORIGNAL_DRAWS_SLOT			10
#define DRAWS_SLOT 					11
#define DRAW_IDX_COUNT_SLOT			12
#define DRAW_IDX_OFFSET_SLOT		13
#define OCCLUSION_UNIFORMS_SLOT		14
#define COMMAND_IDX_SLOT            15
#define DRAW_PATCH_INDIRECT_SLOT    16

#define FRAME_UNIFORMS_SLOT			0
#define FRAGMENT_UNIFORMS_SLOT		1

#define FILTER_UNIFORMS_SLOT		1
#define LUMINANCE_BUFFER_SLOT		2

#define NUM_STATIC_ENVMAPS			9

struct _MTLDrawPatchIndirectArguments
{
    uint patchCount;
    uint instanceCount;
    uint patchStart;
    uint baseInstance;
};

#define RGBA10A2_ENCODE_CONST 0.25
#define RGBA10A2_DECODE_CONST 4.00

#define RGBA10A2_ENCODE(X) X *= RGBA10A2_ENCODE_CONST;
#define RGBA10A2_DECODE(X) X * RGBA10A2_DECODE_CONST


enum TextureBindings
{
	TEXTURE_UNIT0_SLOT = 0,
	TEXTURE_UNIT1_SLOT = 1,
	TEXTURE_UNIT2_SLOT = 2,
	TEXTURE_UNIT3_SLOT = 3,
	TEXTURE_UNIT4_SLOT = 4,
	TEXTURE_UNIT5_SLOT = 5,
	TEXTURE_UNIT6_SLOT = 6,
	TEXTURE_UNIT7_SLOT = 7,
	DEPTH_UNIT0_SLOT = 8,
	CASCADED_SHADOW_TEXTURE_ARRAY_SLOT = 9,
	ENVMAP1_DP_SLOT = 10,
	ENVMAP2_DP_SLOT = 11,
	HIZ_TEXTURE_SLOT = 12,
	STATIC_ENVMAPS_SLOT_0 = 13,
	STATIC_ENVMAPS_SLOT_1 = 14,
	STATIC_ENVMAPS_SLOT_2 = 15,
	STATIC_ENVMAPS_SLOT_3 = 16,
	STATIC_ENVMAPS_SLOT_4 = 17,
	STATIC_ENVMAPS_SLOT_5 = 18,
	STATIC_ENVMAPS_SLOT_6 = 19,
	STATIC_ENVMAPS_SLOT_7 = 20,
	STATIC_ENVMAPS_SLOT_8 = 21,
};


struct FrameUniforms
{
	// DO NOT REORDER: SERIALIZED DATA!
	hfloat4 time_dt_pad2;
	hfloat4 global_light_dir;
	hfloat4 global_light_color;
	hfloat4 mb_velocity_min_max_sfactor_pad;
	hfloat4 ABCD;
	hfloat4 EFW_tau;
	hfloat4 fogCol;
	hfloat4 sky_color;
	hfloat4 ground_color;
	hfloat4 exposure_bloomthreshold_tone_map_white_pad;
	hfloat4 ambient_colors[54];
};

struct VertexUniforms
{
	hfloat4x4 view;
	hfloat4x4 vp;
	hfloat4x4 prev_vp;
	hfloat4x4 car_ao_matrix0;
	hfloat4x4 car_ao_matrix1;
	hfloat4x4 mvp;
	hfloat4x4 mvp2;
	hfloat4x4 mv;
	hfloat4x4 model;
	hfloat4x4 inv_model;
	hfloat4 cam_near_far_pid_vpscale;
};

struct TessellationUniforms
{
	hfloat4 tessellation_factor;
	hfloat4 frustum_planes[6];
	hfloat tessellation_multiplier;
};

struct FragmentUniforms
{
	hfloat4x4 cascaded_shadow_matrices[4];
	hfloat4x4 inv_view;
	hfloat4x4 dpcam_view;
	hfloat4 cascaded_frustum_distances;
	hfloat4 gamma_exp;
	hfloat4 carindex_translucency_ssaostr_fovscale;
	hfloat3 view_pos;
};

struct FilterUniforms
{
	hfloat4x4 inv_view;
	hfloat4x4 dpcam_view;
	hfloat4x4 cascaded_shadow_matrices[4];
	hfloat4 carindex_translucency_ssaostr_fovscale;
	hfloat4 cascaded_frustum_distances;
	hfloat4 depth_parameters;
	hfloat4 corners[4];
	hfloat3 view_pos;
	hfloat2 inv_resolution;
	hfloat2 offset_2d;
	hfloat camera_focus;
	hfloat camera_focus_inv;
	hfloat dof_strength;
	hfloat dof_strength_inv;
	hfloat gauss_lod_level;
	hfloat projection_scale;
};

struct SSAOUniforms
{
	hfloat4 depth_parameters;
	hfloat4 corners[4];
	hfloat projection_scale;
};

struct SSDSConstants
{
	hfloat4x4 cascaded_shadow_matrices[4];
	hfloat4 cascaded_frustum_distances;
	hfloat4 depth_parameters;
	hfloat4 corners[4];
	hfloat3 view_pos;
};

struct LuminanceBuffer
{
	//.x - adaptive luminance
	//.y - current frame average luminance
	//.zw - padding
	hfloat4 adaptive_avg_pad2;
};

struct InstanceData
{
	hfloat4x4 model;
	hfloat4x4 inv_model;
};

struct OcclusionConstants
{
	hfloat4x4 vp;
	hfloat2 view_port_size;
	uint id_count;
	uint instance_id_offset;
	uint frame_counter;
	hfloat near_far_ratio;
};


struct PPVertexInput
{
	hfloat2 in_pos [[attribute(0)]] ;
	hfloat2 in_uv  [[attribute(1)]] ;
};

#define VELOCITY_BUFFER_TYPE vec4

#define PI 3.141592654


// Scene bounding area is about 4000 meters, but shadow far is 220m, SSAO far is 500m
#define MAX_LINEAR_DEPTH 512.0 
#define MAX_BYTE 255.0

/*highp*/ _float2 EncodeLinearDepthToVec2(/*highp*/ _float v )
{
	v /= MAX_LINEAR_DEPTH;
	
	v *= MAX_BYTE;
	
	/*highp*/ _float2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}

/*highp*/ _float DecodeLinearDepthFromVec2(/*highp*/ _float2 v )
{
	/*highp*/ _float r255 = v.x * MAX_BYTE + v.y;
	return MAX_LINEAR_DEPTH * r255 / MAX_BYTE;
}

#define MAX_FLOAT 4.0
_float2 packFloatToVec2(_float v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) /2.0;
	
	v *= MAX_BYTE;
	
	_float2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}

hfloat2 packFloatToVec2Highp(hfloat v)
{
	v /= MAX_FLOAT;
	v = (v + 1.0) /2.0;
	
	v *= MAX_BYTE;
	
	hfloat2 res;
	res.x = floor(v) / MAX_BYTE;
	
	v = fract(v);
	
	res.y = v;
	
	return res;
}


_float4 pack2FloatToVec4(_float2 v)
{
	_float2 ex = packFloatToVec2(v.x);
	_float2 ey = packFloatToVec2(v.y);
	
	return _float4(ex,ey);
}

hfloat4 pack2FloatToVec4Highp(hfloat2 v)
{
	hfloat2 ex = packFloatToVec2Highp(v.x);
	hfloat2 ey = packFloatToVec2Highp(v.y);
	
	return hfloat4(ex,ey);
}


hfloat unpackFloatFromVec2Highp(hfloat2 v)
{
	hfloat r255 = v.x*MAX_BYTE+v.y;
	hfloat x = r255 / MAX_BYTE;
	return MAX_FLOAT * (  2.0*x-1.0  );
}


hfloat2 unpackVec2FFromVec4Highp(hfloat4 value)
{
	return hfloat2(unpackFloatFromVec2Highp(value.xy),unpackFloatFromVec2Highp(value.zw));
}


// The max velocity value(mb_velocity_min_max_sfactor_pad.y) will be never bigger than 0.05
#define MAX_VELOCITY 0.05
//#define MAX_VELOCITY mb_velocity_min_max_sfactor_pad.y
hfloat4 pack_velocity(hfloat2 velocity)
{
#if VELOCITY_BUFFER_RGBA8
	return pack2FloatToVec4(velocity);
#else
	return hfloat4(velocity / (2.0 * MAX_VELOCITY) + 0.5, 0.0, 0.0);
#endif
}

hfloat2 unpack_velocity(texture2d<_float> velocityMap, sampler tex_sampler, hfloat2 uv)
{
	_float4 v = velocityMap.sample(tex_sampler, hfloat2(uv));
#if VELOCITY_BUFFER_RGBA8
	return unpackVec2FFromVec4Highp(v);
#else
	return (hfloat2(v.xy) - 0.5) * (2.0 * MAX_VELOCITY);
#endif
}

_float2 velocityFunction(hfloat4 scPos, hfloat4 prevScPos, hfloat4 mb_velocity_min_max_sfactor_pad)
{
	const _float blur_strength = 0.5 ;

	hfloat2 a = scPos.xy / scPos.w ;
	hfloat2 b = prevScPos.xy / prevScPos.w ;

	hfloat2 qx = a-b;
	
	qx *= blur_strength ;
	
	#define EPS 0.0001
	hfloat qx_length = length(qx) ;

	if (qx_length < EPS)
	{
		qx = 0.0;
	}
	else
	{
		qx = ( qx * clamp( qx_length, mb_velocity_min_max_sfactor_pad.x, mb_velocity_min_max_sfactor_pad.y ) ) / ( qx_length + EPS) ;
	}
		
	_float2 res = _float2(qx);
	
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

#if (CASCADE_COUNT == 3)
int getCascadeIndex(_float fragment_depth, hfloat4 cascaded_frustum_distances)
{	
	hfloat3 distances_h = hfloat3(fragment_depth) - cascaded_frustum_distances.xyz;
    _float3 distances = _float3(distances_h * 100000.0);
    distances = clamp(distances, _float3(0.0), _float3(1.0));

	return int(dot(distances, _float3(1.0)));
}
#else
int getCascadeIndex(_float fragment_depth, hfloat4 cascaded_frustum_distances)
{	
	hfloat4 distances_h = hfloat4(fragment_depth) - cascaded_frustum_distances;
    _float4 distances = _float4(distances_h * 100000.0);
    distances = clamp(distances, _float4(0.0), _float4(1.0));

	return int(dot(distances, _float4(1.0)));
}
#endif

int getMapBasedCascadeIndex(hfloat4 world_pos, thread hfloat4 & shadow_coord, constant hfloat4x4 * cascaded_shadow_matrices)
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

_float shadowStatic(texture2d<hfloat> texture_unit7,
					sampler sampler7,
					hfloat4 world_pos)
{
    hfloat2 uv = world_pos.xz / 1950.0;
    uv += hfloat2(-0.5, 0.5);

    _float baked_shadow = texture_unit7.sample(sampler7, uv).y;
    //baked_ao = clamp( baked_ao + 0.0, 0.0, 1.0);    
    return baked_shadow;
}

_float shadowCascaded(texture2d<_float> texture_unit7,
					  sampler sampler7,
					  depth2d_array<hfloat> cascaded_shadow_texture_array,
					  sampler cascaded_shadow_sampler,
					  hfloat fragment_depth,
					  hfloat4 world_pos,
					  hfloat4 cascaded_frustum_distances,
					  constant hfloat4x4 * cascaded_shadow_matrices)
{    
    /*
    // Distance based selection
	const _float bias_values[4] = _float[4]
	(
        -0.0011,
        -0.0011,
        -0.0011,
		0.0001	
	);
    */
    // Map based selection
    const _float bias_values[4] =
	{
        0.000,
        0.000,
		0.0001,
		0.0001
	};

    hfloat4 shadow_coord;
    
    // Distance based selection
    //int index = getCascadeIndex(fragment_depth);
    // Project to light projection space
    //shadow_coord = cascaded_shadow_matrices[index] * world_pos;
    
    // Map bases selection
    int index = getMapBasedCascadeIndex(world_pos, shadow_coord, cascaded_shadow_matrices);
#if (CASCADE_COUNT == 3)    
    index = clamp(index, 0, 3);
    if (index > 2)
    {
       return shadowStatic(texture_unit7, sampler7, world_pos);
    }
#else    
    if (index > 3)
    {
        return 1.0;        
    }
#endif
		
    // Apply the bias and sample the shadow map
	hfloat z = shadow_coord.z - bias_values[index];
	shadow_coord.z = _float(index);
	shadow_coord.w = z;

	// [AAPL] Y-FLIP
	shadow_coord.y = 1.0 - shadow_coord.y;

	_float shadow_dist = 0.0;
	//PCF
	//shadow_dist = texture(cascaded_shadow_texture_array, shadow_coord.xyzw);
	
	//PCFx4
	hfloat4 gather = cascaded_shadow_texture_array.gather_compare(cascaded_shadow_sampler, shadow_coord.xy, shadow_coord.z, z);

	shadow_dist = _float(dot(gather, 0.25));
	
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
        _float border = cascaded_frustum_distances.z - 0.0001;
        _float attenuation = clamp((fragment_depth - border) / 0.0001, 0.0, 1.0);
        shadow_dist = mix(shadow_dist, shadowStatic(world_pos), attenuation);
    }
#else
	_float border = cascaded_frustum_distances.w - 0.00001;
	_float attenuation = clamp((fragment_depth - border) / 0.00001, 0.0, 1.0);
	shadow_dist = mix(shadow_dist, _float(1.0), attenuation);
#endif
	
	return shadow_dist;
}

///////////////////////////////////////////////////////////////////////////////
//		POST & SCREEN-SPACE EFFECTS
///////////////////////////////////////////////////////////////////////////////

hfloat rand(hfloat2 co)
{
	//screen-space
	return (fract(sin(dot(co.xy ,hfloat2(12.9898,78.233))) * 43758.5453));
}

hfloat4 encodeNormal(hfloat3 N)
{
#if NORMAL_BUFFER_RGBA8
	hfloat temp = 8.0*(N.z)+8.0;
	
	hfloat val = sqrt(abs(temp)); //abs: calling sqrt on <0 is undefined
	return pack2FloatToVec4Highp(N.xy / hfloat2(val) + 0.5);
#else
	return hfloat4(0.5 * N + 0.5, 1.0);
#endif
}

#if NORMAL_BUFFER_RGBA8
#define NORMAL_PREC lowp
#else
#define NORMAL_PREC mediump
#endif

hfloat3 decodeNormal(texture2d<_float> normalMap, sampler tex_sampler, hfloat2 uv)
{
	hfloat4 g_normal = hfloat4(normalMap.sample(tex_sampler, uv));

#if NORMAL_BUFFER_RGBA8 
	hfloat2 a = unpackVec2FFromVec4Highp(g_normal)*_float2(4.0)-_float2(2.0);
	hfloat b = dot(a,a);
	hfloat val = 1.0-b/4.0;
	hfloat c = sqrt(abs(val)); //abs: calling sqrt on <0 is undefined
	
	hfloat3 N;
	N.xy = a*_float2(c);
	N.z = 1.0-b/2.0;
	return N;
#else
	return 2.0 * g_normal.xyz - 1.0;
#endif
}

hfloat getLinearDepth(depth2d<hfloat> depthTex, sampler tex_sampler, hfloat2 uv, hfloat4 depth_parameters)
{
	hfloat d = depthTex.sample(tex_sampler, uv);
	d = depth_parameters.y / (d - depth_parameters.x);

	return d;
}

hfloat getLinearDepth(hfloat depth, hfloat4 depth_parameters)
{
	return depth_parameters.y / (depth - depth_parameters.x);
}

hfloat3 getPositionVS(hfloat linear_depth, hfloat2 uv, constant hfloat4 * corners)
{
	hfloat3 dir0;
	hfloat3 dir1;
	hfloat3 dir2;
	
#ifdef IS_PORTRAIT_MODE
	dir0 = mix( corners[1].xyz, corners[0].xyz, uv.y);
	dir1 = mix( corners[3].xyz, corners[2].xyz, uv.y);
	
	dir2 = mix( dir1, dir0, uv.x); // [AAPL] Y-FLIP
#else
	dir0 = mix( corners[0].xyz, corners[1].xyz, uv.x);
	dir1 = mix( corners[2].xyz, corners[3].xyz, uv.x);
	
	dir2 = mix( dir1, dir0, uv.y); // [AAPL] Y-FLIP
#endif
	
	return linear_depth * dir2;
}

hfloat3 getPositionVS(depth2d<hfloat> depthTex, sampler tex_sampler, hfloat2 uv, hfloat4 depth_parameters, constant hfloat4 * corners)
{
	hfloat d  = getLinearDepth(depthTex, tex_sampler, uv, depth_parameters);
	return getPositionVS(d,uv,corners);
}

hfloat3 getPositionWS(depth2d<hfloat> depthTex, sampler tex_sampler, hfloat2 uv, hfloat4 depth_parameters, constant hfloat4 * corners, hfloat3 view_pos)
{
	//NOTE: corners[4] are different between VS and WS case
	return getPositionVS(depthTex, tex_sampler, uv, depth_parameters, corners) + view_pos;
}

hfloat3 getPositionWS(hfloat linear_depth, hfloat2 uv, constant hfloat4 * corners, hfloat3 view_pos)
{
	//NOTE: corners[4] are different between VS and WS case
	return getPositionVS(linear_depth, uv, corners) + view_pos;
}

///////////////////////////////////////////////////////////////////////////////
//		SHADING
///////////////////////////////////////////////////////////////////////////////
_float getLum(_float3 inCol)
{
	return dot(_float3(0.212671, 0.71516, 0.072169), inCol);
}

_float3 RGBMtoRGB( _float4 rgbm ) 
{
	_float3 dec = 6.0 * rgbm.rgb * rgbm.a;
	return dec*dec; //from "gamma" to linear
}

_float3 RGBEtoRGB( _float4 rgbe) 
{ 
	_float e = (rgbe.a - 0.5) * 256.0;
	e = exp2( e);
	_float3 origVal = rgbe.rgb * _float3( e);
	//_float origLum = getLum(origVal);
	//_float boostedLum = origLum + powr(max(origLum - 3.0, 0.0), 3.0); //brightness bright pixels even more (10 -> 1000)
	//_float3 boostedVal =  origVal * boostedLum / origLum;
	return origVal;
}

_float3 decodeFromByteVec3(_float3 myVec)
{
#ifdef UBYTE_NORMAL_TANGENT
	return myVec = 2.0 * myVec - 1.0;
#endif
	return myVec;
}

_float3x3 calcTBN(hfloat3 normal, hfloat3 tangent)
{
	hfloat3 n = normalize( normal);
	hfloat3 t = normalize( tangent);
	_float3 b = _float3(normalize( cross( t, n)));
	return _float3x3( _float3(t), b, _float3(n));
}


_float3 calcWorldNormal(_float3 texNorm, hfloat3 normal, hfloat3 tangent)
{
	_float3 ts_normal = texNorm * 2.0 - 1.0;
	
	_float3x3 tbn = calcTBN(normal,tangent) ;

	return normalize( tbn * ts_normal);
}


///////////////////////////////////////////////////////////////////////////////
//		LIGHTING
///////////////////////////////////////////////////////////////////////////////

//Physically-based specular
_float getFresnel(_float HdotL, _float specular_color) //Schlick Fresnel
{
	_float base = 1.0 - HdotL;
	_float exponential = powr( base, _float(5.0) );
	_float fresnel_term = specular_color + ( 1.0 - specular_color ) * exponential;
	return fresnel_term;
}

_float getSpecular(_float3 N, _float3 L, _float3 V, _float NdotL_01, _float specular_color, _float roughness)
{
	_float specExp = powr (2.0, (1.0 - roughness) * 10.0 + 2.0);

	_float3 H = normalize(V + L);
	_float NdotH = clamp(dot(N, H),_float(0.0),_float(1.0));
	_float HdotL = dot(H, L); //can never go above 90, no need to clamp

	_float normalisation_term = ( specExp + 2.0 ) / 8.0;
	_float blinn_phong = powr( NdotH, specExp );
	
	_float specular_term = normalisation_term * blinn_phong;	
	_float cosine_term = NdotL_01;
	
	_float fresnel_term = getFresnel(HdotL, specular_color);

	_float specular = specular_term * cosine_term * fresnel_term; // visibility_term, but it is factor out as too costy
	return specular;
}


hfloat3 applyFog(_float3 origCol, hfloat dist, hfloat3  rayDir, hfloat4 global_light_dir, hfloat4 global_light_color, hfloat4 fogCol)
{
    hfloat fogAmount = exp(-(dist * dist) * 0.000001);
	hfloat fogTintFactor = powr( clamp(dot(-rayDir, global_light_dir.xyz),0.0,1.0),4.0);
	hfloat3 finalFogCol = mix(fogCol.xyz, global_light_color.xyz, fogTintFactor);
    return mix(finalFogCol, hfloat3(origCol), clamp(fogAmount, 0.0, 1.0) );
}

_float3 getHemiAmbient(_float3 N, hfloat4 ground_color, hfloat4 sky_color)
{
	return mix(_float3(ground_color.xyz), _float3(sky_color.xyz), clamp(N.y,_float(0.0),_float(1.0)));
}

_float3 getAmbientCubeColor(_float3 N, _float cubeIdx, hfloat4 ambient_colors[])
{
	int offset = int(cubeIdx) * 6;

	_float3 nSquared = N * N;
	int isNegativeX = N.x < 0.0 ? 1 : 0;
	int isNegativeY = N.y < 0.0 ? 1 : 0;
	int isNegativeZ = N.z < 0.0 ? 1 : 0;
	_float3 linearColor = 
        nSquared.x * _float3(ambient_colors[offset + isNegativeX].xyz) +
        nSquared.y * _float3(ambient_colors[offset + isNegativeY+2].xyz) +
        nSquared.z * _float3(ambient_colors[offset + isNegativeZ+4].xyz);
	return linearColor; 

	//return mix(ground_color.xyz, sky_color.xyz, clamp(N.y,0.0,1.0));
}

_float getEnvLodLevel(_float roughness, _float3 view_dir, _float3 N)
{
	_float view_length = length(view_dir);
		
	_float level = dot(view_dir / view_length, N);
	level = clamp(level, _float(0.0), _float(1.0));
	
	level = 1.0 - level;
	
	level = powr(level, _float(64.0));
		
	view_length = clamp(view_length * 0.001, 0.0, 1.0);
	
	level += roughness;
	level = mix(level, _float(1.0), _float(view_length));
	level *= 10.0; // max lod level
	
	return level;
}

//#define STATIC_ENVPROBE_CAPTURE

//NOTE: until more lights need to be combined with the IBL sun, uses IBL specular only
//		until convoluted diffuse envmaps are available (even a single one with sky), uses analytic diffuse/ambient
_float3 getPBRambient(_float3 N, _float cubeIdx, hfloat4 ground_color, hfloat4 sky_color)
{
	return getHemiAmbient(N, ground_color, sky_color);
}

//simple version for realtime cubemap render pass
_float3 getPBRambient(_float3 N, hfloat4 ground_color, hfloat4 sky_color)
{
	return getHemiAmbient(N, ground_color, sky_color);
}

_float3 getPBRdiffuse(texture2d<_float> texture_unit7, sampler sampler7, _float3 N, _float3 L, _float3 V, _float two_sided_lighting, _float shadow, hfloat3 world_pos, hfloat4 global_light_color)
{
	//NOTE: diffuse is almost zero for pure metals
	_float translucent_addition = 0.0;
	if(two_sided_lighting > 0.5)
	{
		hfloat2 uv = world_pos.xz / 1950.0;
		uv += hfloat2(-0.5, 0.5);
		
		_float baked_shadow = texture_unit7.sample(sampler7, uv).y;
	
		translucent_addition = (two_sided_lighting - 0.5) * 2.0 * (clamp(dot(-N,-V),_float(0.0),_float(1.0))) * (baked_shadow + 1.0) * 0.5;
	}
	else if(two_sided_lighting > 0.0)//billboard
	{
		translucent_addition = two_sided_lighting * 2.0 * (1.0 - clamp(dot(N, L),_float(0.0),_float(1.0))); //brightens if facing against the sun
	}
	
	return _float3(global_light_color.xyz) * 2.0 * (clamp(dot(N, L),_float(0.0),_float(1.0)) * shadow + translucent_addition) / PI;
}

//reflections are stronger when surface is looked upon in a steep angle
_float FresnelSchlickWithRoughness(_float specCol, _float3 V, _float3 N, _float roughness)
{
	return specCol * (1.0 + 1.0 * (max(roughness, specCol) - specCol) * powr(1.0 - clamp(dot(V, N), _float(0.0), _float(1.0)), 5.0));
}

_float4 getParaboloid(texture2d_array<_float> texture, hfloat3 R, hfloat envLod, hfloat4x4 dpcam_view)
{
	constexpr sampler tex_sampler(coord::normalized, filter::linear, mip_filter::linear, address::clamp_to_edge);

	//transform to DP-map space (z = fwd, y = up)
	
	//_float3 normalWS = normalize(inv_view * _float4(normalVS,0.0)).xyz;
	
	hfloat3 Rdp = normalize(dpcam_view * hfloat4(R, 0.0)).xyz;

	if(Rdp.z>0.0)
	{
		hfloat2 frontUV = (Rdp.xy / (2.0*(1.0 + Rdp.z))) + 0.5;
		frontUV.y = 1.0 - frontUV.y; // [AAPL] Y-FLIP
		return texture.sample(tex_sampler, frontUV, 0.0, level(envLod));
	}
	else
	{
		hfloat2 backUV = (Rdp.xy / (2.0*(1.0 - Rdp.z))) + 0.5;
		backUV.y = 1.0 - backUV.y; // [AAPL] Y-FLIP
		return texture.sample(tex_sampler, backUV, 1.0, level(envLod));
	}	

}

_float3 PBRhelper(texture2d_array<_float> envmap1_dp,
				  texture2d_array<_float> envmap2_dp,
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
	#ifdef PLATFORM_OSX
				  texturecube_array<_float> static_envmaps,
	#else
				  array<texturecube<_float>, NUM_STATIC_ENVMAPS> static_envmaps,
	#endif
                  sampler static_envmaps_sampler,
#endif
				  _float roughness, _float specular_color, _float cube_index, _float3 view_dir, hfloat3 N, hfloat3 V, _float3 diffuse,
				  _float3 ambient, _float shadow, _float ssao, _float3 albedo, _float3 emissive, hfloat4 ground_color, hfloat4 sky_color,
				  hfloat4 global_light_dir, hfloat4x4 dpcam_view)
{
	//NOTE: specular is almost zero for non-metals
	
	hfloat3 R = reflect(-V, N);	
	_float3 result = _float3(0.0);
	_float3 specular = _float3(0.0);

	roughness = clamp(roughness, _float(0.08), _float(1.0));

	_float envLod = getEnvLodLevel(roughness, view_dir, _float3(N));
	_float mipIntensityCorrection = (1.0 + clamp(envLod, _float(0.0), _float(10.0)) / 10.0 * 1.0);
	_float viewDepBrighten = FresnelSchlickWithRoughness(specular_color, _float3(V), _float3(N), roughness);
	
	_float specStrength = specular_color;
	_float shadow2 = mix(shadow, _float(1.0), powr(specStrength, _float(0.5)));
	
	_float cube_expanded = cube_index;
	if(cube_expanded < 100.0)
	{
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
        _float index = clamp(floor(cube_expanded + _float(0.5f)), _float(0.0f), (_float)(NUM_STATIC_ENVMAPS - 1));
	#ifdef PLATFORM_OSX
		_float4 reflect_color = static_envmaps.sample(static_envmaps_sampler, R, index, level(envLod));
	#else
        _float4 reflect_color = static_envmaps[(int)index].sample(static_envmaps_sampler, R, level(envLod));
	#endif
		specular = RGBMtoRGB(reflect_color).xyz;
#else
		// EXT_texture_cube_map_array is not supported
		specular = _float3(0.5);
#endif
		
/*		if(abs(cube_expanded - 7.0) < 0.4) //HACK: 7th location is the garage, sample dyn cube of car to fake light coming from the opening door
		{
			specular = textureLod(envmap2, R, envLod).xyz;
		}
*/		
#ifdef STATIC_ENVPROBE_CAPTURE
		specular = _float3(0.0);
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
	#ifdef PLATFORM_OSX
		_float4 reflect_color = static_envmaps.sample(static_envmaps_sampler, R, 0.0); //Sky cube map
	#else
		_float4 reflect_color = static_envmaps[0].sample(static_envmaps_sampler, R); //Sky cube map
	#endif
		specular = RGBMtoRGB(reflect_color).xyz;
#else
		// EXT_texture_cube_map_array is not supported
		specular = _float3(0.5);
#endif
		
#ifdef STATIC_ENVPROBE_CAPTURE
		specular = _float3(0.0);
#endif	
		//specular *= mipIntensityCorrection; //glGenerateMips darkens the image
		specular *= viewDepBrighten;		
		
		result = specular;// * shadow2 * ssao;
		result += (diffuse + emissive + ambient) * albedo;
	}
	else //dynamic cubemaps
	{	
		_float reflFix = 1.0 - powr(1.0 - clamp(dot(V, N), 0.0, 1.0), 50.0); //HACK to correct car mesh / normal issues
	
		_float isPaint = 0.0;
		if(cube_expanded < 253.5) //car 0, 252 or 253
		{			
			isPaint = cube_expanded > 252.5 ? 1.0 : 0.0;

			_float3 envmap1_color = RGBMtoRGB_cubemap( getParaboloid(envmap1_dp, R, envLod, dpcam_view) ) ;
			specular = envmap1_color * mipIntensityCorrection; //NOTE: clamp is here to reduce reflection _floattor noise at edges, and white spots from envmap reads
#ifdef STATIC_ENVPROBE_CAPTURE
			specular = _float3(0.0);
#endif			
			
			specular *= viewDepBrighten;
			//specular *= ssao;
		}
		else //car 1, 254 or 255
		{
			isPaint = cube_expanded > 254.5 ? 1.0 : 0.0;

			_float3 envmap2_color = RGBMtoRGB_cubemap(  getParaboloid(envmap2_dp, R, envLod, dpcam_view) ) ;
			specular = envmap2_color * mipIntensityCorrection;
#ifdef STATIC_ENVPROBE_CAPTURE
			specular = _float3(0.0);
#endif						
			specular *= viewDepBrighten;
			//specular *= ssao;
		}
		
		specular *=  reflFix;
		
		//decide if we are shading car paint - based on G-Buffer encoded data
		if(isPaint > 0.5)
		{
			_float  fresnel1 = clamp( _float(dot( N, V )),_float(0.0),_float(1.0));
			_float  fresnel2 = clamp( _float(dot( N, V )),_float(0.0),_float(1.0));
		   

			_float dist = 1.0 - clamp(length(view_dir)/_float(4.0), _float(0.0),_float(1.0));
			_float flakeDim = 0.1 * mix(_float(0.5), _float(1.0), dist);
			_float  fresnel1Sq = mix(fresnel1 * fresnel1, _float(0.0), clamp(_float(dot(V, global_light_dir.xyz)),_float(0.0),_float(1.0)));

			albedo = 		/*(fresnel1) * */albedo /*color*/ + 
								fresnel1Sq * albedo * _float3( 1.15) +
								fresnel1Sq * fresnel1Sq * albedo * _float3( 1.17);// +
								//powr( fresnel2, 16.0 ) * flakeLayerColor * flakeDim;		
		
		
			_float envContrib = powr(clamp(_float(dot(N, V)),_float(0.0),_float(1.0)),_float(0.1));
			result = mix(specular * 1.0, (diffuse + emissive + ambient) * albedo, /*0.5 + 0.5 * */envContrib);
		}
		else
		{
			result = (diffuse + emissive + ambient) * albedo + specular * powr(ssao, _float(2.0));
		}
	}
			
	//TODO IBL based diffuse, analytic specular for extra lights (computed via MicrofacetBRDF)
	
	// Analytical specular
	//specular += MicrofacetBRDF(roughness, specular_color, N, L_extra, V);
	
	return result;
}

_float3 getPBRlighting(texture2d_array<_float> envmap1_dp,
					   texture2d_array<_float> envmap2_dp,
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
	#ifdef PLATFORM_OSX
					   texturecube_array<_float> static_envmaps,
	#else
					   array<texturecube<_float>, NUM_STATIC_ENVMAPS> static_envmaps,
	#endif
                       sampler static_envmaps_sampler,
#endif
					   texture2d<_float> texture_unit7,
					   sampler sampler7,
					   hfloat3 view_dir, hfloat3 N, _float roughness, _float cube_index, _float specular_color, _float3 albedo,
					   _float emissive_translucency, _float bakedao, _float ssao, _float shadow, hfloat3 originWS, hfloat4 ground_color,
					   hfloat4 sky_color, hfloat4 global_light_dir, hfloat4 global_light_color, hfloat4x4 dpcam_view)
{
	hfloat3 V = normalize(view_dir);
	_float3 L = _float3(global_light_dir.xyz);

	_float emissive = 0.0;
	_float two_sided_lighting = 0.0;
	if(cube_index > 49.5 && cube_index < 99.5)
	{
		//add two-sided lighting
		two_sided_lighting = emissive_translucency;
	}
	else
	{
		emissive = powr(emissive_translucency * emissive_translucency, _float(3.0)) * 20.0; //first decompress from G-buffer encode, then make emissive source data nonlinear, then give range for bloom
	}
	
	_float3 diffuse = getPBRdiffuse(texture_unit7, sampler7, _float3(N), L, _float3(V), two_sided_lighting, shadow, originWS, global_light_color);
	_float3 ambient = getPBRambient(_float3(N), cube_index, ground_color, sky_color);
	
	//_float3 result_color = PBRhelper(roughness, specular_color, cube_index, view_dir, N, V, albedo * diffuse * shadow, _float3(emissive) + albedo * ambient * ao, shadow, ao);
	_float ao = min(bakedao, ssao);
	_float3 result_color = PBRhelper(envmap1_dp,
									 envmap2_dp,
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
									 static_envmaps,
                                     static_envmaps_sampler,
#endif
									 roughness, specular_color, cube_index, _float3(view_dir), N, V, diffuse, ambient * ao, shadow, ssao, albedo,
									 _float3(emissive), ground_color, sky_color, global_light_dir, dpcam_view); //ssao dims reflections

	//NOTE: 1. metals: 		the color is largely given by specular, shadow does not affect it
	//		2. non-metals:	specular is affected by diffuse
	
	return result_color;
}
