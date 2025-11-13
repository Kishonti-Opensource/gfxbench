/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#line 1

struct _bezier_patch
{
	mat4 Px;
	mat4 Py;
	mat4 Pz;
};

uniform mediump float itercount;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform vec4 cam_near_far_pid_vpscale;
uniform highp vec4 frustum_planes[6];
uniform vec2 view_port_size;

uniform vec3 view_pos;
uniform float time;

const mat4 BT = mat4(
	vec4( -1.0, 3.0,-3.0, 1.0),
	vec4(  3.0,-6.0, 3.0, 0.0),
	vec4( -3.0, 3.0, 0.0, 0.0),
	vec4(  1.0, 0.0, 0.0, 0.0)
);

#ifdef TYPE_vertex

/***********************************************************************************/
//							VERTEX
/***********************************************************************************/

in vec3 in_position;

void main()
{
	gl_Position = vec4( in_position, 1.0);
}

#endif


#ifdef TYPE_tess_control

/***********************************************************************************/
//							TESS CONTROL
/***********************************************************************************/

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

		bbmin = min(bbmin, tmp);
		bbmax = max(bbmax, tmp);
	}

	gl_BoundingBoxEXT[0] = bbmin;
	gl_BoundingBoxEXT[1] = bbmax;
#endif
}


layout(vertices = 1) out;

patch out _bezier_patch bp;
patch out float patch_wire_scale;

void main()
{
	CalculatePBB( mvp);

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


	vec3 v0 = (mv * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
	vec3 v3 = (mv * vec4(gl_in[3].gl_Position.xyz, 1.0)).xyz;
	vec3 v12 = (mv * vec4(gl_in[12].gl_Position.xyz, 1.0)).xyz;
	vec3 v15 = (mv * vec4(gl_in[15].gl_Position.xyz, 1.0)).xyz;

	//do not tessellate back-facing patches
	{
		vec3 edge0 = v3 - v0;
		vec3 edge2 = v12 - v0;

		vec3 faceNorm = -normalize(cross(edge0, edge2)); //negated because of cw mesh winding

		//NOTE: just an approximation, as the two tris of a quad might not lie on a plane
		//NOTE: low-patch count and high curvature results in post-tess triangles which face the camera even if the original patch did not
		//		so we have to use large offset otherwise gaps would appear
		if(dot(normalize(v0),faceNorm) > 0.8)
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
	float r1 = distance(v12, v15) * 0.5;
	float r2 = distance(v3, v15) * 0.5;
	float r3 = distance(v0, v3) * 0.5;

	//do not tessellate patches completely outside the frustum
	{
		vec3 wA = (model * vec4(gl_in[0].gl_Position.xyz, 1.0)).xyz;
		vec3 wB = (model * vec4(gl_in[3].gl_Position.xyz, 1.0)).xyz;
		vec3 wC = (model * vec4(gl_in[12].gl_Position.xyz, 1.0)).xyz;
		vec3 wD = (model * vec4(gl_in[15].gl_Position.xyz, 1.0)).xyz;

		vec4 centrojd = vec4(vec3(wA + wB + wC + wD) * 0.25, 1.0); //world space
		float radius = 1.0 * max(r0, max(r1, max(r2, r3))); //this will safely do for radius

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
	vec3 C1 = (v12 + v15) * 0.5;
	vec3 C2 = (v3 + v15) * 0.5;
	vec3 C3 = (v0 + v3) * 0.5;

	float screenSize0 = r0 * cam_near_far_pid_vpscale.x / abs(C0.z);
	float screenSize1 = r1 * cam_near_far_pid_vpscale.x / abs(C1.z);
	float screenSize2 = r2 * cam_near_far_pid_vpscale.x / abs(C2.z);
	float screenSize3 = r3 * cam_near_far_pid_vpscale.x / abs(C3.z);

	float tess_multiplier = 800.0 * cam_near_far_pid_vpscale.w; //default for 1080p, when vpscale is 1.0 + FOV does not change here
	
	//tess_multiplier *= abs(sin(time)) + 1.0;
	
	float t0 = clamp(screenSize0 * tess_multiplier, 1.0, 64.0);
	float t1 = clamp(screenSize1 * tess_multiplier, 1.0, 64.0);
	float t2 = clamp(screenSize2 * tess_multiplier, 1.0, 64.0);
	float t3 = clamp(screenSize3 * tess_multiplier, 1.0, 64.0);


	//no cracks this way
	float t_inner0 = (t1 + t3) * 0.5;
	float t_inner1 = (t0 + t2) * 0.5;

	gl_TessLevelInner[0] = t_inner0;
	gl_TessLevelInner[1] = t_inner1;

	gl_TessLevelOuter[0] = t0;
	gl_TessLevelOuter[1] = t3;
	gl_TessLevelOuter[2] = t2;
	gl_TessLevelOuter[3] = t1;

	patch_wire_scale = 100.0 / ((t0 + t1 + t2 + t3 + t_inner0 + t_inner1) * 0.16);
}

#endif


#ifdef TYPE_tess_eval

/***********************************************************************************/
//							TESS EVAL
/***********************************************************************************/

layout(quads, fractional_odd_spacing, cw) in;

patch in _bezier_patch bp;
patch in float patch_wire_scale;

out vec3 normal;
out vec3 view_dir;
out float patch_dist;
out float e_patch_wire_scale;


//////////////////////////////////////////////////////////////////////////////////////////
// 		BEGINNING OF ALU NOISE CODE, FROM: https://github.com/ashima/webgl-noise/
//////////////////////////////////////////////////////////////////////////////////////////
/*
Copyright (C) 2011 by Ashima Arts (Simplex noise)
Copyright (C) 2011 by Stefan Gustavson (Classic noise)
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

vec3 mod289(vec3 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 permute(vec3 x)
{
	return mod289(((x*34.0)+1.0)*x);
}
float snoise(vec2 v)
{
	const vec4 C = vec4(0.211324865405187, // (3.0-sqrt(3.0))/6.0
	0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
	-0.577350269189626, // -1.0 + 2.0 * C.x
	0.024390243902439); // 1.0 / 41.0

	// First corner
	vec2 i = floor(v + dot(v, C.yy) );
	vec2 x0 = v - i + dot(i, C.xx);

	// Other corners
	vec2 i1;

	//i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
	//i1.y = 1.0 - i1.x;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx ;
	// x1 = x0 - i1 + 1.0 * C.xx ;
	// x2 = x0 - 1.0 + 2.0 * C.xx ;

	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
	+ i.x + vec3(0.0, i1.x, 1.0 ));
	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m*m ;
	m = m*m ;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

	// Compute final noise value at P
	vec3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}
//////////////////////////////////////////////////////////////////////////////////////////
// 		END OF ALU NOISE CODE
//////////////////////////////////////////////////////////////////////////////////////////

void main()
{
	vec3 position;
	vec3 tangent;
	vec3 bitangent;

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	e_patch_wire_scale = patch_wire_scale;
	patch_dist = max(abs(0.5 - u), abs(0.5 - v));

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

	normal = normalize( cross( tangent, bitangent));

	float sum = 0.0;
	const float weight = 1.0 / 128.0;
	for(float i=0.0; i<itercount; ++i)
	{
		float stepsize = i * weight - 0.5; //-0.5...0.5
		sum += snoise(vec2(u * (0.7 + stepsize * 0.3), v * (0.7 + stepsize * 0.3)));
	}
	sum *= weight;

	position += sum * normal * 1.2 * (cos(time) + 1.0) * 0.5;

	gl_Position = mvp * vec4( position, 1.0);

	view_dir = position - view_pos;
}

#endif

#ifdef TYPE_geometry

/***********************************************************************************/
//							GEOMETRY
/***********************************************************************************/

in vec3 normal[];
in vec3 view_dir[];
in float patch_dist[];
in float e_patch_wire_scale[];

out vec3 fs_normal;
out vec3 fs_view_dir;
 out float g_patch_wire_scale;
 out vec3 dist;
 out float g_patch_dist;

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
void main()
{
	//screen-size based metric for the wire width
	vec2 scale = vec2(view_port_size.x, view_port_size.y);

	vec2 p0 = scale * gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
	vec2 p1 = scale * gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;
	vec2 p2 = scale * gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w;

	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;
	float area = abs(v1.x*v2.y - v1.y * v2.x);

	gl_Position = gl_in[0].gl_Position;
	g_patch_wire_scale = e_patch_wire_scale[0];
	dist = vec3(area/length(v0),0,0);
	g_patch_dist = patch_dist[0];
	fs_normal = normal[0];
	fs_view_dir = view_dir[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	g_patch_wire_scale = e_patch_wire_scale[1];
	dist = vec3(0,area/length(v1),0);
	g_patch_dist = patch_dist[1];
	fs_normal = normal[1];
	fs_view_dir = view_dir[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	g_patch_wire_scale = e_patch_wire_scale[2];
	dist = vec3(0,0,area/length(v2));
	g_patch_dist = patch_dist[2];;
	fs_normal = normal[2];
	fs_view_dir = view_dir[2];
	EmitVertex();

	EndPrimitive();

}

#endif

#ifdef TYPE_fragment

/***********************************************************************************/
//							FRAGMENT
/***********************************************************************************/

uniform samplerCube env_map;
uniform vec2 hdr_params;

#ifndef USE_GEOMSHADER
#define fs_normal   normal
#define fs_view_dir view_dir
#endif

in vec3 fs_normal;
in vec3 fs_view_dir;
in vec3 dist;
in float g_patch_dist;
in float g_patch_wire_scale;

out vec4 frag_color;


void main()
{
	const vec3 gamma = vec3( 1.0 / 2.2);
	const vec3 light_dir = vec3( 0.0, 1.0, 0.0);

	vec3 view_dir = -normalize( fs_view_dir);

	//winding in mesh!
	vec3 dir3 = -normalize( fs_normal);
	vec3 dir5 = reflect(-light_dir, dir3);

	float diffuse = clamp( dot( dir3, light_dir), 0.0, 1.0);
	float specular = clamp( dot( dir5, view_dir), 0.0, 1.0);

	const vec3 ambicol = vec3( 0.01);
	const vec3 diffcol = vec3( 0.1, 0.2, 0.3);
	vec3 difflight = diffcol * vec3( diffuse) ;

	const vec3 speccol = vec3( 2.0, 1.0, 0.1);
	vec3 speclight = speccol * vec3( pow( specular, 16.0));
	vec3 color = ambicol + difflight + speclight;
#ifdef USE_GEOMSHADER
	{
		color = pow( color, gamma);
		float nearD = min(min(dist.x,dist.y),dist.z);

		float edgeIntensity = exp2(-1.0*nearD*nearD);

		float width = 0.0005 * g_patch_wire_scale;
		float inv_width = 1.0/width;
		float edgeIntensity_patch = clamp((g_patch_dist - (0.5 - width)) * inv_width, 0.0, 1.0);

		vec3 color1 = mix(color, vec3(4.0, 1.5, 0.15), clamp(edgeIntensity * pow(specular, 2.0) * 3.0, 0.0, 1.0));
		vec3 color0 = mix(color, vec3(1.0,0.5,0.05), clamp(edgeIntensity_patch * 20.0, 0.0, 1.0));

		frag_color = vec4( color0 * (vec3(0.1) + color1) * (0.2 + pow(1.0 - ((cos(time) + 1.0) * 0.5), 3.0)), 1.0);
	}
#else
	{
		frag_color = vec4(color, 1.0);
	}
#endif
}

#endif
