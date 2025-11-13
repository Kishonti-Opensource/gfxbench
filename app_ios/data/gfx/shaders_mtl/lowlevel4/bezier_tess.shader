/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "bezier_tess.h"

#ifdef TYPE_vertex

/***********************************************************************************/
//							TESS EVAL
/***********************************************************************************/

struct VertexOut
{
	hfloat4 position [[ position ]];
	v_float3 normal [[ user(NORMAL) ]];
	v_float3 view_dir [[ user(VIEW_DIR) ]];
#ifdef USE_GEOMSHADER
	v_float3 dist;
#endif
	v_float patch_dist [[ user(PATCH_DIST) ]];
	v_float e_patch_wire_scale [[ user(PATCH_WIRE_SCALE) ]];
};


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

_float3 mod289(_float3 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}
_float2 mod289(_float2 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}
_float3 permute(_float3 x)
{
	return mod289(((x*34.0)+1.0)*x);
}
_float snoise(_float2 v)
{
	const _float4 C = _float4(0.211324865405187, // (3.0-sqrt(3.0))/6.0
	0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
	-0.577350269189626, // -1.0 + 2.0 * C.x
	0.024390243902439); // 1.0 / 41.0

	// First corner
	_float2 i = floor(v + dot(v, C.yy) );
	_float2 x0 = v - i + dot(i, C.xx);

	// Other corners
	_float2 i1;

	//i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
	//i1.y = 1.0 - i1.x;
	i1 = (x0.x > x0.y) ? _float2(1.0, 0.0) : _float2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx ;
	// x1 = x0 - i1 + 1.0 * C.xx ;
	// x2 = x0 - 1.0 + 2.0 * C.xx ;

	_float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// Permutations
	i = mod289(i); // Avoid truncation effects in permutation
	_float3 p = permute( permute( i.y + _float3(0.0, i1.y, 1.0 ))
	+ i.x + _float3(0.0, i1.x, 1.0 ));
	_float3 m = max(0.5 - _float3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m*m ;
	m = m*m ;

	// Gradients: 41 points uniformly over a line, mapped onto a diamond.
	// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
	_float3 x = 2.0 * fract(p * C.www) - 1.0;
	_float3 h = abs(x) - 0.5;
	_float3 ox = floor(x + 0.5);
	_float3 a0 = x - ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

	// Compute final noise value at P
	_float3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}
//////////////////////////////////////////////////////////////////////////////////////////
// 		END OF ALU NOISE CODE
//////////////////////////////////////////////////////////////////////////////////////////

[[patch(quad, 16)]]

vertex VertexOut shader_main(
	constant TessUniforms &tu           [[ buffer(0) ]],
	device   PatchInOut   *pi           [[ buffer(1) ]],
             hfloat2       TessCoord    [[ position_in_patch ]],
             ushort        patch_id     [[ patch_id ]]	
)
{
	VertexOut out;

	_float3 position;
	_float3 tangent;
	_float3 bitangent;

	_float u = TessCoord.x;
	_float v = TessCoord.y;

	out.e_patch_wire_scale = pi[patch_id].patch_wire_scale;
	out.patch_dist = max(abs(0.5 - u), abs(0.5 - v));

	_float4 U = _float4( u*u*u, u*u, u, 1.0);
	_float4 V = _float4( v*v*v, v*v, v, 1.0);
	_float4 dU = _float4( 3.0 * u * u, 2.0 * u, 1.0, 0.0);
	_float4 dV = _float4( 3.0 * v * v, 2.0 * v, 1.0, 0.0);

	position.x = dot( U * pi[patch_id].bp.Px, V);
	position.y = dot( U * pi[patch_id].bp.Py, V);
	position.z = dot( U * pi[patch_id].bp.Pz, V);

	tangent.x = dot( dU * pi[patch_id].bp.Px, V);
	tangent.y = dot( dU * pi[patch_id].bp.Py, V);
	tangent.z = dot( dU * pi[patch_id].bp.Pz, V);

	bitangent.x = dot( U * pi[patch_id].bp.Px, dV );
	bitangent.y = dot( U * pi[patch_id].bp.Py, dV );
	bitangent.z = dot( U * pi[patch_id].bp.Pz, dV );

	_float3 normal = normalize( cross( tangent, bitangent));

	_float sum = 0.0;
	const _float weight = 1.0 / 128.0;
	for(_float i=0.0; i<tu.itercount; ++i)
	{
		_float stepsize = i * weight - 0.5; //-0.5...0.5
		sum += snoise(_float2(u * (0.7 + stepsize * 0.3), v * (0.7 + stepsize * 0.3)));
	}
	sum *= weight;

	position += sum * normal * 1.2 * (cos(tu.time) + 1.0) * 0.5;

	out.position = tu.mvp * _float4( position, 1.0);
	out.normal = v_float3(normal);
	out.view_dir = v_float3(position - tu.view_pos);
	
	return out;
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

#if 0
#ifndef USE_GEOMSHADER
#define fs_normal   normal
#define fs_view_dir view_dir
#endif
#endif

struct FragmentInput
{
	v_float3 fs_normal [[ user(NORMAL) ]];
	v_float3 fs_view_dir  [[ user(VIEW_DIR) ]];
#ifdef USE_GEOMSHADER
	v_float3 dist;
#endif
	v_float g_patch_dist [[ user(PATCH_DIST) ]];
	v_float g_patch_wire_scale [[ user(PATCH_WIRE_SCALE) ]];
};


fragment _float4 shader_main(
	FragmentInput input [[ stage_in ]],
	constant TessUniforms &tu [[ buffer(0) ]]
)
{
	const _float3 gamma = _float3( 1.0 / 2.2);
	const _float3 light_dir = _float3( 0.0, 1.0, 0.0);

	_float3 view_dir = -normalize( input.fs_view_dir);

	//winding in mesh!
	_float3 dir3 = -normalize( input.fs_normal);
	_float3 dir5 = reflect(-light_dir, dir3);

	_float diffuse = clamp( dot( dir3, light_dir), _float(0.0), _float(1.0));
	_float specular = clamp( dot( dir5, view_dir), _float(0.0), _float(1.0));

	const _float3 ambicol = _float3( 0.01);
	const _float3 diffcol = _float3( 0.1, 0.2, 0.3);
	_float3 difflight = diffcol * _float3( diffuse) ;

	const _float3 speccol = _float3( 2.0, 1.0, 0.1);
	_float3 speclight = speccol * _float3( powr( specular, _float(16.0)));
	_float3 color = ambicol + difflight + speclight;
#ifdef USE_GEOMSHADER
	{
		color = powr( color, gamma);
		_float nearD = min(min(input.dist.x,input.dist.y),input.dist.z);

		_float edgeIntensity = exp2(-1.0*nearD*nearD);

		_float width = 0.0005 * input.g_patch_wire_scale;
		_float inv_width = 1.0/width;
		_float edgeIntensity_patch = clamp((input.g_patch_dist - (0.5 - width)) * inv_width, 0.0, 1.0);

		_float3 color1 = mix(color, _float3(4.0, 1.5, 0.15), clamp(edgeIntensity * powr(specular, 2.0) * 3.0, 0.0, 1.0));
		_float3 color0 = mix(color, _float3(1.0,0.5,0.05), clamp(edgeIntensity_patch * 20.0, 0.0, 1.0));

		return _float4( color0 * (_float3(0.1) + color1) * (0.2 + powr(1.0 - ((cos(tu.time) + 1.0) * 0.5), 3.0)), 1.0);
	}
#else
	{
		return _float4(color, 1.0);
	}
#endif
}

#endif
