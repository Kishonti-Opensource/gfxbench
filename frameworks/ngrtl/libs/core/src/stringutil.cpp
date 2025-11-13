/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/stringutil.h"
#include "ng/require.h"
#include <sstream>
#include <iomanip>  // setbase(radix)
#include <cstring>  // strcpy
#include <algorithm>
#include <limits>

#include "ng/format.h"
#include "ng/macros.h"

#ifdef isascii
#undef isascii
#endif

namespace ng {

	inline bool isascii(char c)
	{
		return (c & 128) == 0;
	}

	substring trim_nongraph(const substring& s)
	{
		const char* b = s.begin();
		const char* e = s.end();

		while(b != e && isascii(*b) && !isgraph(*b))
			++b;

		if ( b != e )
		{
			--e;
			while(isascii(*e) && !isgraph(*e))
				--e;
			++e;
		}

		return substring(b, e);
	}

namespace strutil_detail
{
	template<typename T>
	const char* itoa_base(T value, itoabuf_t& buf, int base)
	{
		require(2 <= base && base <= 36);

		char* ptr = buf.end();
		*(--ptr) = 0;

		T tmp_value;
		do {
			tmp_value = value;
			value /= base;
			assert(buf.begin() < ptr);
			*(--ptr) = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0)
		{
			assert(buf.begin() < ptr);
			*(--ptr) = '-';
		}
		return ptr;
	}

	template<typename T>
	const char* itoa(T value, itoabuf_t& buf, int base)
	{
		return itoa_base(value, buf, base);
	}

	template<typename T>
	substring itoa_ss(T value, itoabuf_t& buf, int base)
	{
		return substring(itoa_base(value, buf, base), buf.end() - 1);
	}

	template
	const char* itoa<int32_t>(int32_t value, itoabuf_t& buf, int base);
	template
	const char* itoa<uint32_t>(uint32_t value, itoabuf_t& buf, int base);
	template
	const char* itoa<int64_t>(int64_t value, itoabuf_t& buf, int base);
	template
	const char* itoa<uint64_t>(uint64_t value, itoabuf_t& buf, int base);

	template
	substring itoa_ss<int32_t>(int32_t value, itoabuf_t& buf, int base);
	template
	substring itoa_ss<uint32_t>(uint32_t value, itoabuf_t& buf, int base);
	template
	substring itoa_ss<int64_t>(int64_t value, itoabuf_t& buf, int base);
	template
	substring itoa_ss<uint64_t>(uint64_t value, itoabuf_t& buf, int base);
}

std::vector<substring> strtok(substring str, substring delim)
{
	std::vector<substring> v;
	const char* b = str.begin();
	for(;;)
	{
		while(b != str.end() && strchr(delim, *b) != 0)
			++b;
		if ( b == str.end() )
			break;
		//b is at non-separator
		const char* e(b+1);
		while(e != str.end() && strchr(delim, *e) == 0)
			++e;
		v.push_back(substring(b, e));
		b = e;
	}
	return NG_MOVE_IF_SUPPORTED(v);
}

std::string itoa(int32_t value, int base)
{
	ng::itoabuf_32_2 buf;
	substring ss = strutil_detail::itoa_ss(value, buf, base);
	std::string s(std::string(ss.begin(), ss.end()));
	return NG_MOVE_IF_SUPPORTED(s);
}

std::string itoa(uint32_t value, int base)
{
	ng::itoabuf_32_2 buf;
	substring ss = strutil_detail::itoa_ss(value, buf, base);
	std::string s(std::string(ss.begin(), ss.end()));
	return NG_MOVE_IF_SUPPORTED(s);
}

std::string itoa(int64_t value, int base)
{
	ng::itoabuf_64_2 buf;
	substring ss = strutil_detail::itoa_ss(value, buf, base);
	std::string s(std::string(ss.begin(), ss.end()));
	return NG_MOVE_IF_SUPPORTED(s);
}

std::string itoa(uint64_t value, int base)
{
	ng::itoabuf_64_2 buf;
	substring ss = strutil_detail::itoa_ss(value, buf, base);
	std::string s(std::string(ss.begin(), ss.end()));
	return NG_MOVE_IF_SUPPORTED(s);
}

	inline int char_to_digit(char c, int base)
	{
		int d = -1;
		if ( '0' <= c && c <= '9' )
			d = c - '0';
		else if ( 'a' <= c && c <= 'z' )
			d = c - 'a' + 10;
		else if ( 'A' <= c && c <= 'Z' )
			d = c - 'A' + 10;
		else
			return -1;
		return d < base ? d : -1;
	}

	template class strtoi_t<int32_t>;
	template class strtoi_t<uint32_t>;
	template class strtoi_t<int64_t>;
	template class strtoi_t<uint64_t>;
/*
	template<typename T>
	strtoi_t<T>::
    strtoi_t()
    : _base(10)
    , _bOk(false)
    , _end(0)
    , _result(0)
	{}

	template<typename T>
	strtoi_t<T>::
		strtoi_t(int base)
        : _base(base)
        , _bOk(false)
        , _end(0)
        , _result(0)
	{}
*/
	template<typename T>
	bool strtoi_t<T>::
		convert(const substring& ss)
	{
		_bOk = false;
		_result = 0;
		bool bNegative = false;
		_end = ss.begin();
		while(_end < ss.end() && isspace(*_end))
			++_end;
		if ( _end == ss.end() )
		{
			return false;
		}
		if ( *_end == '-' )
		{
			if ( !std::numeric_limits<T>::is_signed )
				return false;
			bNegative = true;
			++_end;
		} else if ( *_end == '+' )
			++_end;

		const char* first_digit = _end;
		bool bOverflow = false;
		if ( !std::numeric_limits<T>::is_signed || !bNegative ) //is_signed helps optimizing away the if when unsigned
		{
			const T max_over_base = std::numeric_limits<T>::max() / _base;
			for(; _end != ss.end(); ++_end)
			{
				int d = char_to_digit(*_end, _base);
				if ( d < 0 )
					break;
				//we're about to add a new digit
				if ( max_over_base < _result )
				{
					bOverflow = true;
					break; //overflow
				}
				_result *= _base;
				if ( _result > std::numeric_limits<T>::max() - d )
				{
					bOverflow = true;
					break; //overflow
				}
				_result += d;
			}
		} else
		{
			T min_over_base = std::numeric_limits<T>::min() / _base;
			for(; _end != ss.end(); ++_end)
			{
				int d = char_to_digit(*_end, _base);
				if ( d < 0 )
					break;
				//we're about to add a new digit
				if ( min_over_base > _result )
				{
					bOverflow = true;
					break; //negative overflow
				}
				_result *= _base;
				if ( _result < std::numeric_limits<T>::min() + d )
				{
					bOverflow = true;
					break; //overflow
				}
				_result -= d;
			}
		}
		ptrdiff_t digit_count = _end - first_digit;

		//first digit nonzero only if one digit
		_bOk = !bOverflow && digit_count > 0 && (digit_count == 1 || *first_digit != '0');

		if ( !_bOk )
			_result = 0;

		return _bOk;
	}


double atof(substring ss, bool* bOk)
{
	//use local buffer or std::string
	const int c_bufsize = 32;
	char buf[c_bufsize];
	const char* input(0);
	std::string str;
	if ( ss.size() < c_bufsize - 1 )
	{
		memcpy(buf, ss.begin(), ss.size());
		buf[ss.size()] = 0;
		input = buf;
	} else
	{
		ss.to_string(OUT str);
		input = str.c_str();
	}

	char* endptr(0);
	double d = strtod(input, &endptr);
	if ( endptr == input || endptr - input != ss.size() )
	{
		if ( bOk == 0 )
			throw std::runtime_error(FORMATSTR("Can't convert to double: \"%s\"", ss));
		else
		{
			*bOk = false;
			return 0;
		}
	}

	if ( bOk )
		*bOk = true;
	return d;
}

#define D(X,Y) \
	X ato##Y(substring ss, bool* bOk) \
	{ \
		strtoi_t<X> s; \
		bool b = s.convert(ss) && s.end() == ss.end(); \
		if ( bOk != nullptr ) \
			*bOk = b; \
		else \
			require(b, FORMATCSTR("atoi: failed to parse \"%s\"", ss)); \
		return s.result(); \
	} \
	X strto##Y(substring ss, const char** endptr, bool* bOk) \
	{ \
		strtoi_t<X> s; \
		bool b = s.convert(ss); \
		*endptr = s.end(); \
		if ( bOk != nullptr ) \
			*bOk = b; \
		else \
			require(b, FORMATCSTR("strtoi: failed to parse \"%s\"", ss)); \
		return s.result(); \
	} \
	X strto##Y(substring ss, bool* bOk) \
	{ \
		strtoi_t<X> s; \
		bool b = s.convert(ss); \
		if ( bOk != nullptr ) \
			*bOk = b; \
		else \
			require(b, FORMATCSTR("strtoi: failed to parse \"%s\"", ss)); \
		return s.result(); \
	}

	D(int32_t, i)
	D(uint32_t, ui)
	D(int64_t, i64)
	D(uint64_t, ui64)

#undef D

	std::string join_strings(const std::vector<std::string>& sl, const substring& delimiter)
	{
		std::string s;
		for(size_t i = 0; i < sl.size(); ++i)
		{
			if ( i != 0 )
				s.insert(s.end(), delimiter.begin(), delimiter.end());
			s += sl[i];
		}
		return NG_MOVE_IF_SUPPORTED(s);
	}

	std::string join_strings(const std::vector<substring>& sl, const substring& delimiter)
	{
		std::string s;
		for(size_t i = 0; i < sl.size(); ++i)
		{
			if ( i != 0 )
				s.insert(s.end(), delimiter.begin(), delimiter.end());
			s.insert(s.end(), sl[i].begin(), sl[i].end());
		}
		return NG_MOVE_IF_SUPPORTED(s);
	}

	void tolower(INOUT std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	}

	void toupper(INOUT std::string& s)
	{
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	}

	std::string tolower_copy(substring s)
	{
		std::string t(s.size(), 0);
		std::transform(s.begin(), s.end(), t.begin(), ::tolower);
		return NG_MOVE_IF_SUPPORTED(t);
	}

	std::string toupper_copy(substring s)
	{
		std::string t(s.size(), 0);
		std::transform(s.begin(), s.end(), t.begin(), ::toupper);
		return NG_MOVE_IF_SUPPORTED(t);
	}

}
