/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"

uniform vec4 tessellation_factor;

#ifdef TYPE_vertex

/***********************************************************************************/
//							VERTEX SHADER
/***********************************************************************************/

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_TEXCOORD1_LOCATION) in vec2 in_texcoord1;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

#ifdef USE_TESSELLATION
	out vec2 v_texcoord0;
	out vec3 v_normal;

	#ifdef INSTANCING	
		flat out int v_instance_id;
	#endif
#else
	out vec2 out_texcoord0;
#endif

void main()
{
#ifdef USE_TESSELLATION
	v_texcoord0 = in_texcoord0;

	vec3 normal = in_normal;	
	decodeFromByteVec3(normal);

	gl_Position = vec4( in_position, 1.0); //will be transformed in tess eval    
	v_normal = normal; //NOTE: object space displacement

	#ifdef INSTANCING
        v_instance_id = gl_InstanceID + instance_offset;
	#endif
#else //USE_TESSELLATION
	out_texcoord0 = in_texcoord0;

	#ifdef INSTANCING
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
			//NOTE: inverted (CW) winding order to match front-face culling
			if(gl_VertexID == 2)
			{
				obj_pos = BL;
			}
			else if(gl_VertexID == 1)
			{
				obj_pos = BR;			
			}
			else if(gl_VertexID == 0)
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
        mat4 instance_mvp = vp * instance_data[gl_InstanceID + instance_offset].model;
		gl_Position = instance_mvp * vec4(obj_pos, 1.0);      
        gl_Position.z = max(gl_Position.z, -1.0); // "pancaking"
	#else //INSTANCING
		gl_Position = mvp * vec4(in_position, 1.0);
        gl_Position.z = max(gl_Position.z, -1.0); // "pancaking"
	#endif
#endif
}

#endif

#ifdef USE_TESSELLATION

/***********************************************************************************/
//							TESS CONTROL
/***********************************************************************************/

#ifdef TYPE_tess_control

#ifdef HAS_ABS_DISPLACEMENT
layout(vertices = 1) out;
patch out _bezier_patch bp;
#ifdef INSTANCING
    flat in int v_instance_id[];
#endif
#ifdef INSTANCING
    flat out int tc_instance_id[];
#endif
#else //HAS_ABS_DISPLACEMENT
layout(vertices = 3) out;

in vec2 v_texcoord0[];
in vec3 v_normal[];
#ifdef INSTANCING
	flat in int v_instance_id[];
#endif

out vec2 tc_texcoord0[];
out vec3 tc_normal[];
#ifdef INSTANCING
	flat out int tc_instance_id[];
#endif
#endif //HAS_ABS_DISPLACEMENT

void CalculatePBB( mat4 mvp_)
{
#ifdef USE_PBB_EXT
	vec4 bbmin;
	vec4 bbmax;
	
	bbmin = mvp_ * gl_in[0].gl_Position;
	
	bbmax = bbmin;
	
	for( int i=1; i<16; i++)
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
#ifdef HAS_ABS_DISPLACEMENT

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
	
	float tess_factor = 1.0; //use fix low-tess factor

	gl_TessLevelInner[0] = tess_factor;
	gl_TessLevelInner[1] = tess_factor;

	gl_TessLevelOuter[0] = tess_factor;
	gl_TessLevelOuter[1] = tess_factor;
	gl_TessLevelOuter[2] = tess_factor;
	gl_TessLevelOuter[3] = tess_factor;

	#ifdef INSTANCING
        tc_instance_id[gl_InvocationID] = v_instance_id[gl_InvocationID];      
	#endif

#else //HAS_ABS_DISPLACEMENT
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tc_texcoord0[gl_InvocationID] = v_texcoord0[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	#ifdef INSTANCING
		tc_instance_id[gl_InvocationID] = v_instance_id[gl_InvocationID]; 
	#endif
			
	float tess_factor = 1.0; //use fix low-tess factor
	
	gl_TessLevelInner[0] = tess_factor;

	gl_TessLevelOuter[0] = tess_factor;
	gl_TessLevelOuter[1] = tess_factor;
	gl_TessLevelOuter[2] = tess_factor;
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

in vec2 tc_texcoord0[];
in vec3 tc_normal[];
#ifdef INSTANCING
	flat in int tc_instance_id[];
#endif
#endif //HAS_ABS_DISPLACEMENT

out vec2 out_texcoord0;
out vec3 out_normal;

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
	out_normal = normalize( cross( bitangent, tangent));
	out_texcoord0 = vec2( 0.0);
#else //HAS_ABS_DISPLACEMENT

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	out_texcoord0 = tc_texcoord0[0] * u + tc_texcoord0[1] * v + tc_texcoord0[2] * w;
	out_normal = tc_normal[0] * u + tc_normal[1] * v + tc_normal[2] * w;
	
	vec3 p2 = gl_in[0].gl_Position.xyz * u + gl_in[1].gl_Position.xyz * v + gl_in[2].gl_Position.xyz * w;
	
	highp float t0_w = texture( texture_unit0, out_texcoord0).w;
	float d = t0_w + tessellation_factor.z;//tessellation_factor: X factor, Y scale, Z bias
	float shadow_tess_inflation_factor = 1.5; //to hide detached shadows - a result of low shadow tess factor
	p2 += out_normal * d * shadow_tess_inflation_factor * tessellation_factor.y * clamp(tessellation_factor.x - 1.0, 0.0, 1.0); //tessellation factor is >1 is active, else 1, can be ~64*/
#endif

	vec4 tmp;
#ifdef INSTANCING
    tmp = vec4( out_normal, 0.0) * instance_data[tc_instance_id[0]].inv_model;
	out_normal = tmp.xyz;
#else
	tmp = vec4( out_normal, 0.0) * inv_model;
	out_normal = tmp.xyz;
#endif

#ifdef INSTANCING
    mat4 instance_mvp = vp *  instance_data[tc_instance_id[0]].model; 
	gl_Position = instance_mvp * vec4( p2, 1.0);
#else
	gl_Position = mvp * vec4( p2, 1.0);
#endif

    gl_Position.z = max(gl_Position.z, -1.0); // "pancaking"
}

#endif // END OF TESS EVAL SHADER

#endif //END OF TESSELLATION PATH

#ifdef TYPE_fragment

in vec2 out_texcoord0;
out vec4 frag_color;

void main()
{			

#ifdef ALPHA_TEST
	if (texture(texture_unit0, out_texcoord0).a < 0.005)
	{
		discard;
	}
#endif

	frag_color = vec4( 1.0);
}

#endif
