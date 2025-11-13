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

#ifdef TYPE_compute

#ifdef HAS_ABS_DISPLACEMENT
#define PATCH_VERTEX_COUNT 16
#else //HAS_ABS_DISPLACEMENT
#define PATCH_VERTEX_COUNT 3
#define NO_ABS_DISPLACEMENT 1
#endif //HAS_ABS_DISPLACEMENT


struct InstanceInput
{
	hfloat4x4 model;
	hfloat4x4 inv_model;
};

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
   // tessFactor->InnerLevels[0] = 0.0;
   // tessFactor->OuterLevels[0] = 0.0;
   // tessFactor->OuterLevels[1] = 0.0;
   // tessFactor->OuterLevels[2] = 0.0;

   // #ifdef HAS_ABS_DISPLACEMENT
   //  tessFactor->InnerLevels[1] = 0.0;
   //  tessFactor->OuterLevels[3] = 0.0;
   // #endif
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
	if(thread_position_in_grid * CONTROL_POINTS_PER_THREAD >= controlData.patchCount * 16)
		return;

	const auto real_thread_position_in_threadgroup = thread_position_in_threadgroup & (REAL_THREADGROUP_DIVISOR - 1);
	const auto sub_threadgroup_in_threadgroup = thread_position_in_threadgroup / REAL_THREADGROUP_DIVISOR;
	const ushort patch_id = thread_position_in_grid / REAL_THREADGROUP_DIVISOR;

	//Could save memory here.
    threadgroup hfloat3 position[PATCHES_PER_THREADGROUP * CONTROL_POINTS_PER_PATCH];

    threadgroup hfloat3* patch_position = position + sub_threadgroup_in_threadgroup * CONTROL_POINTS_PER_PATCH;

		for(int o = 0; o < CONTROL_POINTS_PER_THREAD; o++)
		position[thread_position_in_threadgroup * CONTROL_POINTS_PER_THREAD + o] = hfloat3(vertexInput[indexInput[thread_position_in_grid * CONTROL_POINTS_PER_THREAD + o]].in_position);


	threadgroup_barrier(mem_flags::mem_threadgroup);
	
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
	   			// const auto btv = BTLut[lutIndex(resultCol, m, n, resultRow)];
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

#if HAS_INDIRECT_PATCH
	draw_indirect_args.patchCount = controlData.patchCount;
#endif
}
#else
bool user_main(
				const ushort id,
				thread TessFactor* tessFactor,
				thread UserPerPatchData* userPerPatchData,
				const constant VertexUniforms* uniforms,
				const device VertexInput* vertexInput,
				const device ushort* indexInput,
				const constant TessellationUniforms* tessUniforms
				)
{

    _float4 position[PATCH_VERTEX_COUNT];
    for(int i = 0; i < PATCH_VERTEX_COUNT; i++)
    {
    	position[i].xyz = _float3(vertexInput[indexInput[i]].in_position);
    	position[i].w = 1;
    }

#ifdef HAS_ABS_DISPLACEMENT

	userPerPatchData->Px = hfloat4x4 (
		hfloat4(position[0].x, position[1].x, position[2].x, position[3].x),
		hfloat4(position[4].x, position[5].x, position[6].x, position[7].x),
		hfloat4(position[8].x, position[9].x, position[10].x, position[11].x),
		hfloat4(position[12].x, position[13].x, position[14].x, position[15].x)
	);

	userPerPatchData->Py = hfloat4x4(
		hfloat4(position[0].y, position[1].y, position[2].y, position[3].y),
		hfloat4(position[4].y, position[5].y, position[6].y, position[7].y),
		hfloat4(position[8].y, position[9].y, position[10].y, position[11].y),
		hfloat4(position[12].y, position[13].y, position[14].y, position[15].y)
	);

	userPerPatchData->Pz = hfloat4x4(
		hfloat4(position[0].z, position[1].z, position[2].z, position[3].z),
		hfloat4(position[4].z, position[5].z, position[6].z, position[7].z),
		hfloat4(position[8].z, position[9].z, position[10].z, position[11].z),
		hfloat4(position[12].z, position[13].z, position[14].z, position[15].z)
	);

	userPerPatchData->Px = BT * userPerPatchData->Px * BT;
	userPerPatchData->Py = BT * userPerPatchData->Py * BT;
	userPerPatchData->Pz = BT * userPerPatchData->Pz * BT;
	return true;

   // _float tess_factor = 1.0; //use fix low-tess factor

   // tessFactor->InnerLevels[0] = tess_factor;
   // tessFactor->InnerLevels[1] = tess_factor;

   // tessFactor->OuterLevels[0] = tess_factor;
   // tessFactor->OuterLevels[1] = tess_factor;
   // tessFactor->OuterLevels[2] = tess_factor;
   // tessFactor->OuterLevels[3] = tess_factor;

#else //HAS_ABS_DISPLACEMENT
   // _float tess_factor = 1.0; //use fix low-tess factor

   // tessFactor->InnerLevels[0] = tess_factor;

   // tessFactor->OuterLevels[0] = tess_factor;
   // tessFactor->OuterLevels[1] = tess_factor;
   // tessFactor->OuterLevels[2] = tess_factor;
#endif
}

kernel void shader_main(
						const ushort thread_position_in_grid [[thread_position_in_grid]],
						device TessFactor* tessFactors [[buffer(TESS_FACTORS_SLOT)]],
						device UserPerPatchData* userPerPatchData [[buffer(USER_PER_PATCH_SLOT)]],
						const constant ControlData& controlData [[buffer(CONTROL_DATA_SLOT)]],
						const constant VertexUniforms* uniforms [[buffer(VERTEX_UNIFORMS_SLOT)]],
						const device VertexInput* vertexInput [[buffer(VERTEX_BUFFER_SLOT)]],
						const device ushort* indexInput [[buffer(INDEX_BUFFER_SLOT)]],
						const constant TessellationUniforms* tessUniforms [[buffer(TESSELLATION_UNIFORMS_SLOT)]],
						device _MTLDrawPatchIndirectArguments& draw_indirect_args [[buffer(DRAW_PATCH_INDIRECT_SLOT)]],
						device unsigned* patchIndexBuffer [[buffer(COMMAND_IDX_SLOT)]],
						device atomic_uint& appendIndex [[buffer(DRAWS_SLOT)]]
						)

{

	if(thread_position_in_grid >= controlData.patchCount * controlData.instanceCount)
		return;

	TessFactor localTessFactors;
	UserPerPatchData localUserPerPatchData;

	const auto primID = thread_position_in_grid % controlData.patchCount;

	user_main
	(
		primID,
		&localTessFactors,
		&localUserPerPatchData,
		uniforms,
		vertexInput,
		indexInput + PATCH_VERTEX_COUNT * primID, //APPL: '16' should be read in from a buffer
		tessUniforms
	);

#ifdef HAS_ABS_DISPLACEMENT
	userPerPatchData[thread_position_in_grid] = localUserPerPatchData;
#endif

}
#endif

#endif

#else
#ifdef TYPE_compute
kernel void shader_main(){}
#endif
#endif
