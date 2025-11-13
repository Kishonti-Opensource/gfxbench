/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// Stream Compaction compute shader
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html

#ifdef TYPE_compute

// Input stream to compact
layout(std430, binding = 0) readonly buffer input_buffer_layout
{
   mat4 input_buffer[];
};

// Output compacted stream
layout(std430, binding = 1) writeonly buffer ouput_buffer_layout
{
   mat4 output_buffer[];
};

uniform int stream_offsets[MAX_MIO_COUNT];
uniform float frame_counter;

shared float scan_input[MAX_INSTANCES];
shared float temp[MAX_INSTANCES * 2];

layout (local_size_x = MAX_INSTANCES, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int thread_id = int(gl_LocalInvocationID.x);

    // Load the input to shared memory
    int stream_offset = stream_offsets[gl_WorkGroupID.x];

    int input_index = stream_offset + thread_id * 2;
    mat4 model = input_buffer[input_index];

    scan_input[thread_id] = abs(model[3][3] - frame_counter) < 0.1 ? 1.0 : 0.0;
    temp[thread_id] = scan_input[thread_id];

    memoryBarrierShared();
    barrier();

    // Inclusive Scan
    int pout = 0;
    int pin = 1;
    for (int offset = 1; offset < MAX_INSTANCES; offset *= 2)
    {
        // Swap the buffer indices
        pout = 1 - pout;
        pin = 1 - pout;

        int read_location = pin * MAX_INSTANCES + thread_id;
        if (thread_id >= offset)
        {
            temp[pout * MAX_INSTANCES + thread_id] = temp[read_location - offset] + temp[read_location];
        }
        else
        {
            temp[pout * MAX_INSTANCES + thread_id] = temp[read_location];
        }

        memoryBarrierShared();
        barrier();
    }

    // Scatter
    if (scan_input[thread_id] > 0.5)
    {
        // Write out the matrix to the final index and test the bottom-right corner
        int output_index = stream_offset + (int(temp[pout * MAX_INSTANCES + thread_id]) - 1) * 2;

        model[3][3] = 1.0;
        output_buffer[output_index + 0] = model; // Model
        output_buffer[output_index + 1] = input_buffer[input_index + 1]; // Inverse model
    }
}

#endif
