/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "bezier_tess.h"


#ifdef TYPE_compute

/***********************************************************************************/
//							TESS CONTROL
/***********************************************************************************/

void CalculatePBB( _float4x4 mvp_)
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


struct TessFactors
{
	mfloat TessLevelOuter[4];
	mfloat TessLevelInner[2];
};


struct PatchVertices
{
	hfloat4 v[16];
};


bool4 lessThan(_float4 v1, _float4 v2)
{
	return bool4(v1.x < v2.x, v1.y < v2.y, v1.z < v2.z, v1.w < v2.w);
}


kernel void shader_main(
	constant TessUniforms  &tu                    [[ buffer(0) ]],
	device   PatchVertices *p                     [[ buffer(1) ]],
	device   PatchInOut    *patch_out             [[ buffer(2) ]],
	device   TessFactors   *tess_factors          [[ buffer(3) ]],
	         uint          global_invocation_id   [[ thread_position_in_grid ]]
)
{
	uint i = global_invocation_id;
	
	const _float4x4 BT = _float4x4(
		_float4( -1.0, 3.0,-3.0, 1.0),
		_float4(  3.0,-6.0, 3.0, 0.0),
		_float4( -3.0, 3.0, 0.0, 0.0),
		_float4(  1.0, 0.0, 0.0, 0.0)
	);

	CalculatePBB( tu.mvp);

	patch_out[i].bp.Px = _float4x4(
		_float4( p[i].v[0].x,  p[i].v[1].x,  p[i].v[2].x,  p[i].v[3].x),
		_float4( p[i].v[4].x,  p[i].v[5].x,  p[i].v[6].x,  p[i].v[7].x),
		_float4( p[i].v[8].x,  p[i].v[9].x,  p[i].v[10].x, p[i].v[11].x),
		_float4( p[i].v[12].x, p[i].v[13].x, p[i].v[14].x, p[i].v[15].x)
	);

	patch_out[i].bp.Py = _float4x4(
		_float4( p[i].v[0].y,  p[i].v[1].y,  p[i].v[2].y,  p[i].v[3].y),
		_float4( p[i].v[4].y,  p[i].v[5].y,  p[i].v[6].y,  p[i].v[7].y),
		_float4( p[i].v[8].y,  p[i].v[9].y,  p[i].v[10].y, p[i].v[11].y),
		_float4( p[i].v[12].y, p[i].v[13].y, p[i].v[14].y, p[i].v[15].y)
	);

	patch_out[i].bp.Pz = _float4x4(
		_float4( p[i].v[0].z,  p[i].v[1].z,  p[i].v[2].z,  p[i].v[3].z),
		_float4( p[i].v[4].z,  p[i].v[5].z,  p[i].v[6].z,  p[i].v[7].z),
		_float4( p[i].v[8].z,  p[i].v[9].z,  p[i].v[10].z, p[i].v[11].z),
		_float4( p[i].v[12].z, p[i].v[13].z, p[i].v[14].z, p[i].v[15].z)
	);

	patch_out[i].bp.Px = BT *  patch_out[i].bp.Px * BT;
	patch_out[i].bp.Py = BT *  patch_out[i].bp.Py * BT;
	patch_out[i].bp.Pz = BT *  patch_out[i].bp.Pz * BT;


	_float3 v0 = (tu.mv * _float4(p[i].v[0].xyz, 1.0)).xyz;
	_float3 v3 = (tu.mv * _float4(p[i].v[3].xyz, 1.0)).xyz;
	_float3 v12 = (tu.mv * _float4(p[i].v[12].xyz, 1.0)).xyz;
	_float3 v15 = (tu.mv * _float4(p[i].v[15].xyz, 1.0)).xyz;

	//do not tessellate back-facing patches
	{
		_float3 edge0 = v3 - v0;
		_float3 edge2 = v12 - v0;

		_float3 faceNorm = -normalize(cross(edge0, edge2)); //negated because of cw mesh winding

		//NOTE: just an approximation, as the two tris of a quad might not lie on a plane
		//NOTE: low-patch count and high curvature results in post-tess triangles which face the camera even if the original patch did not
		//		so we have to use large offset otherwise gaps would appear
		if(dot(normalize(v0),faceNorm) > 0.8)
		{
			tess_factors[i].TessLevelInner[0] = 0.0;
			tess_factors[i].TessLevelInner[1] = 0.0;
			tess_factors[i].TessLevelOuter[0] = 0.0;
			tess_factors[i].TessLevelOuter[1] = 0.0;
			tess_factors[i].TessLevelOuter[2] = 0.0;
			tess_factors[i].TessLevelOuter[3] = 0.0;
			return;
		}
	}

	_float r0 = distance(v0, v12) * 0.5;
	_float r1 = distance(v12, v15) * 0.5;
	_float r2 = distance(v3, v15) * 0.5;
	_float r3 = distance(v0, v3) * 0.5;

	//do not tessellate patches completely outside the frustum
	{
		_float3 wA = (tu.model * _float4(p[i].v[0].xyz, 1.0)).xyz;
		_float3 wB = (tu.model * _float4(p[i].v[3].xyz, 1.0)).xyz;
		_float3 wC = (tu.model * _float4(p[i].v[12].xyz, 1.0)).xyz;
		_float3 wD = (tu.model * _float4(p[i].v[15].xyz, 1.0)).xyz;

		_float4 centrojd = _float4(_float3(wA + wB + wC + wD) * 0.25, 1.0); //world space
		_float radius = 1.0 * max(r0, max(r1, max(r2, r3))); //this will safely do for radius

		_float4 dists;
		dists.x = dot(tu.frustum_planes[0], centrojd);
		dists.y = dot(tu.frustum_planes[1], centrojd);
		dists.z = dot(tu.frustum_planes[2], centrojd);
		dists.w = dot(tu.frustum_planes[3], centrojd);

		bool4 comp = lessThan(dists, _float4(-radius));

		_float dist_near = dot(tu.frustum_planes[5], centrojd);

		if(any(comp) || (dist_near < -radius))
		{
			tess_factors[i].TessLevelInner[0] = 0.0;
			tess_factors[i].TessLevelInner[1] = 0.0;
			tess_factors[i].TessLevelOuter[0] = 0.0;
			tess_factors[i].TessLevelOuter[1] = 0.0;
			tess_factors[i].TessLevelOuter[2] = 0.0;
			tess_factors[i].TessLevelOuter[3] = 0.0;
			return;
		}
	}

	//compute tess-factor based on screenspace edge-size
	_float3 C0 = (v0 + v12) * 0.5;
	_float3 C1 = (v12 + v15) * 0.5;
	_float3 C2 = (v3 + v15) * 0.5;
	_float3 C3 = (v0 + v3) * 0.5;

	_float screenSize0 = r0 * tu.cam_near_far_pid_vpscale.x / abs(C0.z);
	_float screenSize1 = r1 * tu.cam_near_far_pid_vpscale.x / abs(C1.z);
	_float screenSize2 = r2 * tu.cam_near_far_pid_vpscale.x / abs(C2.z);
	_float screenSize3 = r3 * tu.cam_near_far_pid_vpscale.x / abs(C3.z);

	_float tess_multiplier = 800.0 * tu.cam_near_far_pid_vpscale.w; //default for 1080p, when vpscale is 1.0 + FOV does not change here
	
	//tess_multiplier *= abs(sin(time)) + 1.0;
	
	_float t0 = clamp(screenSize0 * tess_multiplier, 1.0, 64.0);
	_float t1 = clamp(screenSize1 * tess_multiplier, 1.0, 64.0);
	_float t2 = clamp(screenSize2 * tess_multiplier, 1.0, 64.0);
	_float t3 = clamp(screenSize3 * tess_multiplier, 1.0, 64.0);


	//no cracks this way
	_float t_inner0 = (t1 + t3) * 0.5;
	_float t_inner1 = (t0 + t2) * 0.5;

	tess_factors[i].TessLevelInner[0] = t_inner0;
	tess_factors[i].TessLevelInner[1] = t_inner1;

	tess_factors[i].TessLevelOuter[0] = t0;
	tess_factors[i].TessLevelOuter[1] = t3;
	tess_factors[i].TessLevelOuter[2] = t2;
	tess_factors[i].TessLevelOuter[3] = t1;

	patch_out[i].patch_wire_scale = 100.0 / ((t0 + t1 + t2 + t3 + t_inner0 + t_inner1) * 0.16);
}

#endif

