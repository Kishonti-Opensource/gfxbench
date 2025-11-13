/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"
#include "tessellation_common.h"

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

#ifdef USE_STAGE_IN_CONTROL_POINTS
struct ControlPoint
{
	hfloat3 _in_position	[[attribute(0)]];
	hfloat3 _in_normal		[[attribute(1)]];
	hfloat3 _in_tangent		[[attribute(2)]];
	hfloat2 _in_texcoord0	[[attribute(3)]];
	hfloat2 _in_texcoord1	[[attribute(4)]];

	hfloat3 in_position() { return _in_position; }
	hfloat3 in_normal() { return _in_normal; }
	hfloat3 in_tangent() { return _in_tangent; }
	hfloat4 in_texcoord() { return hfloat4(_in_texcoord0, _in_texcoord1); }
};
#else
struct ControlPoint
{
	_float3 toNormalized(packed_uchar4 c)
	{
		return (_float3(uchar4(c).xyz) * (1.0f / 255.0f) * 2.0) - 1.0f;
	}

	packed_float3 _in_position;
	packed_uchar4 _in_normal;
	packed_uchar4 _in_tangent;
	packed_float4 _in_tesscoord;

	_float3 in_position()
	{
		return _float3(_in_position);
	}

	_float3 in_normal()
	{
		return _float3(toNormalized(_in_normal));
	}
	_float3 in_tangent()
	{
		return _float3(toNormalized(_in_tangent));
	}

	_float4 in_texcoord()
	{
		return _float4(_in_tesscoord);
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

#ifdef USE_TESSELLATION
	hfloat2 out_texcoord0;
	hfloat3 out_normal;

	#ifdef INSTANCING
		int v_instance_id [[flat]];
	#endif
#else
	hfloat2 out_texcoord0;
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
								,device InstanceData* instance_data [[buffer(INSTANCE_UNIFORMS_SLOT)]]
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
		output.out_normal = normalize( cross( bitangent, tangent));
		output.out_texcoord0 = _float2( 0.0);
	#else //HAS_ABS_DISPLACEMENT
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
		output.out_texcoord0 = localVertex.in_texcoord().xy * u;

#ifdef USE_STAGE_IN_CONTROL_POINTS
		localVertex = input.controlPoints[1];
#else
		localVertex = controlPointInput[indexInput[patchId * 3 + 1]];
#endif

		p2 += localVertex.in_position() * v;
		output.out_normal += localVertex.in_normal() * v;
		output.out_texcoord0 += localVertex.in_texcoord().xy * v;

#ifdef USE_STAGE_IN_CONTROL_POINTS
		localVertex = input.controlPoints[2];
#else
		localVertex = controlPointInput[indexInput[patchId * 3 + 2]];
#endif

		p2 += localVertex.in_position() * w;
		output.out_normal += localVertex.in_normal() * w;
		output.out_texcoord0 += localVertex.in_texcoord().xy * w;
		
		hfloat t0_w = texture_unit0.sample(sampler0, output.out_texcoord0).w;
		_float d = t0_w - 0.5 + tessUniforms->tessellation_factor.z;//tessUniforms->tessellation_factor: X factor, Y scale, Z bias
		_float shadow_tess_inflation_factor = 1.5; //to hide detached shadows - a result of low shadow tess factor
		p2 += output.out_normal * d * shadow_tess_inflation_factor * tessUniforms->tessellation_factor.y * clamp(tessUniforms->tessellation_factor.x - 1.0, 0.0, 1.0); //tessellation factor is >1 is active, else 1, can be ~64*/

	#endif

		_float4 tmp;
	#ifdef INSTANCING
	    tmp = _float4( output.out_normal, 0.0) * instance_data[real_instance_id].inv_model;
		output.out_normal = tmp.xyz;
	#else
		tmp = _float4( output.out_normal, 0.0) * uniforms->inv_model;
		output.out_normal = tmp.xyz;
	#endif

	#ifdef INSTANCING
	    hfloat4x4 instance_mvp = uniforms->vp *  instance_data[real_instance_id].model; 
		output.out_position = instance_mvp * _float4( p2, 1.0);
	#else
		output.out_position = uniforms->mvp * _float4( p2, 1.0);
	#endif

	    output.out_position.z = max(output.out_position.z, 0.0); // "pancaking"

#else //USE_TESSELLATION
	output.out_texcoord0 = input.in_texcoord0;

	#ifdef INSTANCING
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
			//NOTE: inverted (CW) winding order to match front-face culling
			if(v_id == 2)
			{
				obj_pos = BL;
			}
			else if(v_id == 1)
			{
				obj_pos = BR;			
			}
			else if(v_id == 0)
			{
				obj_pos = TR;
			}
			else
			{
				obj_pos = TL;
			}
			obj_pos = normalize(obj_pos);
			obj_pos *= size;
	
		#endif //IS_BILLBOARD
        _float4x4 instance_mvp = uniforms->vp * instance_data[i_id].model;
		output.out_position = instance_mvp * _float4(obj_pos, 1.0);
        output.out_position.z = max(output.out_position.z, 0.0); // "pancaking"
	#else //INSTANCING
		output.out_position = uniforms->mvp * _float4(input.in_position, 1.0);
        output.out_position.z = max(output.out_position.z, 0.0); // "pancaking"
	#endif
#endif

	return output;
}

#endif


#ifdef TYPE_fragment

fragment hfloat4 shader_main(VertexOutput input [[stage_in]]
							,texture2d<hfloat> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,sampler sampler0 [[sampler(TEXTURE_UNIT0_SLOT)]])
{

#ifdef ALPHA_TEST
	if (texture_unit0.sample(sampler0, input.out_texcoord0).a < 0.005)
	{
		discard_fragment();
	}
#endif

	return hfloat4(1.0);
}

#endif
