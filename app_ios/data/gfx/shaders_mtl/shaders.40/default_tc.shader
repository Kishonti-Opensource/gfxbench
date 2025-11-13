/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef USE_TESSELLATION

//DEFAULT SHADER FOR OPAQUE MESHES
//	- can use tessellation
//	- can use local 8-bit displacement maps
//	- can use bezier tessellation
//	- renders opaques meshes to G-BUFFER
//		- but can render transparent meshes directly to LIGHT COMBINE target
//	- shall not write to dynamic cube target at runtime

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

/***********************************************************************************/
//							TESS CONTROL
/***********************************************************************************/

#ifdef TYPE_compute

#ifdef HAS_ABS_DISPLACEMENT
#define PATCH_VERTEX_COUNT 16
#else //HAS_ABS_DISPLACEMENT
#define PATCH_VERTEX_COUNT 3
#define NO_ABS_DISPLACEMENT 1
#endif //HAS_ABS_DISPLACEMENT


#ifdef OCCLUSION_CULL_TESSELLATED
//T should be a float4 in some address space.
bool IsVisible(hfloat4x4 mvp_, thread _float4* positions, constant OcclusionConstants& occUniforms, depth2d<hfloat> hiz_texture)
{
    // Homogeneous positions
    hfloat4 pos_h[PATCH_VERTEX_COUNT];

    // Bounding box in NDC for occlusion culling
    hfloat3 bb_min;
    hfloat3 bb_max;

    pos_h[0] = mvp_ * hfloat4(positions[0]);
    bb_min = bb_max = _float3(pos_h[0].xyz / pos_h[0].w);

    hfloat4 tmp;
    // Calculate the bounding box in NDC
    for( int i=1; i<PATCH_VERTEX_COUNT; i++)
	{
        pos_h[i] = mvp_ * hfloat4(positions[i]);
        tmp.xyz = pos_h[i].xyz / pos_h[i].w;

        bb_min = min(bb_min, _float3(tmp.xyz));
        bb_max = max(bb_max, _float3(tmp.xyz));
    }

    // [-1..1] -> [0...1] Screen-space for occlusion culling
    bb_min.xy = bb_min.xy * _float2(0.5) + _float2(0.5);
    bb_max.xy = bb_max.xy * _float2(0.5) + _float2(0.5);

#if 1
    // Execute HiZ occlusion culling. Add a small bias to the depth to avoid visible leaks
    if (!IsVisible(&occUniforms, hiz_texture, bb_min.xy, bb_max.xy, bb_min.z - 0.00001))
    {
        // The patch is not visible, early out
        return false;
    }
#endif
    return true;
}
#else
#define IsVisible(...) true
#endif


struct VertexInput
{
	packed_float3 in_position;
	packed_char4 in_normal;
	packed_char4 in_tangent;
	packed_float4 in_tesscoord;
};

static_assert(sizeof(VertexInput) == 36, "VertexInput does not have required alignment");

void clearTessellationLevels(thread TessFactor* tessFactor)
{
   tessFactor->InnerLevels(0) = 0.0;
   tessFactor->OuterLevels(0) = 0.0;
   tessFactor->OuterLevels(1) = 0.0;
   tessFactor->OuterLevels(2) = 0.0;

#ifdef HAS_ABS_DISPLACEMENT
   tessFactor->InnerLevels(1) = 0.0;
   tessFactor->OuterLevels(3) = 0.0;
#endif
}

ushort lutIndex(ushort x, ushort y, ushort z, ushort w)
{
	return (x * 4 + y) * 16 + z * 4 + w;
}

bool user_main(
				const ushort id,
				thread TessFactor* tessFactor,
				thread UserPerPatchData* userPerPatchData,
				const constant VertexUniforms* uniforms,
				const device VertexInput* vertexInput,
				const device ushort* indexInput,
				const constant TessellationUniforms* tessUniforms,
				constant OcclusionConstants& occUniforms [[buffer(OCCLUSION_UNIFORMS_SLOT)]],
				depth2d<hfloat> hiz_texture [[texture(HIZ_TEXTURE_SLOT)]],
				device InstanceData* instance_data
				)
{
    _float4 position[PATCH_VERTEX_COUNT];
    for(int i = 0; i < PATCH_VERTEX_COUNT; i++)
    {
    	position[i].xyz = _float3(vertexInput[indexInput[i]].in_position);
    	position[i].w = 1;
    }

// Calculate PBB during occlusion culling
#ifdef INSTANCING
	auto instance_mvp = uniforms->vp *  instance_data[id].model;
    bool is_visible = IsVisible(instance_mvp, position, occUniforms, hiz_texture);
#else
     bool is_visible = IsVisible(uniforms->mvp, position, occUniforms, hiz_texture);
#endif

    if (!is_visible)
    {
        // Covered by occluder
		clearTessellationLevels(tessFactor);
        return false;
    }

#ifdef HAS_ABS_DISPLACEMENT
#elif defined(NO_ABS_DISPLACEMENT)

	//[APPL] Not needed, since they're just copying I can use base_instance (e.g patch_id) * 3 to get the right data from the
	//existing vertex information (so no need to copy!)

	// gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	// tc_texcoord0[gl_InvocationID] = v_texcoord0[gl_InvocationID];
	// tc_texcoord1[gl_InvocationID] = v_texcoord1[gl_InvocationID];
	// tc_normal[gl_InvocationID] = v_normal[gl_InvocationID];
	// tc_tangent[gl_InvocationID] = v_tangent[gl_InvocationID];

	auto mv_ = uniforms->mv;
	#ifdef INSTANCING
		mv_ = uniforms->view * instance_data[id].model;
	#endif

	_float3 vA = _float3((mv_ * hfloat4(position[0])).xyz);
	_float3 vB = _float3((mv_ * hfloat4(position[1])).xyz);
	_float3 vC = _float3((mv_ * hfloat4(position[2])).xyz);

	//do not tessellate back-faces
	{
		_float3 edge0 = vB - vA;
		_float3 edge2 = vC - vA;

		_float3 faceNorm = normalize(cross(edge0, edge2));

		if(dot(normalize(vA),faceNorm) > 0.4) //have to use large offset otherwise gaps would appear
		{
			clearTessellationLevels(tessFactor);
			return false;
		}
	}

    _float r0 = distance(vA, vB);
	_float r1 = distance(vB, vC);
	_float r2 = distance(vC, vA);

	//do not tessellate patches outside the frustum
	{
		//0..3: sides, 4: far, 5: near

		#ifdef INSTANCING
			auto wA = hfloat3((instance_data[id].model * hfloat4(position[0])).xyz);
			auto wB = hfloat3((instance_data[id].model * hfloat4(position[1])).xyz);
			auto wC = hfloat3((instance_data[id].model * hfloat4(position[2])).xyz);
		#else
			auto wA = hfloat3((uniforms->model * hfloat4(position[0])).xyz);
			auto wB = hfloat3((uniforms->model * hfloat4(position[1])).xyz);
			auto wC = hfloat3((uniforms->model * hfloat4(position[2])).xyz);
		#endif

		hfloat4 centrojd = hfloat4(hfloat3(wA + wB + wC) * 0.3333, 1.0); //world space
		hfloat radius = 0.75 * max(r0, max(r1, r2)); //this will safely do for radius

		hfloat4 dists;
		dists.x = dot(hfloat4(tessUniforms->frustum_planes[0]), centrojd);
		dists.y = dot(hfloat4(tessUniforms->frustum_planes[1]), centrojd);
		dists.z = dot(hfloat4(tessUniforms->frustum_planes[2]), centrojd);
		dists.w = dot(hfloat4(tessUniforms->frustum_planes[3]), centrojd);

		bool4 comp = dists < hfloat4(-radius);

		hfloat dist_near = dot(tessUniforms->frustum_planes[5], hfloat4(centrojd));

		if(any(comp) || (dist_near < -radius))
		{
			clearTessellationLevels(tessFactor);
			return false;
		}

/*
		//alternative method: just check vertices in NDC - has artifacts if no verts are visible but triangle still is
		_float4 proj0 = mvp * _float4(gl_in[0].gl_Position.xyz, 1.0);
		_float4 proj1 = mvp * _float4(gl_in[1].gl_Position.xyz, 1.0);
		_float4 proj2 = mvp * _float4(gl_in[2].gl_Position.xyz, 1.0);

		_float3 aproj0 = abs(proj0).xyz;
		_float3 aproj1 = abs(proj1).xyz;
		_float3 aproj2 = abs(proj2).xyz;

		_float3 acomp = abs(_float3(proj0.w, proj1.w, proj2.w));

		bool3 comp0 = greaterThanEqual(aproj0, acomp.xxx);
		bool3 comp1 = greaterThanEqual(aproj1, acomp.yyy);
		bool3 comp2 = greaterThanEqual(aproj2, acomp.zzz);

		if(any(comp0) && any(comp1) && any(comp2))
		{
           tessFactor->InnerLevels(0) = 0.0;
           tessFactor->OuterLevels(0) = 0.0;
           tessFactor->OuterLevels(1) = 0.0;
           tessFactor->OuterLevels(2) = 0.0;
			return;
		}
*/
	}

	//compute tess-factor based on screenspace edge-size
	_float C0 = (vA.z + vB.z);
	_float C1 = (vB.z + vC.z);
	_float C2 = (vC.z + vA.z);

	_float screenSize0 = r0 * uniforms->cam_near_far_pid_vpscale.x / abs(C0);
	_float screenSize1 = r1 * uniforms->cam_near_far_pid_vpscale.x / abs(C1);
	_float screenSize2 = r2 * uniforms->cam_near_far_pid_vpscale.x / abs(C2);

	//tc_patch_size[gl_InvocationID] = max(max(screenSize0, screenSize1), screenSize2)  * 10.0;
	mfloat t0 = clamp(screenSize0 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);
	mfloat t1 = clamp(screenSize1 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);
	mfloat t2 = clamp(screenSize2 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);

    mfloat t_inner = max(t1, t2);

   tessFactor->InnerLevels(0) = t_inner;

   tessFactor->OuterLevels(0) = t1;
   tessFactor->OuterLevels(1) = t2;
   tessFactor->OuterLevels(2) = t0;
   return true;

#else
#error Unknown mode
#endif
}

#ifdef HAS_ABS_DISPLACEMENT

#define CONTROL_POINTS_PER_PATCH 16

#ifndef CONTROL_POINTS_PER_THREAD
#define CONTROL_POINTS_PER_THREAD 1
#endif

#ifndef THREADS_PER_THREADGROUP
#define THREADS_PER_THREADGROUP 32
#endif

#define PATCHES_PER_THREADGROUP ((THREADS_PER_THREADGROUP * CONTROL_POINTS_PER_THREAD) / CONTROL_POINTS_PER_PATCH)
#define REAL_THREADGROUP_DIVISOR (CONTROL_POINTS_PER_PATCH / CONTROL_POINTS_PER_THREAD)


constant hfloat4x4 BT(
	   hfloat4( -1.0, 3.0,-3.0, 1.0),
	   hfloat4(  3.0,-6.0, 3.0, 0.0),
	   hfloat4( -3.0, 3.0, 0.0, 0.0),
	   hfloat4(  1.0, 0.0, 0.0, 0.0)
);

// #undef HAS_INDIRECT_PATCH
kernel void shader_main(
						const uint thread_position_in_grid [[thread_position_in_grid]],
						const ushort thread_position_in_threadgroup [[thread_position_in_threadgroup]],
						const ushort threadgroup_position_in_grid [[threadgroup_position_in_grid]],
						device TessFactor* tessFactors [[buffer(TESS_FACTORS_SLOT)]],
						device UserPerPatchData* userPerPatchData [[buffer(USER_PER_PATCH_SLOT)]],
						const device ControlData& controlData [[buffer(CONTROL_DATA_SLOT)]],
						const constant VertexUniforms* uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]],
						const device VertexInput* vertexInput [[buffer(VERTEX_BUFFER_SLOT)]],
						const device ushort* indexInput [[buffer(INDEX_BUFFER_SLOT)]],
						const constant TessellationUniforms* tessUniforms [[buffer(TESSELLATION_UNIFORMS_SLOT)]],
						constant OcclusionConstants& occUniforms [[buffer(OCCLUSION_UNIFORMS_SLOT)]],
						depth2d<hfloat> hiz_texture [[texture(HIZ_TEXTURE_SLOT)]],
						device InstanceData* instance_data [[buffer(INSTANCE_UNIFORMS_SLOT)]],
						device _MTLDrawPatchIndirectArguments& draw_indirect_args [[buffer(DRAW_PATCH_INDIRECT_SLOT)]],
						device unsigned* patchIndexBuffer [[buffer(COMMAND_IDX_SLOT)]]
						)
{
	if(thread_position_in_grid * CONTROL_POINTS_PER_THREAD >= controlData.patchCount * CONTROL_POINTS_PER_PATCH)
		return;

	const auto real_thread_position_in_threadgroup = thread_position_in_threadgroup & (REAL_THREADGROUP_DIVISOR - 1);
	const auto sub_threadgroup_in_threadgroup = thread_position_in_threadgroup / REAL_THREADGROUP_DIVISOR;
	auto patch_id = thread_position_in_grid / REAL_THREADGROUP_DIVISOR;

	//Could save memory here.
    threadgroup hfloat4 position[PATCHES_PER_THREADGROUP * CONTROL_POINTS_PER_PATCH];
    threadgroup hfloat3 pos_screen[PATCHES_PER_THREADGROUP * CONTROL_POINTS_PER_PATCH];
    threadgroup uint earlyOut[PATCHES_PER_THREADGROUP];

    threadgroup hfloat4* patch_position = position + sub_threadgroup_in_threadgroup * CONTROL_POINTS_PER_PATCH;

  for(int o = 0; o < CONTROL_POINTS_PER_THREAD; o++)
  {
  	position[thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o].xyz = hfloat3(vertexInput[indexInput[thread_position_in_grid * CONTROL_POINTS_PER_THREAD + o]].in_position);
  	position[thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o].w = 1;
  }


  for(int o = 0; o < CONTROL_POINTS_PER_THREAD; o++)
  {
  	const auto _pscreen = uniforms->mvp * position[thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o];
  	pos_screen[thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o].xyz = (_pscreen.xyz / _pscreen.w);
  }

	//if(real_thread_position_in_threadgroup == 0)
	{

		const auto startOffset = sub_threadgroup_in_threadgroup * CONTROL_POINTS_PER_PATCH;
		const auto firstRead = startOffset + (real_thread_position_in_threadgroup & 0x7) * 2;
		hfloat3 pos_originals[] = {
			pos_screen[firstRead],
			pos_screen[firstRead + 1]
		};

		pos_screen[startOffset + thread_position_in_threadgroup & 0x3] = min(
			pos_originals[0], 
			pos_originals[1]
			);

		pos_screen[startOffset + thread_position_in_threadgroup & 0x1] = min(
			pos_screen[startOffset + (thread_position_in_threadgroup & 0x1) * 2],
			pos_screen[startOffset + (thread_position_in_threadgroup & 0x1) * 2 + 1]
			);

		hfloat2 bb_min = min(
			pos_screen[startOffset].xy, 
			pos_screen[startOffset + 1].xy
			);

		hfloat zmin = min(
			pos_screen[startOffset].z,
			pos_screen[startOffset + 1].z
			);

		pos_screen[startOffset + thread_position_in_threadgroup & 0x3].xy = max(
			pos_originals[0].xy, 
			pos_originals[1].xy
			);

		pos_screen[startOffset + thread_position_in_threadgroup & 0x1].xy = max(
			pos_screen[startOffset + (thread_position_in_threadgroup & 0x1) * 2].xy,
			pos_screen[startOffset + (thread_position_in_threadgroup & 0x1) * 2 + 1].xy
			);

		hfloat2 bb_max = max(
			pos_screen[startOffset].xy, 
			pos_screen[startOffset + 1].xy
			);

	    // Bounding box in NDC for occlusion culling
	    // [-1..1] -> [0...1] Screen-space for occlusion culling
	    bb_min.xy = bb_min.xy * 0.5 + 0.5;
	    bb_max.xy = bb_max.xy * 0.5 + 0.5;

	    earlyOut[sub_threadgroup_in_threadgroup] = !IsVisible(&occUniforms, hiz_texture, _float2(bb_min.xy), _float2(bb_max.xy), zmin - 0.00001);
	}

    if (earlyOut[sub_threadgroup_in_threadgroup])
    {
    	// The patch is not visible, early out
    	#if !HAS_INDIRECT_PATCH
    	//if we're not using draw indirect, we still need to clear the factors.
		((device mfloat*)(tessFactors + patch_id))[thread_position_in_threadgroup & 3] = 0;
		#endif
    	return;
    }

    auto v0 = (uniforms->mv * patch_position[0]).xyz;
	auto v3 = (uniforms->mv * patch_position[3]).xyz;
	auto v12 = (uniforms->mv * patch_position[12]).xyz;
	auto v15 = (uniforms->mv * patch_position[15]).xyz;

	//do not tessellate back-faces
	// if(real_thread_position_in_threadgroup == 0)
	{
		_float3 edge0 = _float3(v3 - v0);
		_float3 edge2 = _float3(v12 - v0);

		_float3 faceNorm = _float3(normalize(cross(edge0, edge2)));

		//NOTE: just an approximation, as the two tris of a quad might not lie on a plane
		//have to use large offset otherwise gaps would appear
		earlyOut[sub_threadgroup_in_threadgroup] = dot(_float3(normalize(v0)),faceNorm) > 0.15;
	}

	if (earlyOut[sub_threadgroup_in_threadgroup])
    {
    	// The patch is not visible, early out
    	#if !HAS_INDIRECT_PATCH
    	//if we're not using draw indirect, we still need to clear the factors.
		((device mfloat*)(tessFactors + patch_id))[thread_position_in_threadgroup & 3] = 0;
		#endif
    	return;
    }

    // #define r0 pos_screen[8 + sub_threadgroup_in_threadgroup * 16].x
    // #define r1 pos_screen[8 + sub_threadgroup_in_threadgroup * 16].y
    // #define r2 pos_screen[8 + sub_threadgroup_in_threadgroup * 16].z
    // #define r3 pos_screen[9 + sub_threadgroup_in_threadgroup * 16].x

	auto r0 = distance(v0, v12);
	auto r1 = distance(v0, v3);
	auto r2 = distance(v3, v15);
	auto r3 = distance(v12, v15);

	//do not tessellate patches outside the frustum
	// if(real_thread_position_in_threadgroup == 0)
	{

        auto wA = (uniforms->model * patch_position[0]).xyz;
        auto wB = (uniforms->model * patch_position[3]).xyz;
        auto wC = (uniforms->model * patch_position[12]).xyz;
        auto wD = (uniforms->model * patch_position[15]).xyz;

		hfloat4 centrojd = hfloat4(hfloat3(wA + wB + wC + wD) * 0.25, 1.0); //world space
		hfloat radius = 0.75 * max(r0, max(r1, max(r2, r3))); //this will safely do for radius

		hfloat4 dists;
		dists.x = dot(hfloat4(tessUniforms->frustum_planes[0]), centrojd);
		dists.y = dot(hfloat4(tessUniforms->frustum_planes[1]), centrojd);
		dists.z = dot(hfloat4(tessUniforms->frustum_planes[2]), centrojd);
		dists.w = dot(hfloat4(tessUniforms->frustum_planes[3]), centrojd);

		bool4 comp = dists < hfloat4(-radius);

		hfloat dist_near = dot(hfloat4(tessUniforms->frustum_planes[5]), centrojd);

		earlyOut[sub_threadgroup_in_threadgroup] = any(comp) || (dist_near < -radius);
	}

	if (earlyOut[sub_threadgroup_in_threadgroup])
    {
    	// The patch is not visible, early out
    	#if !HAS_INDIRECT_PATCH
    	//if we're not using draw indirect, we still need to clear the factors.
		((device mfloat*)(tessFactors + patch_id))[thread_position_in_threadgroup & 3] = 0;
		#endif
    	return;
    }

	//compute tess-factor based on screenspace edge-size

    _float C0 = (v0.z + v12.z);
	_float C1 = (v0.z + v3.z);
	_float C2 = (v3.z + v15.z);
	_float C3 = (v12.z + v15.z);

	_float screenSize0 = r0 * uniforms->cam_near_far_pid_vpscale.x / abs(C0);
	_float screenSize1 = r1 * uniforms->cam_near_far_pid_vpscale.x / abs(C1);
	_float screenSize2 = r2 * uniforms->cam_near_far_pid_vpscale.x / abs(C2);
	_float screenSize3 = r3 * uniforms->cam_near_far_pid_vpscale.x / abs(C3);

	mfloat t0 = clamp(screenSize0 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);
	mfloat t1 = clamp(screenSize1 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);
	mfloat t2 = clamp(screenSize2 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);
	mfloat t3 = clamp(screenSize3 * tessUniforms->tessellation_multiplier, 1.0, tessUniforms->tessellation_factor.x);

  mfloat t_inner0 = max(t1, t3);
  mfloat t_inner1 = max(t0, t2);

#if HAS_INDIRECT_PATCH
    if(real_thread_position_in_threadgroup == 0)
    {
   		auto patchCount = (volatile device atomic_uint*)&draw_indirect_args.patchCount;
	   	earlyOut[sub_threadgroup_in_threadgroup] = atomic_fetch_add_explicit(patchCount, 1, memory_order_relaxed);
	}
  threadgroup_barrier(mem_flags::mem_none);
	patch_id = earlyOut[sub_threadgroup_in_threadgroup];
#else
	// draw_indirect_args.patchCount = controlData.patchCount;
#endif

   tessFactors[patch_id].InnerLevels(0) = t_inner0;
   tessFactors[patch_id].InnerLevels(1) = t_inner1;

   tessFactors[patch_id].OuterLevels(0) = t0;
   tessFactors[patch_id].OuterLevels(1) = t1;
   tessFactors[patch_id].OuterLevels(2) = t2;
   tessFactors[patch_id].OuterLevels(3) = t3;

   device hfloat* px = (device hfloat*)(&(userPerPatchData + patch_id)->Px);
   device hfloat* py = (device hfloat*)(&(userPerPatchData + patch_id)->Py);
   device hfloat* pz = (device hfloat*)(&(userPerPatchData + patch_id)->Pz);

   for(int o = 0; o < CONTROL_POINTS_PER_THREAD; o++)
   {
       const auto resultRow = (real_thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o) / 4;
       const auto resultCol = (real_thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o) % 4;

       hfloat3 r = hfloat3(0,0,0);

       for(int m = 0; m < 4; m++)
       {
       		for(int n = 0; n < 4; n++)
       		{
       			const auto posIdx = m * 4 + n;
       			const auto btv = BT[m][resultCol] * BT[resultRow][n];

       			r.x += btv * patch_position[posIdx].x;
       			r.y += btv * patch_position[posIdx].y;
       			r.z += btv * patch_position[posIdx].z;
       		}
       }

      px[resultCol * 4 + resultRow] = r.x;
      py[resultCol * 4 + resultRow] = r.y;
      pz[resultCol * 4 + resultRow] = r.z;
   }



}
#else
kernel void shader_main(
						const ushort thread_position_in_grid [[thread_position_in_grid]],
						device TessFactor* tessFactors [[buffer(TESS_FACTORS_SLOT)]],
						device UserPerPatchData* userPerPatchData [[buffer(USER_PER_PATCH_SLOT)]],
						const device ControlData& controlData [[buffer(CONTROL_DATA_SLOT)]],
						const constant VertexUniforms* uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]],
						const device VertexInput* vertexInput [[buffer(VERTEX_BUFFER_SLOT)]],
						const device ushort* indexInput [[buffer(INDEX_BUFFER_SLOT)]],
						const constant TessellationUniforms* tessUniforms [[buffer(TESSELLATION_UNIFORMS_SLOT)]],
						constant OcclusionConstants& occUniforms [[buffer(OCCLUSION_UNIFORMS_SLOT)]],
						depth2d<hfloat> hiz_texture [[texture(HIZ_TEXTURE_SLOT)]],
						device InstanceData* instance_data [[buffer(INSTANCE_UNIFORMS_SLOT)]],
						device _MTLDrawPatchIndirectArguments& draw_indirect_args [[buffer(DRAW_PATCH_INDIRECT_SLOT)]],
						device unsigned* patchIndexBuffer [[buffer(COMMAND_IDX_SLOT)]]
						)

{
	TessFactor localTessFactors = {};

	if(thread_position_in_grid < controlData.patchCount * controlData.instanceCount)
	{
		UserPerPatchData localUserPerPatchData;


		const auto primID = thread_position_in_grid % controlData.patchCount;
		const auto instanceID = thread_position_in_grid / controlData.patchCount;
		auto r = user_main
		(
			instanceID,
			&localTessFactors,
			&localUserPerPatchData,
			uniforms,
			vertexInput,
			indexInput + PATCH_VERTEX_COUNT * primID,
			tessUniforms,
			occUniforms,
			hiz_texture,
			instance_data
		);

		#if HAS_INDIRECT_PATCH && !defined(INSTANCING)
		if(!r)
			return;

			auto patchCountAtomic = (volatile device atomic_uint*)&draw_indirect_args.patchCount;
			auto i = atomic_fetch_add_explicit(patchCountAtomic, 1, memory_order_relaxed);

			tessFactors[i] = localTessFactors;

			#ifdef HAS_ABS_DISPLACEMENT
				userPerPatchData[i] = localUserPerPatchData;
			#else
				patchIndexBuffer[i] = primID;
			#endif

		#else


		tessFactors[thread_position_in_grid] = localTessFactors;

	#ifdef HAS_ABS_DISPLACEMENT
		userPerPatchData[thread_position_in_grid] = localUserPerPatchData;
	#endif
		#endif
	}
}
#endif
#endif // END OF TESS CTRL SHADER
#else
#ifdef TYPE_compute
kernel void shader_main() {} //Dummy shader so the pipeline loader doesnt' complain
#endif
#endif // USE_TESSELLATION
