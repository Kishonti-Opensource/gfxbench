/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_substring_1360616566
#define INCLUDE_GUARD_substring_1360616566

#include <string.h>
#include <assert.h>

#include <string>

#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"
#include "ng/require.h"
#include "ng/cstring.h"
#include "ng/bytevec.h"

namespace ng
{

class NGRTL_EXPORT substring
{
public:
	typedef char value_type;
	typedef size_t size_type;
	typedef char& reference;
	typedef const char* pointer;
	typedef ptrdiff_t difference_type;
	typedef pointer iterator;

	typedef reference const_reference;
	typedef pointer const_pointer;
	typedef iterator const_iterator;

	typedef std::basic_string<value_type> string_type;

	//ctor/dtor
	substring()
		: _b(&_dummy)
		, _e(&_dummy)
	{
	}

	substring(const_pointer p)
		: _b(p)
		, _e(_b + strlen(p))
	{
	}

	substring(const string_type& s)
		: _b(s.data())
		, _e(s.data() + s.size())
	{
	}

	substring(const_pointer b, const_pointer e);
	substring(const_pointer b, size_t s);
	substring(cref_bytevec bv);
	void assign(const_pointer b, const_pointer e);
	void assign(const_pointer b, size_t s);
	void assign(cref_bytevec bv);

	substring(const cstring& y)
		: _b(y.begin())
		, _e(_b + y.size())
	{
	}

	void clear()
	{
		_b = &_dummy;
		_e = &_dummy;
	}

	//query
	bool empty() const
	{
		return _b == _e;
	}
	const_pointer begin() const
	{
		return _b;
	}
	const_pointer end() const
	{
		return _e;
	}
	size_type size() const
	{
		return _e - _b;
	}
	value_type operator[](size_type i) const
	{
		assert(_b + i < _e);
		return *(_b + i);
	}
	value_type front() const
	{
		require(_b < _e);
		return *_b;
	}
	value_type back() const
	{
		require(_b < _e);
		return _e[-1];
	}
	substring left(size_type n) const;
	substring right(size_type n) const;
	substring substr(size_type from, size_type n) const;
	substring butfirst(size_type n) const;
	substring butlast(size_type n) const;

	bool begins_with(substring y) const
	{ return left(y.size()) == y; }
	bool ends_with(substring y) const
	{ return right(y.size()) == y; }

	bool begins_with(char c) const
	{ return !empty() && (*this)[0] == c; }
	bool ends_with(char c) const
	{ return !empty() && (*this)[size() - 1] == c; }

	//comparison
	bool operator==(const substring& y) const;
	bool operator==(const_pointer y) const;
	bool operator==(const string_type& y) const { return *this == substring(y); }
	bool operator!=(const substring& y) const { return !(*this == y); }
	bool operator!=(const_pointer y) const { return !(*this == y); }
	bool operator!=(const string_type& y) const { return !(*this == y); }

	bool operator<(const substring& y) const;
	bool operator<(const_pointer y) const;
	bool operator<(const string_type& y) const { return *this < substring(y); }
	bool operator>(const substring& y) const { return y < *this; }
	bool operator>(const_pointer y) const;
	bool operator>(const string_type& y) const { return *this > substring(y); }

	bool operator<=(const substring& y) const;
	bool operator<=(const_pointer y) const;
	bool operator<=(const string_type& y) const { return *this <= substring(y); }
	bool operator>=(const substring& y) const { return y <= *this; }
	bool operator>=(const_pointer y) const;
	bool operator>=(const string_type& y) const { return *this >= substring(y); }

	std::string to_string() const;
	void to_string(OUT std::string& s) const;

private:

    const_pointer _b;
    const_pointer _e;

	static const value_type _dummy;
};

inline bool operator==(const char* x, const substring& y) { return y == x; }
inline bool operator==(const substring::string_type& x, const substring& y) { return y == x; }
inline bool operator!=(const char* x, const substring& y) { return y != x; }
inline bool operator!=(const substring::string_type& x, const substring& y) { return y != x; }
inline bool operator<(const char* x, const substring& y) { return y > x; }
inline bool operator<(const substring::string_type& x, const substring& y) { return y > x; }
inline bool operator>(const char* x, const substring& y) { return y < x; }
inline bool operator>(const substring::string_type& x, const substring& y) { return y < x; }
inline bool operator<=(const char* x, const substring& y) { return y >= x; }
inline bool operator<=(const substring::string_type& x, const substring& y) { return y >= x; }
inline bool operator>=(const char* x, const substring& y) { return y <= x; }
inline bool operator>=(const substring::string_type& x, const substring& y) { return y <= x; }

inline substring::const_pointer strchr(substring s, substring::value_type c)
{
	return (substring::const_pointer)memchr(s.begin(), c, s.size());
}

#define NG_SUBSTRING_TO_STD(x) (std::string(x.begin(), x.end()))

}


#endif

