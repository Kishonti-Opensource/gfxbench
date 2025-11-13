/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__VBOPOOL_H__
#define __XX__VBOPOOL_H__

#include <vector>
#include <kcl_base.h>
#include <cstring>//size_t

class BufferPool
{
protected:
	enum BufferPoolType {BP_ARRAY_BUFFER, BP_ELEMENT_ARRAY_BUFFER};

	BufferPool(BufferPoolType bufferPoolType, KCL::uint32 max_buffer_capacity);
public:
	virtual ~BufferPool();
    
    KCL::uint32 GetCapacity() ;

private:
	BufferPool(const BufferPool&);
	BufferPool& operator=(const BufferPool&);
};


class VboPool : public BufferPool
{
public:
	static VboPool* Instance();
	static void DeleteInstance();

private:
	static VboPool* s_instance;

	VboPool(KCL::uint32 max_buffer_capacity = 1024*1024) : BufferPool(BP_ARRAY_BUFFER, max_buffer_capacity)
	{
	}
};


class IndexBufferPool : public BufferPool
{
public:
	static IndexBufferPool* Instance();
	static void DeleteInstance();

private:
	static IndexBufferPool* s_instance;

	IndexBufferPool(KCL::uint32 max_buffer_capacity = 1024*1024) : BufferPool(BP_ELEMENT_ARRAY_BUFFER, max_buffer_capacity)
	{
	}
};


#endif //__XX__VBOPOOL_H__

