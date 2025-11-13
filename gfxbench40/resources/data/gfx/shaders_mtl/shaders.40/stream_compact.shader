/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// Stream Compaction compute shader
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch39.html

#ifdef TYPE_compute

kernel void shader_main(uint2 globalId	[[thread_position_in_grid]],
						uint2 localId	[[thread_position_in_threadgroup]],
						uint2 groupId   [[threadgroup_position_in_grid]],
						constant OcclusionConstants * consts	[[buffer(0)]],
						const device int * stream_offsets		[[buffer(1)]],
						const device hfloat4x4 * input_buffer	[[buffer(2)]],
						device hfloat4x4 * output_buffer		[[buffer(3)]])
{
	threadgroup _float scan_input[MAX_INSTANCES];
	threadgroup _float temp[MAX_INSTANCES * 2];

    int thread_id = int(localId.x);

    // Load the input to shared memory
    int stream_offset = stream_offsets[groupId.x];

    int input_index = stream_offset + thread_id * 2;
    hfloat4x4 model = input_buffer[input_index];

    scan_input[thread_id] = abs(model[3][3] - consts->frame_counter) < 0.1 ? 1.0 : 0.0;
    temp[thread_id] = scan_input[thread_id];

	threadgroup_barrier(mem_flags::mem_threadgroup);

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

		threadgroup_barrier(mem_flags::mem_threadgroup);
    }

    // Scatter
    if (scan_input[thread_id] > 0.5)
    {
        // Write out the matrix to the final index and test the bottom-right corner
        int output_index = stream_offset + (int(temp[pout * MAX_INSTANCES + thread_id]) - 1) * 2;

        model[3][3] = 1.0;
        output_buffer[output_index + 0] = model;
        output_buffer[output_index + 1] = input_buffer[input_index + 1]; // Inverse model
    }
}

#endif
