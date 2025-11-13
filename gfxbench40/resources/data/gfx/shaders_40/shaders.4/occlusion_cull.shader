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
    mat4 model;
    mat4 inv_model;
};

struct Mesh
{
    vec4 aabb[8];
    vec4 draw_params;
};

/*
struct DrawElementsIndirectCommand
{
    uint count; //0
    uint instanceCount; //1
    uint firstIndex;
    int baseVertex;
    uint reservedMustBeZero;
};
*/

// Bounding and draw data
layout(std430, binding = 0) readonly buffer mesh_buffer_layout
{
    MeshMatrix matrices[STATIC_MESH_COUNT];
    Mesh meshes[];
};

// Task ids
layout(std430, binding = 1) readonly buffer id_buffer_layout
{
    uint mesh_ids[];
};

// Indirect draw commands
layout(std430, binding = 2) buffer command_buffer_layout
{
    uint commands[];
};

// Instanced matrices
layout(std430, binding = 3) writeonly buffer instance_matrices_buffer_layout
{
    MeshMatrix instance_matrices[];
};

#ifdef DRAW_COUNTER_ENABLED
layout (binding = 0, offset = 0) uniform atomic_uint draw_counter;
#endif

uniform mediump uint id_count;
uniform mediump uint instance_id_offset;
uniform float frame_counter;

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    if (gl_GlobalInvocationID.x < id_count)
    {
        uint mesh_id = mesh_ids[gl_GlobalInvocationID.x];

        if (IsVisible(meshes[mesh_id].aabb))
        {
            uint draw_id = uint(meshes[mesh_id].draw_params.z);

            // Increment the instance count
            uint instance_index = atomicAdd(commands[draw_id * 5u + 1u], 1u);

            // Copy the model and the model inverse matrices
#if USE_MATRIX_STREAM
            // Every thread writes to its own slot
            uint instance_offset = uint(MAX_INSTANCES) * (draw_id - uint(MESH_COUNT)) + uint(meshes[mesh_id].draw_params.w);

            // Store the frame counter in the bottom-right corner of the matrix
            // This element should be always zero in a any model matrices.
            // This way we indicates that this matrix has been written in this frame
            mat4 model = matrices[mesh_id].model;
            model[3][3] = frame_counter;

            instance_matrices[instance_offset].model = model;
            instance_matrices[instance_offset].inv_model = matrices[mesh_id].inv_model;

#else
            // Append the matrix to the end of the queue
            uint instance_offset = uint(MAX_INSTANCES) * (draw_id - uint(MESH_COUNT)) + instance_index;
            instance_matrices[instance_offset] = matrices[mesh_id];
#endif

#ifdef DRAW_COUNTER_ENABLED
            atomicCounterIncrement(draw_counter);
#endif
        }
    }
}

#endif
