/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_MEMSTREAMBUF_INCLUDED
#define NG_MEMSTREAMBUF_INCLUDED

#include <vector>

#include "stream.h"
#include "ng/ngrtl_core_export.h"
#include "ng/bytevec.h"

//memory-based streambuf

namespace ng
{
	class NGRTL_EXPORT IMemStreamBuf
		: public ISeekStream
	{
	public:
		IMemStreamBuf();
		//input data must persist until end of reading, no copy made
		IMemStreamBuf(const void* begin, const void* end);
		IMemStreamBuf(const void* begin, size_t size);
		IMemStreamBuf(const ng::cref_bytevec& bv);
		virtual void read(OUT void *data, size_t size, OUT size_t* nBytesRead = 0);

		virtual void seek32(int32_t offset, int origin);
		virtual void seek64(int64_t offset, int origin);
		virtual int32_t tell32();
		virtual int64_t tell64();
	private:
		const uint8_t* _begin;
		const uint8_t* _cur;
		const uint8_t* _end;

		void seek(ptrdiff_t offset, int origin);
		ptrdiff_t tell();
	};

	//writes into internal buffer
	class NGRTL_EXPORT OMemStreamBuf
		: public OSeekStream
	{
	public:
		OMemStreamBuf();

		size_t capacity() const { return _v.capacity(); }
		void reserve(size_t s);
		size_t size() const { return _v.size(); }
		void clear();
		void swap(INOUT bytevec& v);
		const uint8_t* data() const //valid even if size == 0
			{ const static uint8_t dummy(0); return _v.empty() ? &dummy : &_v[0]; }
		cref_bytevec buffer() const { return cref_bytevec(_v); }

		virtual void seek32(int32_t offset, int origin);
		virtual void seek64(int64_t offset, int origin);
		virtual int32_t tell32();
		virtual int64_t tell64();

		virtual void write(const void *data, size_t size);
		virtual void flush() {}
	private:
		bytevec _v;
		bytevec::iterator _p;
	};
}

#endif
