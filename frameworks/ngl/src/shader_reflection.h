/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//type: 0 - varying, 10 - buffer, 11 - buffer member, 20 - sampler
//format: -1 - invalid, 
//0 - float, 1 - float2, 2 - float3, 3 - float4, 4 - float16
//10 - int, 11 - int2, 12 - int3, 13 - int4, 
//20 - uint, 21 - uint2, 22 - uint3, 23 - uint4
//1000 - ubo, 1001 - read/write ssbo, 1002 - readonly ssbo
//2000 - sampler2D, 2001 - sampler2DShadow, 2002 - subpassInput, 2003 - image
//stage: 0 - vertex, 1 - tess control, 2 - tess eval, 3 - geometry, 4 - fragment, 5 - compute
//group: -1 - invalid, 0 - per-draw group, 1 - per-renderchange group, 2 - manual group, 3 - perdraw fast group

struct _shader_reflection
{
	struct Block
	{
		std::string name;
		int format;
		int type;
		int binding_or_offset_or_location;
		int size;
		int stage;
		int set;
		std::vector<Block> blocks;

		Block()
		{
			binding_or_offset_or_location = -1;
		}
	};

	std::vector<Block> attributes;
	std::vector<Block> uniforms;
};
