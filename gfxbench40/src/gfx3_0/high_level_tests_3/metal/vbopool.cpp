/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vbopool.h"


BufferPool::BufferPool(BufferPoolType bufferPoolType, KCL::uint32 max_buffer_capacity)
{
}


BufferPool::~BufferPool()
{
}

KCL::uint32 BufferPool::GetCapacity()
{
	return 1;
}


VboPool* VboPool::s_instance = 0;
IndexBufferPool* IndexBufferPool::s_instance = 0;


VboPool* VboPool::Instance()
{
	if(!s_instance)
	{
		s_instance = new VboPool();
	}
	return s_instance;
}


void VboPool::DeleteInstance()
{
	delete s_instance;
	s_instance = 0;
}


IndexBufferPool* IndexBufferPool::Instance()
{
	if(!s_instance)
	{
		s_instance = new IndexBufferPool();
	}
	return s_instance;
}


void IndexBufferPool::DeleteInstance()
{
	delete s_instance;
	s_instance = 0;
}

