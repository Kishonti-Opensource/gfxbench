/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_buffer.h"
#include "DXUtils.h"
#include "DX.h"
#include "d3d11/shader.h"

#define DXB_ERROR_BUFFER_SUCCESS		0
#define DXB_ERROR_BUFFER_UNKNOWN_ERROR	1
#define DXB_ERROR_BUFFER_UNINITIALIZED	2
#define DXB_ERROR_BUFFER_ALREADY_MAPPED	3
#define DXB_ERROR_BUFFER_ALREADY_COMMITTED	4
#define DXB_ERROR_BUFFER_SIZE_MISMATCH	5

#define SAFE_RELEASE(x)	if ((x)) { (x)->Release(); (x) = NULL; }
#define SAFE_DELETE(x)	if ((x)) { delete (x); (x) = NULL; }
#define EXIT_IF_FAILED(x) if (FAILED(x)) { return -1; }

DXB::DXBBuffer::DXBBuffer(void) :
	m_buffer(NULL)
{
	ZeroMemory(&m_descriptor, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&m_mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
}

DXB::DXBBuffer::~DXBBuffer(void)
{
	SAFE_RELEASE(m_buffer);
}

long DXB::DXBBuffer::commit(const void* initialData, size_t size, UINT bindFlags, D3D11_USAGE usage, UINT cpuaccessFlags, UINT miscFlags, size_t structureByteStride)
{
	if (m_buffer)
	{
		return DXB_ERROR_BUFFER_ALREADY_COMMITTED;
	}

	assert(size);

	m_descriptor.BindFlags = bindFlags;
	m_descriptor.ByteWidth = size;
	m_descriptor.CPUAccessFlags = cpuaccessFlags;
	m_descriptor.Usage = usage;
	m_descriptor.StructureByteStride =  (UINT)structureByteStride;
	m_descriptor.MiscFlags = (UINT)miscFlags;

	if (initialData)
	{
		D3D11_SUBRESOURCE_DATA resourceData;
		resourceData.pSysMem = initialData;
		resourceData.SysMemPitch = 0;
		resourceData.SysMemSlicePitch = 0;
		DX_THROW_IF_FAILED(DX::getDevice()->CreateBuffer(&m_descriptor, &resourceData, &m_buffer));
	}
	else
	{
		DX_THROW_IF_FAILED(DX::getDevice()->CreateBuffer(&m_descriptor, NULL, &m_buffer));
	}

	return DXB_ERROR_BUFFER_SUCCESS;
}

long DXB::DXBBuffer::updateData(const void* data, size_t size)
{
	if (!m_buffer)
	{
		return DXB_ERROR_BUFFER_UNINITIALIZED;
	}
	else if (m_mappedSubresource.pData)
	{
		return DXB_ERROR_BUFFER_ALREADY_MAPPED;
	}
	else if (size > getSize())
	{
		return DXB_ERROR_BUFFER_SIZE_MISMATCH;
	}

	HRESULT hr = DX::getContext()->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_mappedSubresource);
	if (FAILED(hr))
	{
		return (UINT)hr;
	}

	memcpy(m_mappedSubresource.pData, data, size);
	DX::getContext()->Unmap(m_buffer, 0);
	m_mappedSubresource.pData = NULL;

    return UINT(hr);
}

void* DXB::DXBBuffer::map(D3D11_MAP mapType)
{
	if (!m_buffer)
	{
		return NULL;
	}

	if (!m_mappedSubresource.pData)
	{
		HRESULT hr = DX::getContext()->Map(m_buffer, 0, mapType, 0, &m_mappedSubresource);
		if (FAILED(hr))
		{
			return NULL;
		}
	}

	return m_mappedSubresource.pData;
}

void DXB::DXBBuffer::unmap()
{
	if (m_mappedSubresource.pData)
	{
		DX::getContext()->Unmap(m_buffer, 0);
		m_mappedSubresource.pData = NULL;
	}
}

DXB::DXBIndexBuffer::DXBIndexBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite) :
	KCL::IndexBuffer(indexData, indexCount, releaseUponCommit, cpuWrite),
	m_format(DXGI_FORMAT_R16_UINT)
{
}

DXB::DXBIndexBuffer::DXBIndexBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite) :
	KCL::IndexBuffer(indexData, indexCount, releaseUponCommit, cpuWrite),
	m_format(DXGI_FORMAT_R32_UINT)
{
}

DXB::DXBIndexBuffer::~DXBIndexBuffer()
{
	unmap();	// This performs the necessary check as well
	SAFE_RELEASE(m_buffer);
}

long DXB::DXBIndexBuffer::commit()
{
	return DXBBuffer::commit(
		m_initialData,
		m_indexCount * m_indexSize, 
		D3D11_BIND_INDEX_BUFFER,
		m_cpuWrite ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
		m_cpuWrite ? (UINT)D3D11_CPU_ACCESS_WRITE : 0U);
}

long DXB::DXBIndexBuffer::bind(UINT slotId)
{
	DX::getContext()->IASetIndexBuffer(m_buffer, m_format, 0);
	return DXB_ERROR_BUFFER_SUCCESS;
}

long DXB::DXBIndexBuffer::updateData(const KCL::uint16* indexData, KCL::uint32 indexCount)
{
	long result = DXBBuffer::updateData(indexData, indexCount * 2);
	if (result == DXB_ERROR_BUFFER_SUCCESS)
	{
		m_format = DXGI_FORMAT_R16_UINT;
		m_indexSize = 2;
		m_indexCount = indexCount;
	}

	return result;
}

long DXB::DXBIndexBuffer::updateData(const KCL::uint32* indexData, KCL::uint32 indexCount)
{
	long result = DXBBuffer::updateData(indexData, indexCount * 2);
	if (result == DXB_ERROR_BUFFER_SUCCESS)
	{
		m_format = DXGI_FORMAT_R32_UINT;
		m_indexSize = 4;
		m_indexCount = indexCount;
	}

	return result;
}

DXB::DXBVertexBuffer::DXBVertexBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite) :
	KCL::VertexBuffer(vertexData, vertexSize, vertexCount, releaseUponCommit, cpuWrite, gpuWrite)
{
}

DXB::DXBVertexBuffer::~DXBVertexBuffer()
{
	unmap();	// This performs the necessary check as well
	SAFE_RELEASE(m_buffer);
}

long DXB::DXBVertexBuffer::commit()
{
	return DXBBuffer::commit(
		m_initialData,
		m_vertexSize * m_vertexCount, 
		m_gpuWrite ? (D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT) : D3D11_BIND_VERTEX_BUFFER, 
		m_cpuWrite ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
		m_cpuWrite ? (UINT)D3D11_CPU_ACCESS_WRITE : 0U);
}

ID3D11InputLayout* createVertexLayout(const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT numElements, Shader* shader)
{
	if (pInputElementDescs)
	{
		ID3D11InputLayout* result;
		HRESULT hr = DX::getDevice()->CreateInputLayout(
			pInputElementDescs,
			numElements,
			&shader->m_vs.m_bytes[0],
			shader->m_vs.m_bytes.size(),
			&result);

		return (SUCCEEDED(hr))? result : NULL;
	}
	else
	{
		return NULL;
	}
}

long DXB::DXBVertexBuffer::bind(UINT slotId)
{
	UINT offset = 0;
	DX::getContext()->IASetVertexBuffers(slotId, 1, &m_buffer, (const UINT*)&m_vertexSize, &offset);

	return DXB_ERROR_BUFFER_SUCCESS;
}

long DXB::DXBVertexBuffer::updateData(const void* vertexData, KCL::uint32 vertexCount)
{
	long result = DXBBuffer::updateData(vertexData, m_vertexSize * vertexCount);
	if (result == DXB_ERROR_BUFFER_SUCCESS)
	{
		m_vertexCount = vertexCount;
	}

	return result;
}

DXB::DXBConstantBuffer::DXBConstantBuffer(size_t size) :
	KCL::ConstantBuffer(size)
{
}

DXB::DXBConstantBuffer::~DXBConstantBuffer()
{
	unmap();	// This performs the necessary check as well
	SAFE_RELEASE(m_buffer);
}

long DXB::DXBConstantBuffer::commit()
{
	m_bindToGeometryShader = DX::getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_0;
	return DXBBuffer::commit(NULL, m_bufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

long DXB::DXBConstantBuffer::bind(UINT slotId)
{
	DX::getContext()->VSSetConstantBuffers(slotId, 1, &m_buffer);
	DX::getContext()->PSSetConstantBuffers(slotId, 1, &m_buffer);

	if (m_bindToGeometryShader)
	{
		DX::getContext()->GSSetConstantBuffers(slotId, 1, &m_buffer);
	}

	return DXB_ERROR_BUFFER_SUCCESS;
}

void* DXB::DXBConstantBuffer::map()
{
	return DXBBuffer::map();
}

void DXB::DXBConstantBuffer::unmap()
{
	DXBBuffer::unmap();
}
