/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform sampler2D specular_texture;
uniform float alpha_test_threshold;
uniform vec4 view_pos;
uniform vec4 view_dir;

uniform sampler2D displacement_tex;
uniform vec4 tessellation_factor; 
uniform float cam_near;
uniform vec4 frustum_planes[6]; 
uniform float tessellation_multiplier; 
uniform mat4 mv; 
uniform mat4 model; 
uniform mat4 inv_model; 
uniform mat4 mvp; 

layout(vertices = 3) out;

in vec4 v_position[];
in vec2 v_texcoord[];
in vec3 v_normal[];
in vec3 v_tangent[];

out vec4 tc_position[];
out vec2 tc_texcoord[];
out vec3 tc_normal[];
out vec3 tc_tangent[];

/*bool IsVisible( mat4 mvp_)
{
    vec4 pos_h[3];
    
    vec3 bb_min;
    vec3 bb_max;

    pos_h[0] = mvp_ * gl_in[0].gl_Position;
    bb_min = bb_max = pos_h[0].xyz / pos_h[0].w;

    vec4 tmp;
    
    for( int i=1; i<3; i++)
	{
        pos_h[i] = mvp_ * gl_in[i].gl_Position;
        tmp.xyz = pos_h[i].xyz / pos_h[i].w;

        bb_min = min(bb_min, tmp.xyz);
        bb_max = max(bb_max, tmp.xyz);
    }
    
    bb_min = bb_min * vec3(0.5) + vec3(0.5);
    bb_max = bb_max * vec3(0.5) + vec3(0.5);
    
    if (!IsVisible(bb_min.xy, bb_max.xy, bb_min.z - 0.00001))
    {  
        return false;
    }

    return true;
}
 
void CalculatePBB( mat4 mvp_)
{

}*/

void main()
{
    /*bool is_visible = IsVisible(mvp);

    if (!is_visible)
    {
        gl_TessLevelInner[0] = 0.0;
        gl_TessLevelInner[1] = 0.0;
		gl_TessLevelOuter[0] = 0.0;
		gl_TessLevelOuter[1] = 0.0;
		gl_TessLevelOuter[2] = 0.0;
		gl_TessLevelOuter[3] = 0.0;
        return;
    }*/

	tc_position[gl_InvocationID] = v_position[gl_InvocationID];
	tc_texcoord[gl_InvocationID] = v_texcoord[gl_InvocationID];
	tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	tc_tangent[gl_InvocationID] = v_tangent[gl_InvocationID];

	if (gl_InvocationID == 0) 
	{
		vec3 vA = (mv * vec4(v_position[0].xyz, 1.0)).xyz;
		vec3 vB = (mv * vec4(v_position[1].xyz, 1.0)).xyz;
		vec3 vC = (mv * vec4(v_position[2].xyz, 1.0)).xyz;
		
		{
			vec3 edge0 = vB - vA;
			vec3 edge2 = vC - vA;
			
			vec3 faceNorm = normalize(cross(edge0, edge2));
			
			if(dot(normalize(vA),faceNorm) > 0.4) 
			{
				//0 kinda means discard?				
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
		
		{
			vec3 wA = (model * vec4(v_position[0].xyz, 1.0)).xyz;
			vec3 wB = (model * vec4(v_position[1].xyz, 1.0)).xyz;
			vec3 wC = (model * vec4(v_position[2].xyz, 1.0)).xyz;		
			
			vec4 centroid_pos = vec4(vec3(wA + wB + wC) * 0.3333, 1.0); 
			float radius = 1.5 * max(r0, max(r1, r2)); 
			
			vec4 dists;
			dists.x = dot(frustum_planes[0], centroid_pos);
			dists.y = dot(frustum_planes[1], centroid_pos);
			dists.z = dot(frustum_planes[2], centroid_pos);
			dists.w = dot(frustum_planes[3], centroid_pos);
					
			bvec4 comp = lessThan(dists, vec4(-radius));
			
			float dist_near = dot(frustum_planes[5], centroid_pos);
			
			if(any(comp) || (dist_near < -radius))
			{
				gl_TessLevelInner[0] = 0.0;
				gl_TessLevelOuter[0] = 0.0;
				gl_TessLevelOuter[1] = 0.0;
				gl_TessLevelOuter[2] = 0.0;
				return;			
			}
		}
		
		vec3 C0 = (vA + vB) * 0.5;
		vec3 C1 = (vB + vC) * 0.5;
		vec3 C2 = (vC + vA) * 0.5;
			
		float screenSize0 = r0 * cam_near / abs(C0.z);
		float screenSize1 = r1 * cam_near / abs(C1.z);
		float screenSize2 = r2 * cam_near / abs(C2.z);
		
		float t0 = clamp(screenSize0 * tessellation_multiplier, 1.0, tessellation_factor.x);
		float t1 = clamp(screenSize1 * tessellation_multiplier, 1.0, tessellation_factor.x);
		float t2 = clamp(screenSize2 * tessellation_multiplier, 1.0, tessellation_factor.x);
		
		float t_inner = max(t1, t2);
		
		gl_TessLevelInner[0] = t_inner;

		gl_TessLevelOuter[0] = t1;
		gl_TessLevelOuter[1] = t2;
		gl_TessLevelOuter[2] = t0;
	}
}
