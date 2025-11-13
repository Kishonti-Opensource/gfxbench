/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/stream/memstreambuf.h"

#include "ng/format.h"
#include "ng/require.h"
#include "ng/safecast.h"

#include <limits>
#include <cstring>
#include <cstdio>
#include <algorithm>

namespace ng
{

IMemStreamBuf::
	IMemStreamBuf()
	: _begin(NULL)
	, _cur(NULL)
	, _end(NULL)
{}

IMemStreamBuf::
	IMemStreamBuf(const ng::cref_bytevec& bv)
{
	_begin = bv.begin();
	_end = bv.end();
	_cur = _begin;
}

IMemStreamBuf::
	IMemStreamBuf(const void* begin, const void* end)
	: _begin((const uint8_t*)begin)
	, _cur((const uint8_t*)begin)
	, _end((const uint8_t*)end)
{
}

IMemStreamBuf::
	IMemStreamBuf(const void* begin, size_t size)
	: _begin((const uint8_t*)begin)
	, _cur((const uint8_t*)begin)
	, _end((const uint8_t*)begin + size)
{
}

void IMemStreamBuf::
	read(OUT void *data, size_t size, OUT size_t* nBytesRead)
{
	assert(_end >= _cur);
	size_t nBytesAvail = std::min((size_t)(_end - _cur), size);
	if ( nBytesRead == NULL )
	{
		require(nBytesAvail == size, cstr(format("reading %s bytes, buf only %s bytes left in buffer") % size % nBytesAvail));
	} else
	{
		*nBytesRead = nBytesAvail;
	}
	memcpy(data, _cur, nBytesAvail);
	_cur += nBytesAvail;
}

void IMemStreamBuf::
	seek(ptrdiff_t offset, int origin)
{
	switch(origin)
	{
	case SEEK_SET:
		if ( 0 <= offset && offset <= _end - _begin )
			_cur = _begin + offset;
		else
			require(false, cstr(format("invalid SEEK_SET offset: %s, size: %s") % offset % (_end - _begin) ));
		break;
	case SEEK_CUR:
		if ( offset < 0 )
		{
			if ( -offset <= _cur - _begin )
				_cur += offset;
			else
				require(false, cstr(format("invalid SEEK_CUR offset: %s, tell: %s") % offset % (_cur - _begin) ));
		} else if ( offset > 0 )
		{
			if ( offset <= _end - _cur )
				_cur += offset;
			else
				require(false, cstr(format("invalid SEEK_CUR offset: %s, tell: %s, size: %s") % offset % (_cur - _begin) % (_end - _begin) ) );
		}
		break;
	case SEEK_END:
		if ( offset <= 0 && -offset <= _end - _begin )
			_cur = _end + offset;
		else
			require(false, cstr(format("invalid SEEK_END offset: %s, size: %s") % offset % (_end - _begin) ) );
		break;
	default:
		require(false, cstr(format("unknown origin: %s") % origin));
	}
}

void IMemStreamBuf::
seek32(int32_t offset, int origin)
{
	seek((ptrdiff_t)offset, origin);
}

void IMemStreamBuf::
seek64(int64_t offset, int origin)
{
	require(
		sizeof(ptrdiff_t) == 8 ||
		((int64_t)std::numeric_limits<int32_t>::min() <= offset && offset <= (int64_t)std::numeric_limits<int32_t>::max()),
		cstr(format("64 bit offset too big, offset: %s, size: %s") % offset % (_end - _cur) ));
	seek((ptrdiff_t)offset, origin);
}

int32_t IMemStreamBuf::
	tell32()
{
	ptrdiff_t t = _cur - _begin;
	require(sizeof(ptrdiff_t) == 4 || t <= (ptrdiff_t)std::numeric_limits<int32_t>::max(),
		cstr(format("can't return 64 bit value in 32 bit, position: %s") % t));
	return (int32_t)t;
}

int64_t IMemStreamBuf::
	tell64()
{
	return (int64_t)(_cur - _begin);
}

//=============================================================================
//OMemStreamBuf
//=============================================================================

OMemStreamBuf::OMemStreamBuf()
{
	_p = _v.end();
}

void OMemStreamBuf::reserve(size_t s)
{
	const bool at_end = _p == _v.end();
	const int64_t pos = !at_end ? _p - _v.begin() : 0;
	_v.reserve(s);
	if (at_end)
		_p = _v.end();
	else
		_p = _v.begin() + pos;
}

void OMemStreamBuf::clear()
{
	_v.clear();
	_p = _v.end();
}

void OMemStreamBuf::swap(INOUT bytevec& v)
{
	_v.swap(v);
	_p = _v.end();
}

void OMemStreamBuf::
	write(const void *data, size_t size)
{
	const uint8_t* data_begin = (const uint8_t*)data;
	const uint8_t* data_end = data_begin + size;
	if (_p == _v.end())
	{
		_v.insert(_v.end(), data_begin, data_end);
		_p = _v.end();
	}
	else
	{
		const size_t space_left = static_cast<size_t>(_v.end() - _p);
		if (space_left < size)
		{
			const size_t new_size = _v.size() + size - space_left;
			if ( new_size > _v.capacity())
				reserve( capacity() * 2 < new_size ? capacity() * 2 : new_size * 2 );

			_v.resize( new_size );
		}

		std::copy(data_begin, data_end, _p);
		_p += size;
	}
}

void OMemStreamBuf::
	seek32(int32_t offset, int origin)
{
	//require(false, "seek is not supported");
	seek64(offset, origin);
}

void OMemStreamBuf::
	seek64(int64_t offset, int origin)
{
	switch (origin)
	{
	case SEEK_SET:
		require(offset >= 0, FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		require(static_cast<size_t>(offset) <= size(), FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		_p = _v.begin() + offset;
		break;

	case SEEK_CUR:
		if (offset < 0)
		{
			require( -offset <= _p - _v.begin(), FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		}
		else if (offset > 0)
		{
			require( SAFE_CAST<size_t>(_p - _v.begin() + offset) < size(), FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		}
		_p += offset;
		break;

	case SEEK_END:
		require(offset <= 0, FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		require(static_cast<size_t>(-offset) <= size(), FORMATSTR("Invalid arguments. Offset = %s, Origin = %s", offset, origin));
		_p = _v.end() + offset;
		break;

	default:
		throw std::runtime_error(FORMATSTR("OMemStreamBuf::seek64: origin is invalid. Origin = %s", origin));
	}
}

int32_t OMemStreamBuf::
	tell32()
{
	const int64_t t = tell64();
	require(t <= std::numeric_limits<int32_t>::max(),
		cstr(format("can't return 64 bit value in 32 bit, position: %s") % t));
	return (int32_t)t;
}

int64_t OMemStreamBuf::
	tell64()
{
	const size_t t = _p - _v.begin();
	require(sizeof(size_t) == 4 || t <= (size_t)std::numeric_limits<int64_t>::max(),
		cstr(format("can't return 64 bit unsigned value in 64 bit signed value, position: %s") % t));
	return (int64_t)t;
}


}
