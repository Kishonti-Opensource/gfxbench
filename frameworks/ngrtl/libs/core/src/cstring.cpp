/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/cstring.h"
#ifdef _MSC_VER
#include <string.h>
#define stricmp _stricmp
#else
#include <strings.h>
#define stricmp strcasecmp
#endif


namespace ng
{
	namespace {
	}

	const cstring::
		string_type cstring::empty_string;

	cstring::
		cstring()
		: _uc(UC_SS_WEAK)
	{
		_p.ss = &empty_string;
	}

	cstring::
		cstring(const cstring& y)
	{
		if ( y.cc() )
		{
			_uc = UC_CC;
			_p.cc = y._p.cc;
		} else
		{
			_uc = UC_SS_WEAK;
			_p.ss = y._p.ss;
		}
	}

	cstring::
		~cstring()
	{
		if ( _uc == UC_SS_OWNED )
			delete _p.ss;
	}

	cstring::const_pointer cstring::
		c_str() const
	{
		if ( cc() )
			return _p.cc;
		else
			return _p.ss->c_str();
	}

	cstring::string_type cstring::
		to_string() const
	{
        if (cc())
            return string_type(_p.cc);
        else
    		return *_p.ss;
	}

	bool cstring::
		empty() const
	{
		if ( cc() )
			return *_p.cc == 0;
		else
			return _p.ss->empty();
	}

	cstring::size_type cstring::
		size() const
	{
		if ( cc() )
			return strlen(_p.cc);
		else
			return _p.ss->size();
	}

	cstring::
		value_type cstring::
		operator[](size_type i) const
	{
		assert(i < size());
		if ( cc() )
			return _p.cc[i];
		else
			return _p.ss->at(i);
	}

	cstring::const_iterator cstring::
		begin() const
	{
		if ( cc() )
			return _p.cc;
		else
			return _p.ss->data();
	}

	cstring::const_iterator cstring::
		end() const
	{
		if ( cc() )
			return _p.cc + strlen(_p.cc);
		else
			return _p.ss->data() + _p.ss->size();
	}

	cstring::value_type cstring::
		front() const
	{
		if ( cc() )
		{
			value_type c = *_p.cc;
			require(c != 0, "cstring::front() called on empty string");
			return c;
		} else
		{
			return _p.ss->at(0);
		}
	}

	cstring::value_type cstring::
		back() const
	{
		if ( cc() )
		{
			size_t l = strlen(_p.cc);
			require(l != 0, "cstring::back() called on empty string");
			return _p.cc[l-1];
		} else
		{
			return _p.ss->at(_p.ss->size() - 1);
		}
	}

	bool cstring::
		operator==(const cstring& y) const
	{
		if ( cc() )
		{
			if ( y.cc() )
				return strcmp(_p.cc, y._p.cc) == 0;
			else
				return *y._p.ss == _p.cc;
		} else
		{
			if ( y.cc() )
				return *_p.ss == y._p.cc;
			else
				return *_p.ss == *y._p.ss;
		}
		require(false);
		return false;
	}

	bool cstring::
		operator==(const_pointer y) const
	{
		if ( cc() )
			return strcmp(_p.cc, y) == 0;
		else
			return *_p.ss == y;
	}

	bool cstring::
		operator==(const string_type& y) const
	{
		if ( cc() )
			return y == _p.cc;
		else
			return *_p.ss == y;
	}

	bool cstring::
		operator<(const cstring& y) const
	{
		if ( cc() )
		{
			if ( y.cc() )
				return strcmp(_p.cc, y._p.cc) < 0;
			else
				return *y._p.ss > _p.cc;
		} else
		{
			if ( y.cc() )
				return *_p.ss < y._p.cc;
			else
				return *_p.ss < *y._p.ss;
		}
	}

	bool cstring::
		operator<(const_pointer y) const
	{
		if ( cc() )
			return strcmp(_p.cc, y) < 0;
		else
			return *_p.ss < y;
	}

	bool cstring::
		operator<(const string_type& y) const
	{
		if ( cc() )
			return y > _p.cc;
		else
			return *_p.ss < y;
	}


	bool cstring::
		operator>(const_pointer y) const
	{
		if ( cc() )
			return strcmp(_p.cc, y) > 0;
		else
			return *_p.ss > y;
	}

	bool cstring::
		operator>(const string_type& y) const
	{
		if ( cc() )
			return y < _p.cc;
		else
			return *_p.ss > y;
	}

	bool strieq(const cstring& x, cstring::const_pointer y)
	{
		return stricmp(x.c_str(), y) == 0;
	}
}
