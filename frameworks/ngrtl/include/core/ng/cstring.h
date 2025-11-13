/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_CSTRING_INCLUDED
#define NG_CSTRING_INCLUDED

#include <stddef.h>
#include <assert.h>
#include <string.h>

#include <string>
#include "ng/ngrtl_core_export.h"
#include "ng/require.h"

/* ---- class ng::cstring ---- (const string)

class cstring's sole purpose is to allow functions and method parameters to accept both
std::string and const char*. It's an adaptor:

- it can be constructed with both std::string and const char*
- it can be used in place of an std::string but to use as a const char* you need to use the .c_str() method

The reason it does not provide auto conversion to const char* is to prevent such errors (see guru of the week):

cstring a, b;
cstring c = a + b;

class cstring is not for storaging strings! It is as safe as a const char*. The source data it was constructed from
must remain valid throughout the object's lifetime.

*/
namespace ng
{

class NGRTL_EXPORT cstring
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
	cstring();

	cstring(const_pointer cc)
	: _uc(UC_CC)
	{
		_p.cc = cc;
	}
	
	cstring(const string_type& ss)
	: _uc(UC_SS_WEAK)
	{
		_p.ss = &ss;
	}

	cstring(const cstring& y);
	
	~cstring();
	

	//conversion
	const_pointer c_str() const;
	string_type to_string() const;

	//query
	bool empty() const;
	size_type size() const;

	const_iterator begin() const;
	const_iterator end() const;

	value_type front() const;
	value_type back() const;

	value_type operator[](size_type i) const;

	//comparison
	bool operator==(const cstring& y) const;
	bool operator==(const_pointer y) const;
	bool operator==(const string_type& y) const;
	bool operator!=(const cstring& y) const { return !(*this == y); }
	bool operator!=(const_pointer y) const { return !(*this == y); }
	bool operator!=(const string_type& y) const { return !(*this == y); }

	bool operator<(const cstring& y) const;
	bool operator<(const_pointer y) const;
	bool operator<(const string_type& y) const;
	bool operator>(const cstring& y) const { return y < *this; }
	bool operator>(const_pointer y) const;
	bool operator>(const string_type& y) const;

private:
	typedef cstring this_t;

	enum data_type_enum
	{
		UC_CC,
		UC_SS_WEAK,
		UC_SS_OWNED
	};

	union data_union
	{
		const_pointer cc;
		const string_type* ss;
	};
	
	data_union _p;
	data_type_enum _uc;

	static const string_type empty_string;

	bool cc() const { return _uc == UC_CC; }
};

inline bool operator==(const char* x, const cstring& y) { return y == x; }
inline bool operator==(const cstring::string_type& x, const cstring& y) { return y == x; }
inline bool operator!=(const char* x, const cstring& y) { return y != x; }
inline bool operator!=(const cstring::string_type& x, const cstring& y) { return y != x; }
inline bool operator<(const char* x, const cstring& y) { return y > x; }
inline bool operator<(const cstring::string_type& x, const cstring& y) { return y > x; }
inline bool operator>(const char* x, const cstring& y) { return y < x; }
inline bool operator>(const cstring::string_type& x, const cstring& y) { return y < x; }

// cause compiler error with XCode 7.3 (7D175): "All paths through this function will call itself"
//inline bool strieq(const cstring& x, const cstring& y) { return strieq(x, y.c_str()); }

bool strieq(const cstring& x, cstring::const_pointer y);
inline bool strieq(const cstring& x, const cstring::string_type& y) { return strieq(x, y.c_str()); }
inline bool strieq(const char* x, const cstring& y) { return strieq(y, x); }
inline bool strieq(const cstring::string_type& x, const cstring& y) { return strieq(y, x); }

inline cstring::const_pointer strchr(const cstring& s, cstring::value_type c)
{
	return ::strchr(s.c_str(), c);
}

}

#if 0
namespace ng {


template<typename C>
class basic_cstring
{
public:
	typedef C value_type;
	typedef C* pointer;
	typedef const C* const_pointer;
	typedef const C& reference;
	typedef const C& const_reference;
	typedef ptrdiff_t size_type; //so it's signed
	typedef ptrdiff_t difference_type;
	static const size_type npos = -1;
	typedef const C* iterator;
	typedef iterator const_iterator;
	//typedef TODO reverse_iterator;
	//typedef reverse_iterator const_reverse_iterator;

	iterator begin() const
	{ return _v; }

	iterator end() const
	{ return _v + size(); }

	//reverse_iterator rbegin() const;
	//reverse_iterator rend() const;

	size_type size() const
	{
		if ( _s == npos )
		{
			assert(_bZeroTerm);
			const C* p(_v);
			while(*p) ++p;
			_s = p - _v;
		}
		return _s;
	}

	size_type length() const
	{ return size(); }

	bool empty() const
	{ return _bZeroTerm ? *_v == 0 : _s == 0; }

	reference operator[](size_type n) const
	{ assert(0 <= n && n < size()); return _v[n]; }

	const C* c_str() const
	{
		if ( !_bZeroTerm ) //then clone to make it zero-term
		{
			basic_cstring tmp(*this, true);
			((basic_cstring*)this)->swap(tmp); //temp remove constness
		}
		return _v;
	}

	std::basic_string<C> string() const
	{
		return std::basic_string<C>(_v, size());
	}

	void string(OUT std::basic_string<C>& s) const
	{
		s.assign(_v, size());
	}

	const C* data() const
	{ return _v; }
	
	basic_cstring()
		: _v(nullptr)
		, _s(0)
		, _bOwner(0)
		, _bZeroTerm(1)
	{
		static const C _zero = 0;
		_v = &_zero;
	}

	void clear()
	{
		static const C _zero = 0;
		_v = &_zero;
		_s = 0;
		_bOwner = 0;
		_bZeroTerm = 1;
	}

	basic_cstring(const basic_cstring<C>& y, bool bClone = false)
	: _v(bClone ? new C[y.size() + 1] : y._v)
	, _s(bClone ? y.size() : y._s) //y._s does not calculate size if zero term and unknown size
	, _bOwner(bClone)
	, _bZeroTerm(bClone || y._bZeroTerm)
	{
		if ( bClone )
		{
			for(size_type i = 0; i < _s; ++i)
				(C&)_v[i] = y._v[i];
			(C&)_v[_s] = 0;
		}
	}

	basic_cstring& operator=(const basic_cstring& y)
	{
		basic_cstring z(y);
		swap(z);
		return *this;
	}

	basic_cstring(const C* y)
	: _v(y)
	, _s(npos)
	, _bOwner(0)
	, _bZeroTerm(1)
	{}

	basic_cstring(const C* y, size_type n, bool bZeroTerm = false)
	: _v(y)
	, _s(n)
	, _bOwner(0)
	, _bZeroTerm(bZeroTerm ? 1 : 0)
	{}

	basic_cstring(const std::basic_string<C>& s)
		: _v(s.c_str())
		, _s(s.size())
		, _bOwner(0)
		, _bZeroTerm(1)
	{
	}

	basic_cstring(size_type n, C c)
	: _v(new C[n+1])
	, _s(n)
	, _bOwner(1)
	, _bZeroTerm(1)
	{
		for(size_type i = 0; i < n; ++i)
			_v[i] = c;
		_v[_s] = 0;
	}

	basic_cstring(const C* first, const C* last)
	: _v(first)
	, _s(last - first)
	, _bOwner(0)
	, _bZeroTerm(0)
	{
		require(_s >= 0);
	}

	void assign(const C* first, const C* last)
	{
		_v = first;
		_s = last - first;
		_bOwner = 0;
		_bZeroTerm = 0;
		require(_s >= 0);
	}

	template <class InputIterator>
	basic_cstring(InputIterator first, InputIterator last)
	: _v(new C[last - first + 1])
	, _s(last - first)
	, _bOwner(1)
	, _bZeroTerm(1)
	{
		require(_s >= 0);
		pointer p((pointer)_v);
		for(; first != last; ++first, ++p)
			*p = *first;
		*p = 0;
	}
	
	static basic_cstring make_clone(const basic_cstring& y)
	{
		return basic_cstring(y, true);
	}
	static basic_cstring make_copy(const C* y)
	{
		return basic_cstring(basic_cstring(y), true);
	}
	static basic_cstring make_copy(const C* y, size_type n, bool bZeroTerm = false)
	{
		return basic_cstring(basic_cstring(y, n, bZeroTerm), true);
	}
	~basic_cstring()
	{
		if ( _bOwner )
			delete _v;
	}
	const C& front() const { assert(!empty()); return *_v; }
	const C& back() const { assert(!empty()); return *(_v + size() - 1); }

	void swap(basic_cstring& y)
	{
		{ const C* t = _v; _v = y._v; y._v = t; }
		{ size_type t = _s; _s = y._s; y._s = t; }
		{ bool t = _bOwner != 0; _bOwner = y._bOwner; y._bOwner = t; }
		{ bool t = _bZeroTerm != 0; _bZeroTerm = y._bZeroTerm; y._bZeroTerm = t; }
	}
	basic_cstring substr(size_type pos = 0, size_type n = npos) const
	{
		if ( n != npos )
			return basic_cstring(_v + pos, _v + pos + n);
		else
		{
			if ( _bZeroTerm )
			{
				if ( _s == npos )
					return basic_cstring(_v + pos);
				else
					return basic_cstring(_v + pos, _s, true);
			} else
				return basic_cstring(_v + pos, _v + _s - pos);
		}
	}

	bool operator<(const basic_cstring& y) const
	{
		for(size_type i = 0; ; ++i)
		{
			bool end1 = _s != npos ? i >= _s : _v[i] == 0;
			bool end2 = y._s != npos ? i >= y._s : y._v[i] == 0;

			if ( end1 || end2)
				return end1 && !end2;
			else
			{
				C c1 = _v[i];
				C c2 = y._v[i];
				if ( c1 < c2 )
					return true;
				else if ( c1 > c2 )
					return false;
			}
		}
	}

	bool operator!=(const basic_cstring& y) const
	{
		return !(*this == y);
	}

	bool operator==(const basic_cstring& y) const
	{
		if ( _s != npos && y._s != npos && _s != y._s )
			return false;
		for(size_type i = 0; ; ++i)
		{
			bool end1 = _s != npos ? i >= _s : _v[i] == 0;
			bool end2 = y._s != npos ? i >= y._s : y._v[i] == 0;

			if ( end1 || end2)
				return end1 == end2;
			else
			{
				C c1 = _v[i];
				C c2 = y._v[i];
				if ( c1 != c2 )
					return false;
			}
		}
	}

private:
	mutable const C* _v;
	mutable size_type _s : (8 * sizeof(size_type) - 2); //==npos if zero term and not yet calculated
	mutable size_type _bOwner : 1;
	mutable size_type _bZeroTerm : 1;
};

typedef basic_cstring<char> cstring;
//typedef basic_cstring<uchar> ucstring;

template<typename C>
bool operator==(const C* x, const basic_cstring<C>& y)
{
	return basic_cstring<C>(x) == y;
}

template<typename C>
bool operator==(const std::basic_string<C>& x, const basic_cstring<C>& y)
{
	return basic_cstring<C>(x) == y;
}

template<typename C>
bool operator!=(const std::basic_string<C>& x, const basic_cstring<C>& y)
{
	return basic_cstring<C>(x) != y;
}

template<typename C>
bool operator<(const C* x, const basic_cstring<C>& y)
{
	return basic_cstring<C>(x) < y;
}

template<typename C>
bool operator<(const std::string& x, const basic_cstring<C>& y)
{
	return basic_cstring<C>(x) < y;
}

}
#endif

#endif
