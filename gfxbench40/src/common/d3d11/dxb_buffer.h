/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <d3d11_1.h>
#include "kcl_buffer.h"
#include "d3d11/shader.h"

namespace DXB
{
	class DXBBuffer
	{
	protected:
		D3D11_BUFFER_DESC m_descriptor;
		D3D11_MAPPED_SUBRESOURCE m_mappedSubresource;
		ID3D11Buffer* m_buffer;

	public:
		DXBBuffer(void);
		virtual ~DXBBuffer(void);

		inline ID3D11Buffer* getBuffer() const{ return m_buffer; }
		inline size_t getSize() const	{ return m_descriptor.ByteWidth; }
		void* map(D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD);
		void unmap();
		void getData(void** data, size_t* size);

	protected:
		long commit(const void* initialData, size_t size, UINT bindFlags, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT cpuaccessFlags = 0, UINT miscFlags = 0, size_t structureByteStride = 0);
		long updateData(const void* data, size_t size);
	};

	class DXBIndexBuffer : public DXBBuffer, public KCL::IndexBuffer
	{
	protected:
		DXGI_FORMAT m_format;

	public:
		class Factory : public KCL::IndexBuffer::Factory
		{
		public:
			/* override */
			virtual KCL::IndexBuffer* CreateBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite)
			{
				assert(indexCount);
				return new DXBIndexBuffer(indexData, indexCount, releaseUponCommit, cpuWrite);
			}

			/* override */
			virtual KCL::IndexBuffer* CreateBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit, bool cpuWrite)
			{
				assert(indexCount);
				return new DXBIndexBuffer(indexData, indexCount, releaseUponCommit, cpuWrite);
			}
		};

	public:
		DXBIndexBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false);
		DXBIndexBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false);
		virtual ~DXBIndexBuffer();

		/* override */ virtual long commit();
		/* override */ virtual long bind(UINT slotId);
		/* override */ virtual long updateData(const KCL::uint16* indexData, KCL::uint32 indexCount);
		/* override */ virtual long updateData(const KCL::uint32* indexData, KCL::uint32 indexCount);
	};



	class DXBVertexBuffer : public DXBBuffer, public KCL::VertexBuffer
	{
	public:
		class Factory : public KCL::VertexBuffer::Factory
		{
		protected:
			/* override */
			virtual KCL::VertexBuffer* CreateBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite)
			{
				assert(vertexSize);
				assert(vertexCount);
				return new DXBVertexBuffer(vertexData, vertexSize, vertexCount, releaseUponCommit, cpuWrite, gpuWrite);
			}
		};

		DXBVertexBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite);
		virtual ~DXBVertexBuffer();

		/* override */ virtual long commit();
		/* override */ virtual long bind(UINT slotId);
		/* override */ virtual long updateData(const void* vertexData, KCL::uint32 vertexCount);

		long createVertexLayout(const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs, UINT numElements, Shader* shader);
	};



	class DXBConstantBuffer : public DXBBuffer, public KCL::ConstantBuffer
	{
	public:
		class Factory : public KCL::ConstantBuffer::Factory
		{
		public:
			/* override */
			virtual KCL::ConstantBuffer* CreateBuffer(size_t size)
			{
				assert(size);
				return new DXBConstantBuffer(size);
			}
		};

		DXBConstantBuffer(size_t size);
		virtual ~DXBConstantBuffer();

		/* override */ virtual long commit();
		/* override */ virtual long bind(KCL::uint32 slotId);
		/* override */ virtual void* map();
		/* override */ virtual void unmap();

	private:
		bool m_bindToGeometryShader;
	};
}