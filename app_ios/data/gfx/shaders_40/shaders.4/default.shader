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

#include "common.h"
#include "occlusion_cull_common.h"
#include "attribute_locations.h"

// texture_unit0 - albedo, opacity or displacement in alpha (UV0) - oldname: color
// texture_unit1 - optional ao map (used for tunnel) - oldname: lightmap
// texture_unit2 - specular color (grayscale), emissive in alpha (UV0) - oldname: mask
// texture_unit3 - normalmap, smoothness in alpha (UV0) - oldname: normal
// texture_unit4 - optional - abs. pos map
// texture_unit5 - optional - abs. norm map
// texture_unit6 - optional -  flake normal perturb map

uniform vec4 tessellation_factor; //limit, scale, bias, max dist
uniform vec4 carindex_translucency_ssaostr_fovscale;
uniform vec4 cam_near_far_pid_vpscale;
uniform highp vec4 frustum_planes[6];
uniform float tessellation_multiplier;

#ifdef TYPE_vertex

/***********************************************************************************/
//							VERTEX SHADER
/***********************************************************************************/

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_TEXCOORD1_LOCATION) in vec2 in_texcoord1;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

#ifdef USE_TESSELLATION //NOTE: world-pos dependent attribs will get sent to PS via tess eval shader (out_...)
	out vec2 v_texcoord0;
	out vec2 v_texcoord1;
	out vec3 v_normal;
	out vec3 v_tangent;
	#ifdef INSTANCING
        flat out int v_instance_id;
	#endif
#else 
	out vec2 out_texcoord0;
	out vec2 out_texcoord1;
	out vec3 out_normal;
	out vec3 out_tangent;
	out vec3 out_worldpos;

	#ifdef VELOCITY_BUFFER
		out vec4 out_scPos ;
		out vec4 out_prevScPos ;
	#endif

	#ifdef HAS_CAR_AO //TODO: replace this with deferred decals
		out vec4 out_car_ao_texcoord0;
		out vec4 out_car_ao_texcoord1;
	#endif
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	#ifndef USE_TESSELLATION
		flat out int out_instance_id;
	#endif
#endif

void main()
{	
	vec3 normal = in_normal;	
	vec3 tangent = in_tangent;
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);
	
#ifdef FAKE_NORMALS
	normal = normalize(in_position);
	tangent = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
#endif

#ifdef USE_TESSELLATION
	gl_Position = vec4( in_position, 1.0); //will be transformed in tess eval
#else

	vec3 obj_pos = in_position;
	#ifdef IS_BILLBOARD
		vec3 cam_right = vec3(view[0][0], view[1][0], view[2][0]);
		vec3 cam_up =    vec3(view[0][1], view[1][1], view[2][1]);
		vec3 cam_fwd =   vec3(view[0][2], view[1][2], view[2][2]);
		float size = length(in_position);
		
		vec3 BL = (-cam_up) + (-cam_right);
		vec3 TL = (cam_up) + (-cam_right);
		vec3 BR = (-cam_up) + (cam_right);
		vec3 TR = (cam_up) + (cam_right);
		if(gl_VertexID == 0)
		{
			obj_pos = BL;
		}
		else if(gl_VertexID == 1)
		{
			obj_pos = BR;			
		}
		else if(gl_VertexID == 2)
		{
			obj_pos = TR;
		}
		else
		{
			obj_pos = TL;
		}
		obj_pos = normalize(obj_pos);
		vec3 rotated_pos = obj_pos;
		obj_pos *= size;
	#endif

	#ifdef INSTANCING		
        int instance_id = gl_InstanceID + instance_offset;
		mat4 instance_mvp = vp * instance_data[instance_id].model;
		vec4 _scPos = instance_mvp * vec4( obj_pos, 1.0);
	
		vec4 world_position = instance_data[instance_id].model * vec4( obj_pos, 1.0);
	#else
		vec4 _scPos = mvp * vec4( obj_pos, 1.0);

		vec4 world_position = model * vec4( obj_pos, 1.0);
	#endif

	gl_Position = _scPos;
	out_worldpos = world_position.xyz;

	#ifdef VELOCITY_BUFFER
		out_scPos = _scPos ;
		#ifdef INSTANCING
			mat4 prev_instance_mvp = prev_vp * instance_data[instance_id].model;
			out_prevScPos = prev_instance_mvp * vec4(obj_pos, 1.0);
		#else
			out_prevScPos = mvp2 * vec4(obj_pos, 1.0); 
		#endif
	#endif

	#ifdef HAS_CAR_AO
		out_car_ao_texcoord0 = car_ao_matrix0 * world_position;
		out_car_ao_texcoord1 = car_ao_matrix1 * world_position;
	#endif
#endif

#ifdef USE_TESSELLATION
	v_texcoord0 = in_texcoord0;
	v_texcoord1 = in_texcoord1;
	#ifdef INSTANCING
		v_instance_id = gl_InstanceID + instance_offset;
	#endif
	
	v_normal = normal; //NOTE: object space displacement
	v_tangent = tangent; //NOTE: object space displacement
#else

	vec4 tmp;
#ifdef INSTANCING
	#ifdef IS_BILLBOARD //only instanced billboard rendering is supported
		out_normal = normalize(rotated_pos * 1.0 + cam_fwd);
		out_tangent = normalize(rotated_pos * 1.0 + cam_right);
	#else
		tmp = vec4( normal, 0.0) * instance_data[instance_id].inv_model;
		out_normal = tmp.xyz;

		tmp = vec4( tangent, 0.0) * instance_data[instance_id].inv_model;
		out_tangent = tmp.xyz;	
	#endif
#else
	tmp = vec4( normal, 0.0) * inv_model;
	out_normal = tmp.xyz;

	tmp = vec4( tangent, 0.0) * inv_model;
	out_tangent = tmp.xyz;
#endif


	out_texcoord0 = in_texcoord0;
	out_texcoord1 = in_texcoord1;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	#ifndef USE_TESSELLATION
		out_instance_id = instance_id;
	#endif
#endif
}

#endif

#ifdef USE_TESSELLATION

/***********************************************************************************/
//							TESS CONTROL
/***********************************************************************************/

#ifdef TYPE_tess_control

in vec2 v_texcoord0[];
in vec2 v_texcoord1[];
in vec3 v_normal[];
in vec3 v_tangent[];
#ifdef INSTANCING
	flat in int v_instance_id[];
#endif


#ifdef HAS_ABS_DISPLACEMENT
#define PATCH_VERTEX_COUNT 16
layout(vertices = 1) out;
patch out _bezier_patch bp;
#ifdef INSTANCING
    flat out int tc_instance_id[];
#endif
#else
#define PATCH_VERTEX_COUNT 3
layout(vertices = 3) out;
out vec2 tc_texcoord0[];
out vec2 tc_texcoord1[];
out vec3 tc_normal[];
out vec3 tc_tangent[];
#ifdef INSTANCING
    flat out int tc_instance_id[];
#endif
#endif
 

bool IsVisible( mat4 mvp_)
{
    // Homogeneous positions
    highp vec4 pos_h[PATCH_VERTEX_COUNT];

    // Bounding box in NDC for occlusion culling
    highp vec3 bb_min;
    highp vec3 bb_max;

    pos_h[0] = mvp_ * gl_in[0].gl_Position;
    bb_min = bb_max = pos_h[0].xyz / pos_h[0].w;

    highp vec4 tmp;
    // Calculate the bounding box in NDC
    for( int i=1; i<PATCH_VERTEX_COUNT; i++)
	{
        pos_h[i] = mvp_ * gl_in[i].gl_Position;
        tmp.xyz = pos_h[i].xyz / pos_h[i].w;

        bb_min = min(bb_min, tmp.xyz);
        bb_max = max(bb_max, tmp.xyz);
    }

    // [-1..1] -> [0...1] Screen-space for occlusion culling
    bb_min = bb_min * vec3(0.5) + vec3(0.5);
    bb_max = bb_max * vec3(0.5) + vec3(0.5);

    // Execute HiZ occlusion culling. Add a small bias to the depth to avoid visible leaks
    if (!IsVisible(bb_min.xy, bb_max.xy, bb_min.z - 0.00001))
    {
        // The patch is not visible, early out
        return false;
    }

#ifdef USE_PBB_EXT
    // Primitive bounding box bounds
    highp vec4 pp_bb_min;
    highp vec4 pp_bb_max;
    
    pp_bb_min = pp_bb_max = pos_h[0];

    for( int i=1; i<PATCH_VERTEX_COUNT; i++)
	{
        tmp = pos_h[i];
        
        pp_bb_min = min(pp_bb_min, tmp);
        pp_bb_max = max(pp_bb_max, tmp);
    }

    gl_BoundingBoxEXT[0] = pp_bb_min;
	gl_BoundingBoxEXT[1] = pp_bb_max;

#endif
    return true;
}
 
void CalculatePBB( mat4 mvp_)
{
#ifdef USE_PBB_EXT
	vec4 bbmin;
	vec4 bbmax;
	
	bbmin = mvp_ * gl_in[0].gl_Position;
	
	bbmax = bbmin;
	
	for( int i=1; i<gl_PatchVerticesIn; i++)
	{
		vec4 tmp;
		
		tmp = mvp_ * gl_in[i].gl_Position;
		
		bbmin.x = bbmin.x > tmp.x ? tmp.x : bbmin.x;
		bbmax.x = bbmax.x < tmp.x ? tmp.x : bbmax.x;

		bbmin.y = bbmin.y > tmp.y ? tmp.y : bbmin.y;
		bbmax.y = bbmax.y < tmp.y ? tmp.y : bbmax.y;

		bbmin.z = bbmin.z > tmp.z ? tmp.z : bbmin.z;
		bbmax.z = bbmax.z < tmp.z ? tmp.z : bbmax.z;

		bbmin.w = bbmin.w > tmp.w ? tmp.w : bbmin.w;
		bbmax.w = bbmax.w < tmp.w ? tmp.w : bbmax.w;
	}

	gl_BoundingBoxEXT[0] = bbmin;
	gl_BoundingBoxEXT[1] = bbmax;
#endif
}

void main()
{
// Calculate PBB during occlusion culling 
#ifdef INSTANCING
	mat4 instance_mvp = vp *  instance_data[v_instance_id[gl_InvocationID]].model;
    bool is_visible = IsVisible(instance_mvp);    
#else	
     bool is_visible = IsVisible(mvp);
#endif


#ifdef HAS_ABS_DISPLACEMENT
    if (!is_visible)
    {
        // Covered by occluder
        gl_TessLevelInner[0] = 0.0;
        gl_TessLevelInner[1] = 0.0;
		gl_TessLevelOuter[0] = 0.0;
		gl_TessLevelOuter[1] = 0.0;
		gl_TessLevelOuter[2] = 0.0;
		gl_TessLevelOuter[3] = 0.0;
        return;
    }

	vec3 v0 = (mv * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
	vec3 v3 = (mv * vec4(gl_in[3].gl_Position.xyz, 1.0)).xyz;
	vec3 v12 = (mv * vec4(gl_in[12].gl_Position.xyz, 1.0)).xyz;
	vec3 v15 = (mv * vec4(gl_in[15].gl_Position.xyz, 1.0)).xyz;
	
	//do not tessellate back-faces
	{
		vec3 edge0 = v3 - v0;
		vec3 edge2 = v12 - v0;
		
		vec3 faceNorm = normalize(cross(edge0, edge2));
		
		
		//NOTE: just an approximation, as the two tris of a quad might not lie on a plane
		//have to use large offset otherwise gaps would appear
		if(dot(normalize(v0),faceNorm) > 0.15)
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
			return;
		}
	}

    float r0 = distance(v0, v12) * 0.5;
	float r1 = distance(v0, v3) * 0.5;
	float r2 = distance(v3, v15) * 0.5;
	float r3 = distance(v12, v15) * 0.5;
	
	//do not tessellate patches outside the frustum
	{
		vec3 wA = (model * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
		vec3 wB = (model * vec4(gl_in[3].gl_Position.xyz, 1.0)).xyz;
		vec3 wC = (model * vec4(gl_in[12].gl_Position.xyz, 1.0)).xyz;
		vec3 wD = (model * vec4(gl_in[15].gl_Position.xyz, 1.0)).xyz;		
	
		vec4 centrojd = vec4(vec3(wA + wB + wC + wD) * 0.25, 1.0); //world space
		float radius = 1.5 * max(r0, max(r1, max(r2, r3))); //this will safely do for radius
		
		vec4 dists;
		dists.x = dot(frustum_planes[0], centrojd);
		dists.y = dot(frustum_planes[1], centrojd);
		dists.z = dot(frustum_planes[2], centrojd);
		dists.w = dot(frustum_planes[3], centrojd);
				
		bvec4 comp = lessThan(dists, vec4(-radius));
		
		float dist_near = dot(frustum_planes[5], centrojd);
		
		if(any(comp) || (dist_near < -radius))
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
			return;			
		}	
	}
    
	//compute tess-factor based on screenspace edge-size
    vec3 C0 = (v0 + v12) * 0.5;
	vec3 C1 = (v0 + v3) * 0.5;
	vec3 C2 = (v3 + v15) * 0.5;
	vec3 C3 = (v12 + v15) * 0.5;
	
	float screenSize0 = r0 * cam_near_far_pid_vpscale.x / abs(C0.z);
	float screenSize1 = r1 * cam_near_far_pid_vpscale.x / abs(C1.z);
	float screenSize2 = r2 * cam_near_far_pid_vpscale.x / abs(C2.z);
	float screenSize3 = r3 * cam_near_far_pid_vpscale.x / abs(C3.z);
	
	float t0 = clamp(screenSize0 * tessellation_multiplier, 1.0, tessellation_factor.x);
	float t1 = clamp(screenSize1 * tessellation_multiplier, 1.0, tessellation_factor.x);
	float t2 = clamp(screenSize2 * tessellation_multiplier, 1.0, tessellation_factor.x);
	float t3 = clamp(screenSize3 * tessellation_multiplier, 1.0, tessellation_factor.x);
	
    float t_inner0 = max(t1, t3);
    float t_inner1 = max(t0, t2);
	
	gl_TessLevelInner[0] = t_inner0;
	gl_TessLevelInner[1] = t_inner1;

	gl_TessLevelOuter[0] = t0;
	gl_TessLevelOuter[1] = t1;
	gl_TessLevelOuter[2] = t2;
	gl_TessLevelOuter[3] = t3;

	#ifdef INSTANCING
        tc_instance_id[gl_InvocationID] = v_instance_id[gl_InvocationID];       
	#endif

	bp.Px = mat4(
	gl_in[0].gl_Position.x, gl_in[1].gl_Position.x, gl_in[2].gl_Position.x, gl_in[3].gl_Position.x,
	gl_in[4].gl_Position.x, gl_in[5].gl_Position.x, gl_in[6].gl_Position.x, gl_in[7].gl_Position.x,
	gl_in[8].gl_Position.x, gl_in[9].gl_Position.x, gl_in[10].gl_Position.x, gl_in[11].gl_Position.x,
	gl_in[12].gl_Position.x, gl_in[13].gl_Position.x, gl_in[14].gl_Position.x, gl_in[15].gl_Position.x
	);

	bp.Py = mat4(
	gl_in[0].gl_Position.y, gl_in[1].gl_Position.y, gl_in[2].gl_Position.y, gl_in[3].gl_Position.y,
	gl_in[4].gl_Position.y, gl_in[5].gl_Position.y, gl_in[6].gl_Position.y, gl_in[7].gl_Position.y,
	gl_in[8].gl_Position.y, gl_in[9].gl_Position.y, gl_in[10].gl_Position.y, gl_in[11].gl_Position.y,
	gl_in[12].gl_Position.y, gl_in[13].gl_Position.y, gl_in[14].gl_Position.y, gl_in[15].gl_Position.y
	);

	bp.Pz = mat4(
	gl_in[0].gl_Position.z, gl_in[1].gl_Position.z, gl_in[2].gl_Position.z, gl_in[3].gl_Position.z,
	gl_in[4].gl_Position.z, gl_in[5].gl_Position.z, gl_in[6].gl_Position.z, gl_in[7].gl_Position.z,
	gl_in[8].gl_Position.z, gl_in[9].gl_Position.z, gl_in[10].gl_Position.z, gl_in[11].gl_Position.z,
	gl_in[12].gl_Position.z, gl_in[13].gl_Position.z, gl_in[14].gl_Position.z, gl_in[15].gl_Position.z
	);

	bp.Px = BT *  bp.Px * BT;
	bp.Py = BT *  bp.Py * BT;
	bp.Pz = BT *  bp.Pz * BT;

#else
    if (!is_visible)
    {
        // Covered by occluder
        gl_TessLevelInner[0] = 0.0;
        gl_TessLevelInner[1] = 0.0;
		gl_TessLevelOuter[0] = 0.0;
		gl_TessLevelOuter[1] = 0.0;
		gl_TessLevelOuter[2] = 0.0;
		gl_TessLevelOuter[3] = 0.0;
        return;
    }

	mat4 mv_;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tc_texcoord0[gl_InvocationID] = v_texcoord0[gl_InvocationID];
	tc_texcoord1[gl_InvocationID] = v_texcoord1[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	tc_tangent[gl_InvocationID] = v_tangent[gl_InvocationID];

	mv_ = mv;
	#ifdef INSTANCING
        tc_instance_id[gl_InvocationID] = v_instance_id[gl_InvocationID];
		mv_ = view * instance_data[v_instance_id[gl_InvocationID]].model;
	#endif  

	vec3 vA = (mv_ * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
	vec3 vB = (mv_ * vec4(gl_in[1].gl_Position.xyz, 1.0)).xyz;
	vec3 vC = (mv_ * vec4(gl_in[2].gl_Position.xyz, 1.0)).xyz;

	//do not tessellate back-faces
	{
		vec3 edge0 = vB - vA;
		vec3 edge2 = vC - vA;
		
		vec3 faceNorm = normalize(cross(edge0, edge2));
		
		if(dot(normalize(vA),faceNorm) > 0.4) //have to use large offset otherwise gaps would appear
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			return;
		}
	}
    
    float r0 = distance(vA, vB) * 0.5;
	float r1 = distance(vB, vC) * 0.5;
	float r2 = distance(vC, vA) * 0.5;
	
	//do not tessellate patches outside the frustum
	{
		//0..3: sides, 4: far, 5: near
		
		#ifdef INSTANCING
			vec3 wA = (instance_data[v_instance_id[gl_InvocationID]].model * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
			vec3 wB = (instance_data[v_instance_id[gl_InvocationID]].model * vec4(gl_in[1].gl_Position.xyz, 1.0)).xyz;
			vec3 wC = (instance_data[v_instance_id[gl_InvocationID]].model * vec4(gl_in[2].gl_Position.xyz, 1.0)).xyz;
		#else
			vec3 wA = (model * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
			vec3 wB = (model * vec4(gl_in[1].gl_Position.xyz, 1.0)).xyz;
			vec3 wC = (model * vec4(gl_in[2].gl_Position.xyz, 1.0)).xyz;		
		#endif
		
		vec4 centrojd = vec4(vec3(wA + wB + wC) * 0.3333, 1.0); //world space
		float radius = 1.5 * max(r0, max(r1, r2)); //this will safely do for radius
		
		vec4 dists;
		dists.x = dot(frustum_planes[0], centrojd);
		dists.y = dot(frustum_planes[1], centrojd);
		dists.z = dot(frustum_planes[2], centrojd);
		dists.w = dot(frustum_planes[3], centrojd);
				
		bvec4 comp = lessThan(dists, vec4(-radius));
		
		float dist_near = dot(frustum_planes[5], centrojd);
		
		if(any(comp) || (dist_near < -radius))
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			return;			
		}
		
/*		
		//alternative method: just check vertices in NDC - has artifacts if no verts are visible but triangle still is
		vec4 proj0 = mvp * vec4(gl_in[0].gl_Position.xyz, 1.0);
		vec4 proj1 = mvp * vec4(gl_in[1].gl_Position.xyz, 1.0);
		vec4 proj2 = mvp * vec4(gl_in[2].gl_Position.xyz, 1.0);
			
		vec3 aproj0 = abs(proj0).xyz;
		vec3 aproj1 = abs(proj1).xyz;
		vec3 aproj2 = abs(proj2).xyz;
		
		vec3 acomp = abs(vec3(proj0.w, proj1.w, proj2.w));
		
		bvec3 comp0 = greaterThanEqual(aproj0, acomp.xxx);
		bvec3 comp1 = greaterThanEqual(aproj1, acomp.yyy);
		bvec3 comp2 = greaterThanEqual(aproj2, acomp.zzz);
		
		if(any(comp0) && any(comp1) && any(comp2))
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			return;	
		}
*/
	}

	//compute tess-factor based on screenspace edge-size	
	vec3 C0 = (vA + vB) * 0.5;
	vec3 C1 = (vB + vC) * 0.5;
	vec3 C2 = (vC + vA) * 0.5;
		
	float screenSize0 = r0 * cam_near_far_pid_vpscale.x / abs(C0.z);
	float screenSize1 = r1 * cam_near_far_pid_vpscale.x / abs(C1.z);
	float screenSize2 = r2 * cam_near_far_pid_vpscale.x / abs(C2.z);
	
	//tc_patch_size[gl_InvocationID] = max(max(screenSize0, screenSize1), screenSize2)  * 10.0;	
	float t0 = clamp(screenSize0 * tessellation_multiplier, 1.0, tessellation_factor.x);
	float t1 = clamp(screenSize1 * tessellation_multiplier, 1.0, tessellation_factor.x);
	float t2 = clamp(screenSize2 * tessellation_multiplier, 1.0, tessellation_factor.x);
	
    float t_inner = max(t1, t2);
	
	gl_TessLevelInner[0] = t_inner;

	gl_TessLevelOuter[0] = t1;
	gl_TessLevelOuter[1] = t2;
	gl_TessLevelOuter[2] = t0;
#endif
}

#endif // END OF TESS CTRL SHADER


#ifdef TYPE_tess_eval

/***********************************************************************************/
//							TESS EVAL
/***********************************************************************************/


#ifdef HAS_ABS_DISPLACEMENT
layout(quads, fractional_odd_spacing, ccw) in;
patch in _bezier_patch bp;
#ifdef INSTANCING
    flat in int tc_instance_id[];
#endif
#else //HAS_ABS_DISPLACEMENT
layout(triangles, fractional_odd_spacing, ccw) in;
//in float tc_patch_size[];
//out float te_patch_size;
in vec2 tc_texcoord0[];
in vec2 tc_texcoord1[];
in vec3 tc_normal[];
in vec3 tc_tangent[];
#ifdef INSTANCING
    flat in int tc_instance_id[];
#endif
#endif //HAS_ABS_DISPLACEMENT


out vec2 out_texcoord0;
out vec2 out_texcoord1;
out vec3 out_normal;
out vec3 out_tangent;
out vec3 out_worldpos;

#ifdef VELOCITY_BUFFER
	out vec4 out_scPos ;
	out vec4 out_prevScPos ;
#endif

#ifdef HAS_CAR_AO //TODO: replace this with deferred decals
	out vec4 out_car_ao_texcoord0;
	out vec4 out_car_ao_texcoord1;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	flat out int out_instance_id;	
#endif

void main()
{	
#ifdef HAS_ABS_DISPLACEMENT
	vec3 position;
	vec3 tangent;
	vec3 bitangent;

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec4 U = vec4( u*u*u, u*u, u, 1.0);
	vec4 V = vec4( v*v*v, v*v, v, 1.0);
	vec4 dU = vec4( 3.0 * u * u, 2.0 * u, 1.0, 0.0);
	vec4 dV = vec4( 3.0 * v * v, 2.0 * v, 1.0, 0.0);

	position.x = dot( U * bp.Px, V);
	position.y = dot( U * bp.Py, V);
	position.z = dot( U * bp.Pz, V);
	
	tangent.x = dot( dU * bp.Px, V);
	tangent.y = dot( dU * bp.Py, V);
	tangent.z = dot( dU * bp.Pz, V);
	
	bitangent.x = dot( U * bp.Px, dV );
	bitangent.y = dot( U * bp.Py, dV );
	bitangent.z = dot( U * bp.Pz, dV );
	
	vec3 p2 = position;
	out_normal = normalize( cross( tangent, bitangent));

	out_tangent = tangent;
	out_texcoord0 = vec2( 0.0);
	out_texcoord1 = vec2( 0.0);
#else //HAS_ABS_DISPLACEMENT
	//te_patch_size = tc_patch_size[0];

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	out_texcoord0 = tc_texcoord0[0] * u + tc_texcoord0[1] * v + tc_texcoord0[2] * w;
	out_texcoord1 = tc_texcoord1[0] * u + tc_texcoord1[1] * v + tc_texcoord1[2] * w;
	out_normal = tc_normal[0] * u + tc_normal[1] * v + tc_normal[2] * w;
	out_tangent = tc_tangent[0] * u + tc_tangent[1] * v + tc_tangent[2] * w;
	
	vec3 p2 = gl_in[0].gl_Position.xyz * u + gl_in[1].gl_Position.xyz * v + gl_in[2].gl_Position.xyz * w;
	
	highp float t0_w = texture( texture_unit0, out_texcoord0).w;
	float d = t0_w - 0.5 + tessellation_factor.z;//tessellation_factor: X factor, Y scale, Z bias
	p2 += out_normal * d * tessellation_factor.y * clamp(tessellation_factor.x - 1.0, 0.0, 1.0); //tessellation factor is >1 is active, else 1, can be ~64*/
#endif //HAS_ABS_DISPLACEMENT

	vec4 tmp;
#ifdef INSTANCING

    tmp = vec4( out_normal, 0.0) * instance_data[tc_instance_id[0]].inv_model;
	out_normal = tmp.xyz;

    tmp = vec4( out_tangent, 0.0) * instance_data[tc_instance_id[0]].inv_model;
	out_tangent = tmp.xyz;
#else
	tmp = vec4( out_normal, 0.0) * inv_model;
	out_normal = tmp.xyz;

	tmp = vec4( out_tangent, 0.0) * inv_model;
	out_tangent = tmp.xyz;
#endif

#ifdef INSTANCING
	mat4 instance_mvp = vp *  instance_data[tc_instance_id[0]].model; 
	vec4 _scPos = instance_mvp * vec4( p2, 1.0);

	vec4 world_position = instance_data[tc_instance_id[0]].model * vec4( p2, 1.0);
#else
	vec4 _scPos = mvp * vec4( p2, 1.0);

	vec4 world_position = model * vec4( p2, 1.0);
#endif
	out_worldpos = world_position.xyz;

#ifdef VELOCITY_BUFFER
	out_scPos = _scPos ;
	#ifdef INSTANCING
		mat4 instance_prev_mvp = prev_vp * instance_data[tc_instance_id[0]].model; 
		out_prevScPos = instance_prev_mvp * vec4(p2, 1.0); 
	#else
		out_prevScPos = mvp2 * vec4(p2, 1.0); 
	#endif
#endif

#ifdef HAS_CAR_AO
	out_car_ao_texcoord0 = car_ao_matrix0 * world_position;
	out_car_ao_texcoord1 = car_ao_matrix1 * world_position;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	out_instance_id = tc_instance_id[0];
#endif

    gl_Position = _scPos;
}

#endif // END OF TESS EVAL SHADER

#endif //END OF TESSELLATION PATH

#ifdef DRAW_GS_WIREFRAME

#ifdef TYPE_geometry

/***********************************************************************************/
//							GEOMETRY
/***********************************************************************************/

#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
//in float te_patch_size[];
//out float g_patch_size;
#endif
#endif

in vec2 out_texcoord0[];
in vec2 out_texcoord1[];
in vec3 out_normal[];
in vec3 out_tangent[];
in vec3 out_worldpos[];

#ifdef HAS_CAR_AO
	in vec4 out_car_ao_texcoord0[];
	in vec4 out_car_ao_texcoord1[];
#endif

#ifdef VELOCITY_BUFFER
	in vec4 out_scPos[];
	in vec4 out_prevScPos[];
#endif


out vec2 fs_texcoord0;
out vec2 fs_texcoord1;
out vec3 fs_normal;
out vec3 fs_tangent;
out vec3 fs_worldpos;

noperspective out vec3 dist;

#ifdef VELOCITY_BUFFER
	out vec4 fs_scPos ;
	out vec4 fs_prevScPos ;
#endif

#ifdef HAS_CAR_AO //TODO: replace this with deferred decals
	out vec4 fs_car_ao_texcoord0;
	out vec4 fs_car_ao_texcoord1;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	flat in int out_instance_id[];
	flat out int fs_instance_id;
#endif

//layout (binding = 1, offset = 0) uniform atomic_uint clip_counter;

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
void main()
{
/*    for (int i = 0; i < 3; i++)
    {
            gl_Position = gl_in[i].gl_Position;
        
            fs_texcoord0 = out_texcoord0[i];
            fs_texcoord1 = out_texcoord1[i];
            fs_normal = out_normal[i];
            fs_tangent = out_tangent[i];
            fs_worldpos = out_worldpos[i];
            
    #ifdef VELOCITY_BUFFER
            fs_scPos = out_scPos[i];
            fs_prevScPos = out_prevScPos[i];
    #endif
            
    #ifdef HAS_CAR_AO //TODO: replace this with deferred decals
            fs_car_ao_texcoord0 = out_car_ao_texcoord0[i];
            fs_car_ao_texcoord1 = out_car_ao_texcoord1[i];
    #endif
            
            EmitVertex();
    }
    
    EndPrimitive();    
*/	

	// taken from 'Single-Pass Wireframe Rendering'
	vec2 WIN_SCALE = vec2(1280.0, 720.0);
	vec2 p0 = WIN_SCALE * gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
	vec2 p1 = WIN_SCALE * gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;
	vec2 p2 = WIN_SCALE * gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w;
	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;
	float area = abs(v1.x*v2.y - v1.y * v2.x);

	dist = vec3(area/length(v0),0,0);
	gl_Position = gl_in[0].gl_Position;
	fs_texcoord0 = out_texcoord0[0];
	fs_texcoord1 = out_texcoord1[0];
	fs_normal = out_normal[0];
	fs_tangent = out_tangent[0];
	fs_worldpos = out_worldpos[0];
    #ifdef VELOCITY_BUFFER
		fs_scPos = out_scPos[0];
		fs_prevScPos = out_prevScPos[0];
    #endif
    #ifdef HAS_CAR_AO //TODO: replace this with deferred decals
		fs_car_ao_texcoord0 = out_car_ao_texcoord0[0];
		fs_car_ao_texcoord1 = out_car_ao_texcoord1[0];
    #endif
#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
	//g_patch_size = te_patch_size[0];
#endif	
#endif
#if (defined EDITOR_MODE) && (defined INSTANCING)
	fs_instance_id = out_instance_id[0];
#endif
	EmitVertex();
	
	dist = vec3(0,area/length(v1),0);
	gl_Position = gl_in[1].gl_Position;
	fs_texcoord0 = out_texcoord0[1];
	fs_texcoord1 = out_texcoord1[1];
	fs_normal = out_normal[1];
	fs_tangent = out_tangent[1];
	fs_worldpos = out_worldpos[1];
    #ifdef VELOCITY_BUFFER
		fs_scPos = out_scPos[1];
		fs_prevScPos = out_prevScPos[1];
    #endif
    #ifdef HAS_CAR_AO //TODO: replace this with deferred decals
            fs_car_ao_texcoord0 = out_car_ao_texcoord0[1];
            fs_car_ao_texcoord1 = out_car_ao_texcoord1[1];
    #endif
#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
	//g_patch_size = te_patch_size[1];
#endif	
#endif
#if (defined EDITOR_MODE) && (defined INSTANCING)
	fs_instance_id = out_instance_id[1];
#endif
	EmitVertex();

	dist = vec3(0,0,area/length(v2));
	gl_Position = gl_in[2].gl_Position;
	fs_texcoord0 = out_texcoord0[2];
	fs_texcoord1 = out_texcoord1[2];
	fs_normal = out_normal[2];
	fs_tangent = out_tangent[2];
	fs_worldpos = out_worldpos[2];
    #ifdef VELOCITY_BUFFER
		fs_scPos = out_scPos[2];
		fs_prevScPos = out_prevScPos[2];
    #endif            
    #ifdef HAS_CAR_AO //TODO: replace this with deferred decals
		fs_car_ao_texcoord0 = out_car_ao_texcoord0[2];
		fs_car_ao_texcoord1 = out_car_ao_texcoord1[2];
    #endif
#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
	//g_patch_size = te_patch_size[2];
#endif	
#endif
#if (defined EDITOR_MODE) && (defined INSTANCING)
	fs_instance_id = out_instance_id[2];
#endif

	EmitVertex();
	
	EndPrimitive();

}

#endif // END OF TESS EVAL SHADER

#endif //END OF GS PATH

#ifdef TYPE_fragment

/***********************************************************************************/
//							FRAGMENT SHADER
/***********************************************************************************/

#ifdef USE_TESSELLATION
#ifndef HAS_ABS_DISPLACEMENT
//in float g_patch_size;
#endif
#endif

#ifdef DRAW_GS_WIREFRAME

in vec2 fs_texcoord0;
in vec2 fs_texcoord1;
in vec3 fs_normal;
in vec3 fs_tangent;
in highp vec3 fs_worldpos;

noperspective in vec3 dist;

#ifdef HAS_CAR_AO
	in vec4 fs_car_ao_texcoord0;
	in vec4 fs_car_ao_texcoord1;
#endif

#ifdef VELOCITY_BUFFER
	layout(location = 1) out VELOCITY_BUFFER_TYPE velocity ;
	in vec4 fs_scPos ;
	in vec4 fs_prevScPos ;
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	flat in int fs_instance_id;
#endif

#else

in vec2 out_texcoord0;
in vec2 out_texcoord1;
in vec3 out_normal;
in vec3 out_tangent;
in highp vec3 out_worldpos;

#ifdef HAS_CAR_AO
	in vec4 out_car_ao_texcoord0;
	in vec4 out_car_ao_texcoord1;
#endif

#ifdef VELOCITY_BUFFER
	layout(location = 1) out VELOCITY_BUFFER_TYPE velocity ;
	in vec4 out_scPos ;
	in vec4 out_prevScPos ;
#endif

#endif

#ifdef G_BUFFER_PASS
	layout(location = 2) out vec4 frag_normal;
	layout(location = 3) out vec4 frag_params;
#endif

layout(location = 0) out vec4 frag_color;


#ifdef HAS_CAR_PAINT
	const float microFlakePerturbA = 0.1;
	const float microFlakePerturb = 1.0;
	const float normalPerturb = 1.0;

	//Watch intensities: colors are listed in from broad to narrow focus order, and add up
	const vec3 paintColor0 = vec3(0.1, 0.1, 0.1);
	const vec3 paintColorMid = vec3(0.15, 0.15, 0.15);
	const vec3 paintColor2 = vec3(0.17, 0.17, 0.17);
	const vec3 flakeLayerColor = vec3(0.252, 0.25, 0.25);
#endif

#if (defined EDITOR_MODE) && (defined INSTANCING)
	flat in int out_instance_id;
#endif

void main()
{	
#ifdef DRAW_GS_WIREFRAME
	vec2 out_texcoord0 = fs_texcoord0;
	vec2 out_texcoord1 = fs_texcoord1;
	vec3 out_normal = fs_normal;
	vec3 out_tangent = fs_tangent;
	vec3 out_worldpos = fs_worldpos;

#ifdef HAS_CAR_AO	
	vec4 out_car_ao_texcoord0 = fs_car_ao_texcoord0;
	vec4 out_car_ao_texcoord1 = fs_car_ao_texcoord1;
#endif

#ifdef VELOCITY_BUFFER
	vec4 out_scPos = fs_scPos;
	vec4 out_prevScPos = fs_prevScPos;
#endif

	#if (defined EDITOR_MODE) && (defined INSTANCING)
		int out_instance_id = fs_instance_id;
	#endif

#endif


#ifdef HAS_CAR_AO //TODO: replace this with deferred decals
	float car_ao0 = 1.0;
	float car_ao1 = 1.0;
#ifndef STATIC_ENVPROBE_CAPTURE
	car_ao0 = textureProj( texture_unit4, out_car_ao_texcoord0).x;
	if( out_car_ao_texcoord0.x < 0.0 || out_car_ao_texcoord0.x > 1.0 || out_car_ao_texcoord0.y < 0.0 || out_car_ao_texcoord0.y > 1.0 || out_car_ao_texcoord0.z < 0.0 || out_car_ao_texcoord0.z > 1.0) //hack not to project ao onto overhangs
		car_ao0 = 1.0;

	car_ao1 = textureProj( texture_unit4, out_car_ao_texcoord1).x;
	if( out_car_ao_texcoord1.x < 0.0 || out_car_ao_texcoord1.x > 1.0 || out_car_ao_texcoord1.y < 0.0 || out_car_ao_texcoord1.y > 1.0 || out_car_ao_texcoord1.z < 0.0 || out_car_ao_texcoord1.z > 1.0) //hack not to project ao onto overhangs
		car_ao1 = 1.0;
#endif
#endif

	//NOTE: no sRGB format for alpha channels
	vec4 albedo_extras = texture( texture_unit0, out_texcoord0); //+ displacement or opacity
	
	//NOTE: on desktop, DXT can produce artifacts with dark textures unless it is compressed with gamma	
	//(relates to "greening" DXT compression artifacts)	
#ifndef GL_ES
	albedo_extras = pow(albedo_extras, gamma_exp);
#endif
	
	vec3 albedo = albedo_extras.xyz;
	
#ifdef DRAW_GS_WIREFRAME	
	float nearD = min(min(dist[0],dist[1]),dist[2]);
	float edgeIntensity = exp2(-1.0*nearD*nearD);	
	albedo = mix(vec3(0.5), vec3(0.0), edgeIntensity);
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
		discard;
#endif
	}
#endif	

	//NOTE: texture_unit1 used to store baked shadow, ao, emissive
	float baked_ao = 1.0; //texture( texture_unit1, out_texcoord1).x;
#if defined(HAS_CAR_PAINT) || defined(USE_WORLD_AO)
	highp vec2 uv = out_worldpos.xz / 1950.0;
	uv += vec2(-0.5, 0.5);
	baked_ao = texture(texture_unit7, uv).x;
#else
	baked_ao = texture( texture_unit1, out_texcoord1).x;//tunnel
#endif
	
	vec4 specCol_emissive = texture( texture_unit2, out_texcoord0);
	vec4 texNorm_smoothness = texture(texture_unit3, out_texcoord0);

#ifdef VELOCITY_BUFFER
	//velocity = pack2FloatToVec4( velocityFunction(out_scPos,out_prevScPos) );
	velocity = pack_velocity( velocityFunction(out_scPos,out_prevScPos) );
#endif	
	
	float specCol = specCol_emissive.y; //TODO specular color can be compressed as greyscale
	float local_emissive = 0.0;
#ifdef HAS_LOCAL_EMISSIVE
	local_emissive = sqrt(specCol_emissive.w); //so source data has more precision in the dark when passing through the G-buffer
#endif
	
	float smoothness = texNorm_smoothness.w; //smoothness is assumed linear, not sRGB!
	vec3 texNorm = vec3(0.0);
	highp vec3 N = vec3(0.0);
#ifdef HAS_CAR_PAINT
	N = normalize(out_normal);
#else
	texNorm = texNorm_smoothness.xyz;
	N = calcWorldNormal(texNorm, out_normal, out_tangent);
#endif
	
	highp vec3 NV = normalize(vec4(N,0) * inv_view).xyz; //view-space normals
	
#ifdef IS_TWO_SIDED	
	if(gl_FrontFacing == false)
	{
		NV *= -1.0;
	}
#endif
	
	highp vec3 view_dir = view_pos - out_worldpos;
	
#ifdef HAS_CAR_PAINT
	specCol = 0.8; //car paint mat specCol shall be texture based as well
#endif

	float ao = baked_ao;
#ifdef HAS_CAR_AO
	ao *= car_ao0 * car_ao1;
#endif	
	
#ifdef HAS_LOCAL_EMISSIVE
	float emissive_translucency = local_emissive;
#else
	float emissive_translucency = carindex_translucency_ssaostr_fovscale.y;
#endif	
	
	//TODO comment cubemap indexing usage
	float envmapIdx = carindex_translucency_ssaostr_fovscale.x;
		
#ifdef G_BUFFER_PASS
	frag_color = vec4(albedo, emissive_translucency); //albedoRGB, emissive
	frag_normal = encodeNormal(NV); //encoded view normal
	frag_params = vec4(specCol, envmapIdx, smoothness, ao); //specCol, envmapIdx, smoothness, ao	
		
	#ifdef EDITOR_MODE
		#ifdef INSTANCING
			int mesh_selected = int(instance_selected[out_instance_id].x);
		#else
			int mesh_selected = editor_mesh_selected;
		#endif 

		if (mesh_selected == 1)
		{
			frag_color = vec4(albedo * editor_select_color , 0.0); //albedoRGB, emissive
			frag_normal = encodeNormal(NV); //encoded view normal
			frag_params = vec4(0.0, 0.0, 0.0, 1.0); //specCol, envmapIdx, smoothness, ao	
		}	
	#endif
	
#elif defined HAS_LOCAL_OPACITY //transparent object render directly to light combine target
	
#ifdef STATIC_ENVPROBE_CAPTURE	
	discard;
#endif
	
	float roughness = 1.0 - smoothness;
	highp vec3 normalWS = normalize(N);
	
	float view_length = length(view_dir);
	vec3 view_dir_norm = normalize(view_dir);
	
	// Transform Z from view projection space to window space assuming (0, 1) depth range
	// ndc_z = z / w
	// depth = ((depth_range_far - depth_range_near) / 2) * ndc_z + (depth_range_far + depth_range_near) / 2
	float screenDepth = (out_scPos.z) / out_scPos.w * 0.5 + 0.5;
	
	float shadow = shadowCascaded(screenDepth, vec4(out_worldpos, 1.0));
	
//	float envContrib = 1.0 - pow(clamp(dot(N, view_dir),0.0,1.0),0.4);
//	specCol = mix(specCol, specCol, envContrib);
	//shadow = 1.0;
	
	float two_sided_lighting = 0.0;

	
	envmapIdx *= 255.0;
	//if((envmapIdx < 99.5)&& (envmapIdx > 49.5))
	//{
		//envmapIdx -= 50.0;
	//}	
	
	vec3 final_color = getPBRlighting(view_dir, normalWS, roughness, envmapIdx, specCol, albedo, local_emissive, ao, 1.0, shadow, out_worldpos); 

#ifdef ALPHA_TEST
	float opacity = 1.0;
#else
	float opacity = albedo_extras.w * albedo_extras.w; //sRGB to linear, no sRGB for alpha channels
#endif
	
	final_color = applyFog(final_color, view_length, view_dir_norm);
	
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
	
	frag_color = RGBD_transparent_encode( vec4(final_color,opacity) );
	
#else
#error - CHECK SHADER DEFINES GIVEN TO COMPILER//shall either write to G-BUFFER or to LIGHT_COMBINE if transparent
#endif
}

#endif // END OF FRAG SHADER
