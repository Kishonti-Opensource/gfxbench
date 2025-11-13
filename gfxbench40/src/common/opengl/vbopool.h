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

	void AddData(size_t sz, const void* data, KCL::uint32 &result_bufferId, size_t &result_bufferOffset);
	void SubData(size_t sz, const void* data, const KCL::uint32 bufferId, const size_t bufferOffset);
	void BindBuffer(KCL::uint32 id);
	
	void SetCapacity(KCL::uint32 max_buffer_capacity);
	KCL::uint32 GetCapacity();
	void Clear();

private:
	BufferPool(const BufferPool&);
	BufferPool& operator=(const BufferPool&);

	const KCL::uint32 m_bufferPoolType; // GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
	KCL::uint32 m_max_buffer_capacity;
	KCL::uint32 m_lastBound;

	struct BufferWithCount
	{
		BufferWithCount() : m_buffer(0), m_size(0)
		{
		}

		KCL::uint32 m_buffer;
		size_t m_size;
	};

	std::vector<BufferWithCount> m_buffers;
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

