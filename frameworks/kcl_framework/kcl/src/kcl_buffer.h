/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_BUFFER_H
#define KCL_BUFFER_H

#include "kcl_image.h"
#include "kcl_math3d.h"

namespace KCL
{
	class KCL_API IndexBuffer
	{
	protected:
		const void* m_initialData;
		bool m_isCommitted;
		bool m_cpuWrite;
		bool m_releaseUponCommit;
		size_t m_indexSize;
		KCL::uint32 m_indexCount;

	public:
		class Factory
		{
		public:
			virtual KCL::IndexBuffer* CreateBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false) = 0;
			virtual KCL::IndexBuffer* CreateBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false) = 0;
		};

		static Factory* factory;

		IndexBuffer(const KCL::uint16* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false);
		IndexBuffer(const KCL::uint32* indexData, KCL::uint32 indexCount, bool releaseUponCommit = false, bool cpuWrite = false);
		virtual ~IndexBuffer();

		/**
			Gets the size of the elements in the buffer.
			@return		size of the elements in the buffer.
		*/
		inline size_t getIndexSize()	{ return m_indexSize; }

		/**
			Gets the number of the elements in the buffer.
			@return		number of the elements in the buffer.
		*/
		inline KCL::uint32 getIndexCount()	{ return m_indexCount; }

		/**
			Gets a value indicating whether the resource is committed or not.
			@return		true if the resource is committed, false otherwise.
		*/
		inline bool isCommitted() const	{ return m_isCommitted; };

		/**
			Creates platform-specific resources for the index buffer.
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long commit() = 0;

		/**
			Binds the buffer to the specified input slot.
			@slotId		Identifier of the input slot.
		*/
		virtual long bind(KCL::uint32 slotId) = 0;

		virtual long updateData(const KCL::uint16* indexData, KCL::uint32 indexCount) = 0;

		virtual long updateData(const KCL::uint32* indexData, KCL::uint32 indexCount) = 0;
	};

	class KCL_API VertexBuffer
	{
	protected:
		const void* m_initialData;
		bool m_isCommitted;
		bool m_gpuWrite;
		bool m_cpuWrite;
		bool m_releaseUponCommit;
		size_t m_vertexSize;
		KCL::uint32 m_vertexCount;

	public:
		class Factory
		{
		public:
			template<class T> KCL::VertexBuffer* CreateBuffer(KCL::uint32 vertexCount, bool cpuWrite = false, bool gpuWrite = false)
			{
				return CreateBuffer(NULL, sizeof(T), vertexCount, false, cpuWrite, gpuWrite);
			}

			template<class T> KCL::VertexBuffer* CreateBuffer(const T* vertexData, KCL::uint32 vertexCount, bool releaseUponCommit = false, bool cpuWrite = false, bool gpuWrite = false)
			{
				return CreateBuffer((const void*)vertexData, sizeof(T), vertexCount, releaseUponCommit, cpuWrite, gpuWrite);
			}

		protected:
			virtual KCL::VertexBuffer* CreateBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite) = 0;
		};

		static Factory* factory;

		VertexBuffer(const void* vertexData, size_t vertexSize, KCL::uint32 vertexCount, bool releaseUponCommit, bool cpuWrite, bool gpuWrite);
		virtual ~VertexBuffer();

		/**
			Gets the size of the elements in the buffer.
			@return		size of the elements in the buffer.
		*/
		inline size_t getVertexSize()	{ return m_vertexSize; }

		/**
			Gets the number of the elements in the buffer.
			@return		number of the elements in the buffer.
		*/
		inline KCL::uint32 getVertexCount()	{ return m_vertexCount; }

		/**
			Creates platform-specific resources for the index buffer.
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long commit() = 0;

		/**
			Binds the buffer to the specified input slot.
			@slotId		Identifier of the input slot.
		*/
		virtual long bind(KCL::uint32 slotId) = 0;

		virtual long updateData(const void* vertexData, KCL::uint32 vertexCount) = 0;
	};

	class KCL_API ConstantBuffer
	{
	protected:
		size_t m_bufferSize;

	public:
		class Factory
		{
		public:
			template<class T> KCL::ConstantBuffer* CreateBuffer()
			{
				return CreateBuffer(sizeof(T));
			}

		protected:
			virtual KCL::ConstantBuffer* CreateBuffer(size_t size) = 0;
		};

		static Factory* factory;

		ConstantBuffer(size_t size);
		virtual ~ConstantBuffer();

		/**
			Gets the size of the buffer.
			@return		size of the buffer.
		*/
		inline size_t getSize()	{ return m_bufferSize; }

		/**
			Creates platform-specific resources for the index buffer.
			@return		implementation-specific error code. 0 for success.
		*/
		virtual long commit() = 0;

		/**
			Binds the buffer to the specified input slot.
			@slotId		Identifier of the input slot.
		*/
		virtual long bind(KCL::uint32 slotId) = 0;

		/**

		*/
		virtual void* map() = 0;

		virtual void unmap() = 0;
	};
};

#endif