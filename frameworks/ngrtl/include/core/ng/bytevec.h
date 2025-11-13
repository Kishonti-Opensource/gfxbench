/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_bytevec_1345728006
#define INCLUDE_GUARD_bytevec_1345728006

#include <stdint.h>

#include <vector>
#include <string>
#include <cstring>
#include <assert.h>

#include "ng/macros.h"

namespace ng {

typedef std::vector<uint8_t> bytevec;

/* ---- cref_bytevec ----

Const-ref bytevec: Immutable object containing a weak pointer to a memory block (and block's size)

	Can be used as input parameter type accepting both (pointer,size) and std::vector<uint8_t> arguments
	Or as return value.

Example:

	void writeIntoFile(cref_bytevec bv) //this involves a copy-constructor (cheap, since cref_bytevec is only 2 vars)
	
		or 
		
	void writeIntoFile(const cref_bytevec& bv) //no copy-ctor, more typing
	
	
	Now you can use this function:
	
	writeIntoFile(cref_bytevec(myptr, mysize))
	writeIntoFile(my_std_vec_of_bytes)
	
As return value

	cref_bytevec getBuffer() const
	{
		return m_myStdVecOfBytesBuffer;
	}

*/

class cref_bytevec
{
public:
	typedef uint8_t value_type;

	cref_bytevec()
	: _p(&_dummy)
	, _s(0)
	{}
	
	cref_bytevec(const void* p, size_t s)
	: _p(!!p ? (const uint8_t*)p : &_dummy)
	, _s(s)
	{
		assert(!!p || s == 0);
	}
	
	cref_bytevec(const char* p)
	: _p((const uint8_t*)p)
	, _s(strlen(p))
	{
	}

	cref_bytevec(const std::string& v)
	: _p(v.empty() ? &_dummy : (const uint8_t*)v.data())
	, _s(v.size())
	{}

	cref_bytevec(const std::vector<uint8_t>& v)
	: _p(v.empty() ? &_dummy : &v.front())
	, _s(v.size())
	{}
	
	cref_bytevec(const std::vector<int8_t>& v)
	: _p(v.empty() ? &_dummy : (const uint8_t*)&v.front())
	, _s(v.size())
	{}

	void assign(const void* p, size_t s)
	{
		_p = (!!p ? (const uint8_t*)p : &_dummy);
		_s = s;
		assert(!!p || s == 0);
	}
	
	void assign(const void* b, const void* e)
	{
		_p = (const uint8_t*)b;
		_s = (const uint8_t*)e - (const uint8_t*)b;
		assert(b <= e);
	}

	void assign(const std::string& v)
	{
		_p = (v.empty() ? &_dummy : (const uint8_t*)v.data());
		_s = v.size();
	}

	void assign(const std::vector<uint8_t>& v)
	{
		_p = (v.empty() ? &_dummy : &v.front());
		_s = v.size();
	}
	
	void assign(const std::vector<int8_t>& v)
	{
		_p = (v.empty() ? &_dummy : (const uint8_t*)&v.front());
		_s = v.size();
	}

	size_t size() const { return _s; }
	const uint8_t* data() const { return _p; }
	const uint8_t* begin() const { return _p; }
	const uint8_t* end() const { return _p + _s; }
	bool empty() const
	{
		return _s == 0;
	}
	void clear()
	{
		_p = &_dummy;
		_s = 0;
	}
	uint8_t front() const { return *_p; }
private:
	const uint8_t* _p;
	size_t _s;
	static const uint8_t _dummy;
};


}


#endif

