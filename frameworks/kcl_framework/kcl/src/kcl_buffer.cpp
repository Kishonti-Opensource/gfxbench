/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_buffer.h"

KCL::VertexBuffer::Factory* KCL::VertexBuffer::factory = NULL;
KCL::IndexBuffer::Factory* KCL::IndexBuffer::factory = NULL;
KCL::ConstantBuffer::Factory* KCL::ConstantBuffer::factory = NULL;

KCL::VertexBuffer::VertexBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite) :
	m_initialData(vertexData),
	m_vertexSize(vertexSize),
	m_vertexCount(vertexCount),
	m_releaseUponCommit(releaseUponCommit),
	m_gpuWrite(gpuWrite),
	m_cpuWrite(cpuWrite)
{
}

KCL::VertexBuffer::~VertexBuffer()
{
}

KCL::IndexBuffer::IndexBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite) :
	m_initialData(indexData),
	m_indexSize(2),
	m_indexCount(indexCount),
	m_releaseUponCommit(releaseUponCommit),
	m_cpuWrite(cpuWrite)
{
}

KCL::IndexBuffer::IndexBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite) :
	m_initialData(indexData),
	m_indexSize(4),
	m_indexCount(indexCount),
	m_releaseUponCommit(releaseUponCommit),
	m_cpuWrite(cpuWrite)
{
}

KCL::IndexBuffer::~IndexBuffer()
{
}

KCL::ConstantBuffer::ConstantBuffer(size_t size) :
	m_bufferSize(size)
{
}

KCL::ConstantBuffer::~ConstantBuffer()
{
}