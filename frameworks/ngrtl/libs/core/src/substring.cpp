/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/substring.h"
#include <algorithm>
namespace ng
{

const substring::
	value_type substring::_dummy(0);

substring::
	substring(const_pointer b, const_pointer e)
	: _b(b)
	, _e(e)
{
	require(b <= e);
}

substring::
	substring(const_pointer b, size_t s)
	: _b(b)
	, _e(b + s)
{
}

substring::
	substring(cref_bytevec cbv)
	: _b((const char*)cbv.begin())
	, _e((const char*)cbv.end())
{
}

void substring::
	assign(const_pointer b, const_pointer e)
{
	_b = b;
	_e = e;
	require(b <= e);
}

void substring::
	assign(const_pointer b, size_t s)
{
	_b = b;
	_e = b + s;
}

void substring::
	assign(cref_bytevec cbv)
{
	_b = (const char*)cbv.begin();
	_e = (const char*)cbv.end();
}

substring substring::
	left(size_type n) const
{
	return substring(begin(), std::min(size(), n));
}

substring substring::
	right(size_type n) const
{
	n = std::min(size(), n);
	return substring(end() - n, end());
}

substring substring::
	butfirst(size_type n) const
{
	return right(std::max<size_type>(size() - n, 0));
}

substring substring::
	butlast(size_type n) const
{
	return left(std::max<size_type>(size() - n, 0));
}

substring substring::
	substr(size_type from, size_type n) const
{
	from = std::min(from, size());
	size_type e = std::min(size(), from + n);
	return substring(begin() + from, begin() + e);
}


bool substring::
	operator==(const substring& y) const
{
	size_type s1 = size();
	size_type s2 = y.size();
	if ( s1 != s2 )
		return false;
	return memcmp(_b, y._b, s1) == 0;
}

bool substring::
	operator==(const_pointer y) const
{
	const_pointer it(_b);
	while(it < _e && *y != 0)
	{
		if ( *it != *y )
			return false;
		++it;
		++y;
	}
	return (it == _e) == (*y == 0);
}

bool substring::
	operator<(const substring& y) const
{
	size_type s1 = size();
	size_type s2 = y.size();
	int i = memcmp(_b, y._b, std::min(s1, s2));
	if ( i == 0 )
		return s1 < s2;
	else
		return i < 0;
}

bool substring::
	operator<(const_pointer y) const
{
	const_pointer it(_b);
	while(it < _e && *y != 0)
	{
		if ( *it < *y )
			return true;

		if ( *it > *y )
			return false;

		++it;
		++y;
	}
	return it == _e && *y != 0;
}

bool substring::
	operator>(const_pointer y) const
{
	const_pointer it(_b);
	while(it < _e && *y != 0)
	{
		if ( *it > *y )
			return true;

		if ( *it < *y )
			return false;

		++it;
		++y;
	}
	return it < _e && *y == 0;
}

bool substring::
	operator<=(const substring& y) const
{
	size_type s1 = size();
	size_type s2 = y.size();
	int i = memcmp(_b, y._b, std::min(s1, s2));
	if ( i == 0 )
		return s1 <= s2;
	else
		return i < 0;
}

bool substring::
	operator<=(const_pointer y) const
{
	const_pointer it(_b);
	while(it < _e && *y != 0)
	{
		if ( *it < *y )
			return true;

		if ( *it > *y )
			return false;

		++it;
		++y;
	}
	return it == _e;
}

bool substring::
	operator>=(const_pointer y) const
{
	const_pointer it(_b);
	while(it < _e && *y != 0)
	{
		if ( *it > *y )
			return true;

		if ( *it < *y )
			return false;

		++it;
		++y;
	}
	return *y == 0;
}

std::string substring::
	to_string() const
{
	return std::string(begin(), end());
}

void substring::
	to_string(OUT std::string& s) const
{
	s.assign(begin(), end());
}

}
