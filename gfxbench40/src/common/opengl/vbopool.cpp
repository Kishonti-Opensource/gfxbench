/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vbopool.h"
#include "platform.h"


BufferPool::BufferPool(BufferPoolType bufferPoolType, KCL::uint32 max_buffer_capacity) :
	m_bufferPoolType(bufferPoolType == BP_ARRAY_BUFFER ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER),
	m_max_buffer_capacity(max_buffer_capacity),
	m_lastBound(0)
{
}


BufferPool::~BufferPool()
{
	Clear();
}


void BufferPool::AddData(size_t sz, const void* data, KCL::uint32 &result_bufferId, size_t &result_bufferOffset)
{
	result_bufferId = 0;
	result_bufferOffset = 0;

	bool createNewBuffer = (sz >= m_max_buffer_capacity);

	if(!createNewBuffer) // search a buffer that has enough space, if not found any: createNewBuffer = true
	{
		bool bufferNotFound = true;

		
		for(size_t i=0; i<m_buffers.size() && bufferNotFound; ++i)
		{
			if( m_buffers[i].m_size + sz <= m_max_buffer_capacity )
			{
				bufferNotFound = false;

				result_bufferId = m_buffers[i].m_buffer;
				result_bufferOffset = m_buffers[i].m_size;
				
				m_buffers[i].m_size += sz;

				glBindBuffer(m_bufferPoolType, result_bufferId);
				glBufferSubData(m_bufferPoolType, (GLintptr)result_bufferOffset, (GLsizeiptr)sz, data);
				glBindBuffer(m_bufferPoolType, 0);
			}
		}


		createNewBuffer = bufferNotFound;
	}

	if(createNewBuffer)
	{
		result_bufferOffset = 0;
		glGenBuffers(1, &result_bufferId);

		glBindBuffer(m_bufferPoolType, result_bufferId);
		
		if(sz >= m_max_buffer_capacity)
		{
			glBufferData(m_bufferPoolType, (GLsizeiptr)sz, data, GL_STATIC_DRAW);
		}
		else
		{
			glBufferData(m_bufferPoolType, (GLsizeiptr)m_max_buffer_capacity, 0, GL_STATIC_DRAW);
			glBufferSubData(m_bufferPoolType, 0, (GLsizeiptr)sz, data);
		}
		
		glBindBuffer(m_bufferPoolType, 0);

		BufferWithCount newBuffer;
		newBuffer.m_buffer = result_bufferId;
		newBuffer.m_size = sz;
		m_buffers.push_back(newBuffer);
	}
}


void BufferPool::SubData(size_t sz, const void* data, const KCL::uint32 bufferId, const size_t bufferOffset)
{
	BindBuffer(bufferId);

	glBufferSubData( m_bufferPoolType, (GLintptr)bufferOffset, (GLsizeiptr)sz, data);

	BindBuffer(0);
}


void BufferPool::BindBuffer(KCL::uint32 id)
{
	if(id)
	{
		if(id != m_lastBound)
		{
			m_lastBound = id;
			glBindBuffer(m_bufferPoolType, id);
		}
	}
	else
	{
		m_lastBound = 0;
		glBindBuffer(m_bufferPoolType, 0);
	}
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
	BindBuffer(0);

	for(size_t i=0; i<m_buffers.size(); ++i)
	{
		glDeleteBuffers(1, &(m_buffers[i].m_buffer) );
	}

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

