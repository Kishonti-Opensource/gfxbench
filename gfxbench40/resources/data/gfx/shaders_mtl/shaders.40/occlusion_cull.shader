/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#include "occlusion_cull_common.h"

#ifdef TYPE_compute

struct MeshMatrix
{
    hfloat4x4 model;
    hfloat4x4 inv_model;
};

struct Mesh
{
    hfloat4 aabb[8];
    hfloat4 draw_params;
};

kernel void shader_main(uint2 globalId							[[thread_position_in_grid]],
						constant OcclusionConstants * consts	[[buffer(0)]],
						const device Mesh * meshdata			[[buffer(1)]],
						const device MeshMatrix * matrixdata	[[buffer(2)]],
						const device uint * mesh_ids			[[buffer(3)]],
						device atomic_uint * commands			[[buffer(4)]],
						device MeshMatrix * instance_matrices	[[buffer(5)]],
#ifdef DRAW_COUNTER_ENABLED
						device atomic_uint * draw_counter		[[buffer(6)]],
#endif
						depth2d<hfloat> hiz_texture				[[texture(0)]])
{
    if (globalId.x < consts->id_count)
    {
        uint mesh_id = mesh_ids[globalId.x];

        if (IsVisible(consts, hiz_texture, meshdata[mesh_id].aabb))
        {
            uint draw_id = uint(meshdata[mesh_id].draw_params.z);

            uint instance_index = atomic_fetch_add_explicit(&commands[draw_id * 5 + 1], 1u, memory_order::memory_order_relaxed);

            // Copy the model and the model inverse matrices
#if USE_MATRIX_STREAM
            // Every thread writes to its own slot
            uint instance_offset = uint(MAX_INSTANCES) * (draw_id - uint(MESH_COUNT)) + uint(meshdata[mesh_id].draw_params.w);

            // Store the frame counter in the bottom-right corner of the matrix
            // This element should be always zero in a any model matrices.
            // This way we indicates that this matrix has been written in this frame
            hfloat4x4 model = matrixdata[mesh_id].model;
            model[3][3] = consts->frame_counter;

            instance_matrices[instance_offset].model = model;
            instance_matrices[instance_offset].inv_model = matrixdata[mesh_id].inv_model;

#else
            // Append the matrix to the end of the queue
            uint instance_offset = uint(MAX_INSTANCES) * (draw_id - uint(MESH_COUNT)) + instance_index;
            instance_matrices[instance_offset] = matrixdata[mesh_id];
#endif

#ifdef DRAW_COUNTER_ENABLED
			atomic_fetch_add_explicit(draw_counter, 1u, memory_order::memory_order_relaxed);
#endif
        }
    }
}

#endif
