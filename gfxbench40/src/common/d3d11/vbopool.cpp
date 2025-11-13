/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vbopool.h"
#include "platform.h"


BufferPool::BufferPool(D3D11_BIND_FLAG bufferPoolType, KCL::uint32 max_buffer_capacity) :
	m_bufferPoolType(bufferPoolType),
	m_max_buffer_capacity(max_buffer_capacity),
	m_lastBound(0)
{
}


BufferPool::~BufferPool()
{
	Clear();
}


void BufferPool::AddData(size_t sz, const void* data, KCL::uint32 &result_bufferId, KCL::uint32 &result_bufferOffset, KCL::uint32 stride)
{
	BufferWithCount newBuffer;
	newBuffer.m_size = sz;
	newBuffer.m_buffer;
	newBuffer.m_stride = stride;
	
    D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
    vertexBufferData.pSysMem = data;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;
    DX_THROW_IF_FAILED(
        DX::getDevice()->CreateBuffer(
			&CD3D11_BUFFER_DESC(sz, m_bufferPoolType),
            &vertexBufferData,
			&newBuffer.m_buffer
            )
        );
	
	m_buffers.push_back(newBuffer);
	result_bufferOffset = 0;
	result_bufferId = m_buffers.size();
}



void BufferPool::SubData(size_t sz, const void* data, const KCL::uint32 bufferId, const KCL::uint32 bufferOffset)
{
	DX::getContext()->UpdateSubresource( 
		m_buffers[bufferId-1].m_buffer.Get(),
		0,
		0,
		data,
		0, 
		0);
}


void BufferPool::SetCapacity(KCL::uint32 max_buffer_capacity)
{
	Clear();
	m_max_buffer_capacity = max_buffer_capacity;
}

KCL::uint32 BufferPool::GetCapacity()
{
	return m_max_buffer_capacity;
}

void BufferPool::Clear()
{
	m_lastBound = 0;
	m_buffers.clear();
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
	if (s_instance)
	{
		delete s_instance;
		s_instance = NULL;
	}
}


void VboPool::BindBuffer(KCL::uint32 id, UINT slot)
{
	// Track only the 0th slot...
	if (!slot)
	{
		if (id == m_lastBound)
		{
			return;
		}
		else
		{
			m_lastBound = id;
		}
	}

	if(id)
	{
		unsigned int offset = 0;
		DX::getContext()->IASetVertexBuffers(
			slot,
			1,
			m_buffers[id-1].m_buffer.GetAddressOf(),
			&m_buffers[id-1].m_stride,
			&offset
			);
	}
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
	if (s_instance)
	{
		delete s_instance;
		s_instance = NULL;
	}
}


void IndexBufferPool::BindBuffer(KCL::uint32 id)
{
	if(id)
	{
		if(id != m_lastBound)
		{
			m_lastBound = id;
			DX::getContext()->IASetIndexBuffer(
				m_buffers[id-1].m_buffer.Get(),
				DXGI_FORMAT_R16_UINT,
				0
				);
		}
	}
	else
	{
		m_lastBound = 0;
	}
}
