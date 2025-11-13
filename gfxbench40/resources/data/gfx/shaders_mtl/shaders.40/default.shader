/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//DEFAULT SHADER FOR OPAQUE MESHES
//	- can use tessellation
//	- can use local 8-bit displacement maps
//	- can use bezier tessellation
//	- renders opaques meshes to G-BUFFER
//		- but can render transparent meshes directly to LIGHT COMBINE target
//	- shall not write to dynamic cube target at runtime

#ifdef USE_TESSELLATION
#ifdef HAS_LOCAL_DISPLACEMENT

#elif defined(HAS_ABS_DISPLACEMENT)
#else
#error Expected HAS_LOCAL_DISPLACEMENT or HAS_ABS_DISPLACEMENT
#endif
#endif

#ifdef HAS_ABS_DISPLACEMENT
#define HAS_ABS_DISPLACEMENT 1
#else
#define NO_ABS_DISPLACEMENT 1
#endif


#include "common.h"
#include "occlusion_cull_common.h"
#include "tessellation_common.h"

// texture_unit0 - albedo, opacity or displacement in alpha (UV0) - oldname: color
// texture_unit1 - optional ao map (used for tunnel) - oldname: lightmap
// texture_unit2 - specular color (grayscale), emissive in alpha (UV0) - oldname: mask
// texture_unit3 - normalmap, smoothness in alpha (UV0) - oldname: normal
// texture_unit4 - optional - abs. pos map
// texture_unit5 - optional - abs. norm map
// texture_unit6 - optional -  flake normal perturb map

#ifdef USE_STAGE_IN_CONTROL_POINTS
struct ControlPoint
{
	hfloat3 _in_position	[[attribute(0)]];
	hfloat3 _in_normal		[[attribute(1)]];
	hfloat3 _in_tangent		[[attribute(2)]];
	hfloat2 _in_texcoord0	[[attribute(3)]];
	hfloat2 _in_texcoord1	[[attribute(4)]];

	hfloat3 in_position() { return _in_position; }
	hfloat3 in_normal() { return hfloat3(decodeFromByteVec3(_float3(_in_normal))); }
	hfloat3 in_tangent() { return hfloat3(decodeFromByteVec3(_float3(_in_tangent))); }
	hfloat4 in_texcoord() { return hfloat4(_in_texcoord0, _in_texcoord1); }
};
#else
struct ControlPoint
{
	packed_float3 _in_position;
	packed_uchar4 _in_normal;
	packed_uchar4 _in_tangent;
	packed_float4 _in_tesscoord;

	hfloat3 in_position()
	{
		return hfloat3(_in_position);
	}

	hfloat3 in_normal()
	{
		return (hfloat3(uchar4(_in_normal).xyz) / 255.0f) * 2.0f - 1.0f; 
	}
	hfloat3 in_tangent()
	{
		return (hfloat3(uchar4(_in_tangent).xyz) / 255.0f) * 2.0f - 1.0f;
	}
	hfloat4 in_texcoord()
	{
		return hfloat4(_in_tesscoord);
	}
};
#endif

struct MeshInput
{
	hfloat3 in_position		[[attribute(0)]];
	hfloat3 in_normal		[[attribute(1)]];
	hfloat3 in_tangent		[[attribute(2)]];
	hfloat2 in_texcoord0	[[attribute(3)]];
	hfloat2 in_texcoord1	[[attribute(4)]];
};

struct TessellationInput
{
#ifdef HAS_ABS_DISPLACEMENT
	hfloat2 tesscoord [[position_in_patch]];
#else
	#ifdef USE_STAGE_IN_CONTROL_POINTS
		patch_control_point<ControlPoint> controlPoints;
	#endif
	hfloat3 tesscoord [[position_in_patch]];
#endif
};

struct VertexOutput
{
	hfloat4 out_position [[position]];
	hfloat2 out_texcoord0;
	hfloat2 out_texcoord1;
	hfloat3 out_normal;
	hfloat3 out_tangent;
	hfloat3 out_worldpos;

#ifdef VELOCITY_BUFFER
	hfloat4 out_scPos;
    hfloat4 out_prevScPos;
#endif

#ifdef HAS_CAR_AO
	hfloat4 out_car_ao_texcoord0;
	hfloat4 out_car_ao_texcoord1;
#endif
};

struct FragmentOutput
{
	_float4 frag_color	[[color(0)]];
#ifdef VELOCITY_BUFFER
	_float4 velocity	[[color(1)]];
#endif
#ifdef G_BUFFER_PASS
	_float4 frag_normal	[[color(2)]];
	_float4 frag_params	[[color(3)]];
#endif
};


#ifdef USE_TESSELLATION
	using VertexInput = TessellationInput;
#else
	using VertexInput = MeshInput;
#endif


#ifdef TYPE_vertex

/***********************************************************************************/
//							VERTEX SHADER
/***********************************************************************************/

#ifdef USE_TESSELLATION
#ifdef HAS_ABS_DISPLACEMENT
	[[patch(quad, 0)]]
#else
#ifdef USE_STAGE_IN_CONTROL_POINTS
	[[patch(triangle, 3)]]
#else
	[[patch(triangle, 0)]]
#endif
#endif
#endif
vertex VertexOutput shader_main(VertexInput input [[stage_in]]
								,constant VertexUniforms * uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]]
#ifdef INSTANCING
								,device InstanceData * instance_data [[buffer(INSTANCE_UNIFORMS_SLOT)]]
								,uint i_id [[ instance_id ]]
#endif
#ifdef USE_TESSELLATION
								,uint base_instance [[base_instance]]
	#ifdef HAS_LOCAL_DISPLACEMENT
		#if !defined(USE_STAGE_IN_CONTROL_POINTS)
								,device ushort* indexInput [[buffer(INDEX_BUFFER_SLOT)]]
								,device ControlPoint* controlPointInput [[buffer(VERTEX_BUFFER_SLOT)]]
		#endif
								,const constant TessellationUniforms* tessUniforms [[buffer(TESSELLATION_UNIFORMS_SLOT)]]
								,texture2d<hfloat> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
								,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]]
	#else
								,device UserPerPatchData* userPatch [[buffer(USER_PER_PATCH_SLOT)]]
	#endif
#endif
#ifdef USE_TESSELLATION
								,uint patch_id [[ patch_id ]]
#else
								,uint v_id [[vertex_id]]
#endif
								)
{
	VertexOutput output;

#ifdef USE_TESSELLATION
#ifdef INSTANCING
	const auto real_instance_id = i_id;
#else
	const auto real_instance_id = 0;
#endif
	const auto real_patch_id = patch_id;

	auto gl_TessCoord = input.tesscoord;

	#ifdef HAS_ABS_DISPLACEMENT
        UserPerPatchData bp = userPatch[real_patch_id];

		_float3 position;
		_float3 tangent;
		_float3 bitangent;

		_float u = gl_TessCoord.x;
		_float v = gl_TessCoord.y;

		_float4 U = _float4( u*u*u, u*u, u, 1.0);
		_float4 V = _float4( v*v*v, v*v, v, 1.0);
		_float4 dU = _float4( 3.0 * u * u, 2.0 * u, 1.0, 0.0);
		_float4 dV = _float4( 3.0 * v * v, 2.0 * v, 1.0, 0.0);

		position.x = dot( U * bp.Px, V);
		position.y = dot( U * bp.Py, V);
		position.z = dot( U * bp.Pz, V);
		
		tangent.x = dot( dU * bp.Px, V);
		tangent.y = dot( dU * bp.Py, V);
		tangent.z = dot( dU * bp.Pz, V);
		
		bitangent.x = dot( U * bp.Px, dV );
		bitangent.y = dot( U * bp.Py, dV );
		bitangent.z = dot( U * bp.Pz, dV );
		
		_float3 p2 = position;
		output.out_normal = normalize( cross( tangent, bitangent));

		output.out_tangent = tangent;
		output.out_texcoord0 = _float2( 0.0);
		output.out_texcoord1 = _float2( 0.0);
	#else
		_float u = gl_TessCoord.x;
		_float v = gl_TessCoord.y;
		_float w = gl_TessCoord.z;

		auto patchId = real_patch_id;

#ifdef USE_STAGE_IN_CONTROL_POINTS
		auto localVertex = input.controlPoints[0];
#else
		auto localVertex = controlPointInput[indexInput[patchId * 3 + 0]];
#endif

		_float3 p2 = localVertex.in_position() * u;
		output.out_normal = localVertex.in_normal() * u;
		output.out_tangent = localVertex.in_tangent() * u;
		output.out_texcoord0 = localVertex.in_texcoord().xy * u;
		output.out_texcoord1 = localVertex.in_texcoord().zw * u;

#ifdef USE_STAGE_IN_CONTROL_POINTS
		localVertex = input.controlPoints[1];
#else
		localVertex = controlPointInput[indexInput[patchId * 3 + 1]];
#endif

		p2 += localVertex.in_position() * v;
		output.out_normal += localVertex.in_normal() * v;
		output.out_tangent += localVertex.in_tangent() * v;
		output.out_texcoord0 += localVertex.in_texcoord().xy * v;
		output.out_texcoord1 += localVertex.in_texcoord().zw * v;

#ifdef USE_STAGE_IN_CONTROL_POINTS
		localVertex = input.controlPoints[2];
#else
		localVertex = controlPointInput[indexInput[patchId * 3 + 2]];
#endif

		p2 += localVertex.in_position() * w;
		output.out_normal += localVertex.in_normal() * w;
		output.out_tangent += localVertex.in_tangent() * w;
		output.out_texcoord0 += localVertex.in_texcoord().xy * w;
		output.out_texcoord1 += localVertex.in_texcoord().zw * w;

		hfloat t0_w = texture_unit0.sample(sampler0, output.out_texcoord0).w;
		_float d = t0_w - 0.5 + tessUniforms->tessellation_factor.z;//tessUniforms->tessellation_factor: X factor, Y scale, Z bias
		p2 += output.out_normal * d * tessUniforms->tessellation_factor.y * clamp(tessUniforms->tessellation_factor.x - 1.0, 0.0, 1.0); //tessellation factor is >1 is active, else 1, can be ~64*/
		

	#endif

#ifdef INSTANCING 
		_float4 tmp = _float4( output.out_normal, 0.0) * instance_data[real_instance_id].inv_model;
		output.out_normal = tmp.xyz;

		tmp = _float4( output.out_tangent, 0.0) * instance_data[real_instance_id].inv_model;
		output.out_tangent = tmp.xyz;
#else
		_float4 tmp = _float4( output.out_normal, 0.0) * uniforms->inv_model;
		output.out_normal = tmp.xyz;

		tmp = _float4( output.out_tangent, 0.0) * uniforms->inv_model;
		output.out_tangent = tmp.xyz;
#endif

#ifdef INSTANCING
		_float4x4 _mvp = uniforms->vp * instance_data[real_instance_id].model;
		_float4 _scPos = _mvp * _float4( p2, 1.0);

		_float4 world_position = instance_data[real_instance_id].model * _float4( p2, 1.0);

		output.out_worldpos = world_position.xyz;
		output.out_position = _scPos;
#else
		_float4 _scPos = uniforms->mvp * _float4( p2, 1.0);

		_float4 world_position = uniforms->model * _float4( p2, 1.0);

		output.out_worldpos = world_position.xyz;
		output.out_position = _scPos;
#endif

#ifdef VELOCITY_BUFFER
	output.out_scPos = _scPos;
	#ifdef INSTANCING
		_float4x4 instance_prev_mvp = uniforms->prev_vp * instance_data[real_instance_id].model; 
		output.out_prevScPos = instance_prev_mvp * _float4(p2, 1.0); 
	#else
		output.out_prevScPos = uniforms->mvp2 * _float4(p2, 1.0); 
	#endif
#endif

#ifdef HAS_CAR_AO
	output.out_car_ao_texcoord0 = uniforms->car_ao_matrix0 * world_position;
	output.out_car_ao_texcoord1 = uniforms->car_ao_matrix1 * world_position;
#endif


#else
	_float3 normal = decodeFromByteVec3(input.in_normal);
	_float3 tangent = decodeFromByteVec3(input.in_tangent);

	#ifdef FAKE_NORMALS
		normal = normalize(input.in_position);
		tangent = normalize(cross(normal, _float3(0.0, 1.0, 0.0)));
	#endif

	_float3 obj_pos = input.in_position;
	#ifdef IS_BILLBOARD
		_float3 cam_right = _float3(uniforms->view[0][0], uniforms->view[1][0], uniforms->view[2][0]);
		_float3 cam_up =    _float3(uniforms->view[0][1], uniforms->view[1][1], uniforms->view[2][1]);
		_float3 cam_fwd =   _float3(uniforms->view[0][2], uniforms->view[1][2], uniforms->view[2][2]);
		_float size = length(input.in_position);
		
		_float3 BL = (-cam_up) + (-cam_right);
		_float3 TL = (cam_up) + (-cam_right);
		_float3 BR = (-cam_up) + (cam_right);
		_float3 TR = (cam_up) + (cam_right);
		if(v_id == 0)
		{
			obj_pos = BL;
		}
		else if(v_id == 1)
		{
			obj_pos = BR;			
		}
		else if(v_id == 2)
		{
			obj_pos = TR;
		}
		else
		{
			obj_pos = TL;
		}
		obj_pos = normalize(obj_pos);
		_float3 rotated_pos = obj_pos;
		obj_pos *= size;
	#endif

	#ifdef INSTANCING
		_float4x4 instance_mvp = uniforms->vp * instance_data[i_id].model;
		_float4 _scPos = instance_mvp * _float4( obj_pos, 1.0);
	
		_float4 world_position = instance_data[i_id].model * _float4( obj_pos, 1.0);
	#else
		_float4 _scPos = uniforms->mvp * _float4( obj_pos, 1.0);

		_float4 world_position = uniforms->model * _float4( obj_pos, 1.0);
	#endif

	output.out_position = _scPos;
	output.out_worldpos = world_position.xyz;

	#ifdef VELOCITY_BUFFER
		output.out_scPos = _scPos;

		#ifdef INSTANCING
			_float4x4 prev_instance_mvp = uniforms->prev_vp * instance_data[i_id].model;
			output.out_prevScPos = prev_instance_mvp * _float4(obj_pos, 1.0);
		#else
			output.out_prevScPos = uniforms->mvp2 * _float4(obj_pos, 1.0);
		#endif
	#endif

	#ifdef HAS_CAR_AO
		output.out_car_ao_texcoord0 = uniforms->car_ao_matrix0 * world_position;
		output.out_car_ao_texcoord1 = uniforms->car_ao_matrix1 * world_position;
	#endif

	_float4 tmp;
	#ifdef INSTANCING
		#ifdef IS_BILLBOARD //only instanced billboard rendering is supported
			output.out_normal = normalize(rotated_pos * 1.0 + cam_fwd);
			output.out_tangent = normalize(rotated_pos * 1.0 + cam_right);
		#else
			tmp = _float4(normal, 0.0) * instance_data[i_id].inv_model;
			output.out_normal = tmp.xyz;

			tmp = _float4(tangent, 0.0) * instance_data[i_id].inv_model;
			output.out_tangent = tmp.xyz;
		#endif
	#else
		tmp = _float4(normal, 0.0) * uniforms->inv_model;
		output.out_normal = tmp.xyz;

		tmp = _float4(tangent, 0.0) * uniforms->inv_model;
		output.out_tangent = tmp.xyz;
	#endif

	output.out_texcoord0 = input.in_texcoord0;
	output.out_texcoord1 = input.in_texcoord1;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	#ifndef USE_TESSELLATION
		out_instance_id = instance_id;
	#endif
#endif

	return output;
}

#endif


#ifdef TYPE_fragment

/***********************************************************************************/
//							FRAGMENT SHADER
/***********************************************************************************/

fragment FragmentOutput shader_main(VertexOutput input [[stage_in]]
									,constant FrameUniforms * frame_consts [[buffer(FRAME_UNIFORMS_SLOT)]]
									,constant FragmentUniforms * frag_consts [[buffer(FRAGMENT_UNIFORMS_SLOT)]]
#ifdef IS_TWO_SIDED
									,bool front_facing [[front_facing]]
#endif
									,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
									,texture2d<_float> texture_unit1 [[texture(TEXTURE_UNIT1_SLOT)]]
									,texture2d<_float> texture_unit2 [[texture(TEXTURE_UNIT2_SLOT)]]
									,texture2d<_float> texture_unit3 [[texture(TEXTURE_UNIT3_SLOT)]]
									,texture2d<hfloat> texture_unit4 [[texture(TEXTURE_UNIT4_SLOT)]]
									,texture2d<_float> texture_unit7 [[texture(TEXTURE_UNIT7_SLOT)]]
									,texture2d_array<_float> envmap1_dp [[texture(ENVMAP1_DP_SLOT)]]
									,texture2d_array<_float> envmap2_dp [[texture(ENVMAP2_DP_SLOT)]]
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
	#ifdef PLATFORM_OSX
									,texturecube_array<_float> static_envmaps [[texture(STATIC_ENVMAPS_SLOT_0)]]
	#else
									,array<texturecube<_float>, NUM_STATIC_ENVMAPS> static_envmaps [[texture(STATIC_ENVMAPS_SLOT_0)]]
	#endif
#endif
									,depth2d_array<hfloat> cascaded_shadow_unit [[texture(CASCADED_SHADOW_TEXTURE_ARRAY_SLOT)]]
									,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]]
									,sampler sampler1 [[sampler(TEXTURE_UNIT1_SLOT)]]
									,sampler sampler2 [[sampler(TEXTURE_UNIT2_SLOT)]]
									,sampler sampler3 [[sampler(TEXTURE_UNIT3_SLOT)]]
									,sampler sampler4 [[sampler(TEXTURE_UNIT4_SLOT)]]
									,sampler sampler7 [[sampler(TEXTURE_UNIT7_SLOT)]]
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
                                    ,sampler static_envmaps_sampler [[sampler(STATIC_ENVMAPS_SLOT_0)]]
#endif
									,sampler cascaded_shadow_sampler [[sampler(CASCADED_SHADOW_TEXTURE_ARRAY_SLOT)]])
{
	FragmentOutput output;

#ifdef HAS_CAR_AO
    hfloat4 out_car_ao_texcoord0 = input.out_car_ao_texcoord0;
    hfloat4 out_car_ao_texcoord1 = input.out_car_ao_texcoord1;
#endif

#ifdef VELOCITY_BUFFER
    hfloat4 out_scPos = input.out_scPos;
    hfloat4 out_prevScPos = input.out_prevScPos;
#endif

#ifdef HAS_CAR_AO //TODO: replace this with deferred decals
	_float car_ao0 = 1.0;
	_float car_ao1 = 1.0;
#ifndef STATIC_ENVPROBE_CAPTURE
	car_ao0 = texture_unit4.sample(sampler4, out_car_ao_texcoord0.xy/out_car_ao_texcoord0.w).x;
	if( out_car_ao_texcoord0.x < 0.0 || out_car_ao_texcoord0.x > 1.0 || out_car_ao_texcoord0.y < 0.0 || out_car_ao_texcoord0.y > 1.0 || out_car_ao_texcoord0.z < 0.0 || out_car_ao_texcoord0.z > 1.0) //hack not to project ao onto overhangs
		car_ao0 = 1.0;

	car_ao1 = texture_unit4.sample(sampler4, out_car_ao_texcoord1.xy/out_car_ao_texcoord1.w).x;
	if( out_car_ao_texcoord1.x < 0.0 || out_car_ao_texcoord1.x > 1.0 || out_car_ao_texcoord1.y < 0.0 || out_car_ao_texcoord1.y > 1.0 || out_car_ao_texcoord1.z < 0.0 || out_car_ao_texcoord1.z > 1.0) //hack not to project ao onto overhangs
		car_ao1 = 1.0;
#endif
#endif

	//NOTE: no sRGB format for alpha channels
	_float4 albedo_extras = texture_unit0.sample(sampler0, input.out_texcoord0); //+ displacement or opacity
	
	//NOTE: on desktop, DXT can produce artifacts with dark textures unless it is compressed with gamma	
	//(relates to "greening" DXT compression artifacts)	
#if defined PLATFORM_OSX && 0 // [AAPL] - Not sure this is required
	albedo_extras = powr(albedo_extras, frag_consts->gamma_exp);
#endif
	
	_float3 albedo = albedo_extras.xyz;
	
#ifdef DRAW_GS_WIREFRAME	
	_float nearD = min(min(dist[0],dist[1]),dist[2]);
	_float edgeIntensity = exp2(-1.0*nearD*nearD);	
	albedo = mix(_float3(0.5), _float3(0.0), edgeIntensity);
#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
	//albedo.gb *= sat(g_patch_size);
#endif	
#endif
#endif	
	
#ifdef ALPHA_TEST
	if(albedo_extras.w < 0.5)
	{
#ifndef DRAW_GS_WIREFRAME
		discard_fragment();
#endif
	}
#endif	

	//NOTE: texture_unit1 used to store baked shadow, ao, emissive
	_float baked_ao = 1.0; //texture( texture_unit1, out_texcoord1).x;
#if defined(HAS_CAR_PAINT) || defined(USE_WORLD_AO)
	hfloat2 uv = input.out_worldpos.xz / 1950.0;
	uv += hfloat2(-0.5, 0.5);
	baked_ao = texture_unit7.sample(sampler7, uv).x;
#else
	baked_ao = texture_unit1.sample(sampler1, input.out_texcoord1).x;//tunnel
#endif
	
	_float4 specCol_emissive = texture_unit2.sample(sampler2, input.out_texcoord0);
	_float4 texNorm_smoothness = texture_unit3.sample(sampler3, input.out_texcoord0);

#ifdef VELOCITY_BUFFER
	output.velocity = _float4(pack_velocity(hfloat2(velocityFunction(out_scPos, out_prevScPos,frame_consts->mb_velocity_min_max_sfactor_pad))));
#endif	
	
	_float specCol = specCol_emissive.y; //TODO specular color can be compressed as greyscale
	_float local_emissive = 0.0;
#ifdef HAS_LOCAL_EMISSIVE
	local_emissive = sqrt(specCol_emissive.w); //so source data has more precision in the dark when passing through the G-buffer
#endif
	
	_float smoothness = texNorm_smoothness.w; //smoothness is assumed linear, not sRGB!
	_float3 texNorm = _float3(0.0);
	hfloat3 N = 0.0;
#ifdef HAS_CAR_PAINT
	N = normalize(input.out_normal);
#else
	texNorm = texNorm_smoothness.xyz;
	N = hfloat3(calcWorldNormal(texNorm, input.out_normal, input.out_tangent));
#endif
	
	hfloat3 NV = normalize(hfloat4(N,0) * frag_consts->inv_view).xyz; //view-space normals
	
#ifdef IS_TWO_SIDED
	if(front_facing == false)
	{
		NV *= -1.0;
	}
#endif
	
	hfloat3 view_dir = frag_consts->view_pos - input.out_worldpos;
	
#ifdef HAS_CAR_PAINT
	specCol = 0.8; //car paint mat specCol shall be texture based as well
#endif

	_float ao = baked_ao;
#ifdef HAS_CAR_AO
	ao *= car_ao0 * car_ao1;
#endif	
	
#ifdef HAS_LOCAL_EMISSIVE
	_float emissive_translucency = local_emissive;
#else
	_float emissive_translucency = frag_consts->carindex_translucency_ssaostr_fovscale.y;
#endif	
	
	//TODO comment cubemap indexing usage
	_float envmapIdx = frag_consts->carindex_translucency_ssaostr_fovscale.x;
		
#ifdef G_BUFFER_PASS
	output.frag_color = _float4(albedo, emissive_translucency); //albedoRGB, emissive
	output.frag_normal = _float4(encodeNormal(NV)); //encoded view normal
	output.frag_params = _float4(specCol, envmapIdx, smoothness, ao); //specCol, envmapIdx, smoothness, ao
		
	#ifdef EDITOR_MODE
		#ifdef INSTANCING
			int mesh_selected = int(instance_selected[out_instance_id].x);
		#else
			int mesh_selected = editor_mesh_selected;
		#endif 

		if (mesh_selected == 1)
		{
			frag_color = _float4(albedo * editor_select_color , 0.0); //albedoRGB, emissive
			frag_normal = _float4(encodeNormal(NV)); //encoded view normal
			frag_params = _float4(0.0, 0.0, 0.0, 1.0); //specCol, envmapIdx, smoothness, ao	
		}	
	#endif
	
#elif defined HAS_LOCAL_OPACITY //transparent object render directly to light combine target
	
#ifdef STATIC_ENVPROBE_CAPTURE	
	discard_fragment();
#endif
	
	_float roughness = 1.0 - smoothness;
	hfloat3 normalWS = normalize(N);
	
	_float view_length = length(view_dir);
	hfloat3 view_dir_norm = normalize(view_dir);
	
	// Transform Z from view projection space to window space assuming (0, 1) depth range
	// ndc_z = z / w
	// depth = ((depth_range_far - depth_range_near) / 2) * ndc_z + (depth_range_far + depth_range_near) / 2
	_float screenDepth = (out_scPos.z) / out_scPos.w * 0.5 + 0.5;

	_float shadow = shadowCascaded(texture_unit7, sampler7, cascaded_shadow_unit, cascaded_shadow_sampler, screenDepth, hfloat4(input.out_worldpos, 1.0), frag_consts->cascaded_frustum_distances, frag_consts->cascaded_shadow_matrices);

//	_float envContrib = 1.0 - powr(clamp(dot(N, view_dir),0.0,1.0),0.4);
//	specCol = mix(specCol, specCol, envContrib);
	//shadow = 1.0;
	
	_float two_sided_lighting = 0.0;

	
	envmapIdx *= 255.0;
	//if((envmapIdx < 99.5)&& (envmapIdx > 49.5))
	//{
		//envmapIdx -= 50.0;
	//}	

	_float3 final_color = getPBRlighting(envmap1_dp,
										envmap2_dp,
#ifdef HAS_TEXTURE_CUBE_MAP_ARRAY_EXT
										static_envmaps,
                                        static_envmaps_sampler,
#endif
										texture_unit7,
										sampler7,
										view_dir, normalWS, roughness, envmapIdx, specCol, albedo, local_emissive, ao, 1.0, shadow, input.out_worldpos, frame_consts->ground_color, frame_consts->sky_color, frame_consts->global_light_dir, frame_consts->global_light_color, frag_consts->dpcam_view);

#ifdef ALPHA_TEST
	_float opacity = 1.0;
#else
	_float opacity = albedo_extras.w * albedo_extras.w; //sRGB to linear, no sRGB for alpha channels
#endif
	
	final_color = _float3(applyFog(final_color, view_length, view_dir_norm, frame_consts->global_light_dir, frame_consts->global_light_color, frame_consts->fogCol));
	
	#ifdef EDITOR_MODE
		#ifdef INSTANCING
			int mesh_selected = int(instance_selected[out_instance_id].x);
		#else
			int mesh_selected = editor_mesh_selected;
		#endif 
		if (mesh_selected == 1)
		{
			final_color = albedo.xyz * editor_select_color;
		}
	#endif
	
	output.frag_color = RGBD_transparent_encode( _float4(final_color,opacity) );
#else
#error - CHECK SHADER DEFINES GIVEN TO COMPILER//shall either write to G-BUFFER or to LIGHT_COMBINE if transparent
#endif
	return output;
}

#endif // END OF FRAG SHADER
