/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __XX__VBOPOOL_H__
#define __XX__VBOPOOL_H__

#include <vector>
#include <d3d11_1.h>
#include "d3d11/DXUtils.h"
#include "d3d11/DX.h"
#include <kcl_base.h>



class BufferPool
{
protected:
	BufferPool(D3D11_BIND_FLAG bufferPoolType, KCL::uint32 max_buffer_capacity);

public:
	virtual ~BufferPool();

	virtual void BindBuffer(KCL::uint32 id) = 0;
	inline void ForgetLastBound()	{ m_lastBound = 0xcdcdcdcd; }

	void AddData(size_t sz, const void* data, KCL::uint32 &result_bufferId, KCL::uint32 &result_bufferOffset, KCL::uint32 stride = 0);
	void SubData(size_t sz, const void* data, const KCL::uint32 bufferId, const KCL::uint32 bufferOffset);

	void SetCapacity(KCL::uint32 max_buffer_capacity);
	KCL::uint32 GetCapacity();
	void Clear();
	inline ID3D11Buffer* getBuffer(KCL::uint32 id) { return m_buffers[id-1].m_buffer.Get(); }

protected:
	BufferPool(const BufferPool&);
	BufferPool& operator=(const BufferPool&);
	const D3D11_BIND_FLAG m_bufferPoolType;
	KCL::uint32 m_max_buffer_capacity;
	KCL::uint32 m_lastBound;

	struct BufferWithCount
	{
		BufferWithCount() : m_size(0)
		{
		}

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
		size_t m_size;
		KCL::uint32 m_stride;
	};

	std::vector<BufferWithCount> m_buffers;
};


class VboPool : public BufferPool
{
public:
	static VboPool* Instance();
	static void DeleteInstance();
	/* override */ virtual void BindBuffer(KCL::uint32 id) { BindBuffer(id, 0); }
	virtual void BindBuffer(KCL::uint32 id, UINT slot);

private:
	static VboPool* s_instance;

	VboPool(KCL::uint32 max_buffer_capacity = 1024*1024) : BufferPool(D3D11_BIND_VERTEX_BUFFER, max_buffer_capacity)
	{
	}
};


class IndexBufferPool : public BufferPool
{
public:
	static IndexBufferPool* Instance();
	static void DeleteInstance();
	/* override */ virtual void BindBuffer(KCL::uint32 id);

private:
	static IndexBufferPool* s_instance;

	IndexBufferPool(KCL::uint32 max_buffer_capacity = 1024*1024) : BufferPool(D3D11_BIND_INDEX_BUFFER, max_buffer_capacity)
	{
	}
};


#endif //__XX__VBOPOOL_H__

