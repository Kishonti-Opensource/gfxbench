/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_FORMAT_INCLUDED
#define NG_FORMAT_INCLUDED

#include <string>
#include <deque>
#include <sstream>

#include "ng/substring.h"

/* Very small subset of boost::format, non-optimized for performance

Use it like this:

	std::string s = str(format("string %s number %s anything %s") % "qweqw" % 12.32 % (char)48);

	someFunctionExpectingConstCharPtr( cstr(format("string %s number %s anything %s") % "qweqw" % 12.32 % (char)48) );
	
The following convenience macros are provided for easy usage:

	std::string s = FORMATSTR("string %s number %s anything %s", "qweqw", 12.32, (char)48);

	someFunctionExpectingConstCharPtr( FORMATCSTR("string %s number %s anything %s"), "qweqw", 12.32, (char)48);

Use %s for all types of arguments. No other formatting options are supported.

*/

namespace ng
{

class NGRTL_EXPORT format
{
public:
	format();
	explicit format(const char* formatString);
	std::string str() const;

	template<typename T>
		format& operator%(const T&);

	format& operator%(const cstring& x) { return operator%(x.c_str()); }
	format& operator%(const substring& x) { return operator%(std::string(x.begin(), x.end())); }
private:
	std::string _formatString;
	typedef std::deque<std::string> ParsedArgs;
	ParsedArgs _parsedArgs;
};

template<typename T>
format& format::
	operator%(const T& x)
{
	std::ostringstream oss;
	oss << x;
	_parsedArgs.push_back(oss.str());
	return *this;
}


inline std::string str(const format& f) { return f.str(); }

class NGRTL_EXPORT cstr
{
public:
	cstr(const format& f) : _s(f.str()) {}
	operator const char*() const { return _s.c_str(); }
private:
	std::string _s;
};

class NGRTL_EXPORT FormatStrHelper
{
public:
	FormatStrHelper(const char* formatString)
		: _format(formatString)
	{}
	FormatStrHelper(const std::string& formatString)
		: _format(formatString.c_str())
	{}
	std::string run() { return _format.str(); }

	template<typename A1>
	std::string run(const A1& a1) { return (_format % a1).str(); }

	template<typename A1, typename A2>
	std::string run(const A1& a1, const A2& a2) { return (_format % a1 % a2).str(); }

	template<typename A1, typename A2, typename A3>
	std::string run(const A1& a1, const A2& a2, const A3& a3) { return (_format % a1 % a2 % a3).str(); }

	template<typename A1, typename A2, typename A3, typename A4>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4) { return (_format % a1 % a2 % a3 % a4).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) { return (_format % a1 % a2 % a3 % a4 % a5).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) { return (_format % a1 % a2 % a3 % a4 % a5 % a6).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) { return (_format % a1 % a2 % a3 % a4 % a5 % a6 % a7).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) { return (_format % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) { return (_format % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8 % a9).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) { return (_format % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8 % a9 % a10).str(); }

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	std::string run(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) { return (_format % a1 % a2 % a3 % a4 % a5 % a6 % a7 % a8 % a9 % a10 % a11).str(); }
private:
	format _format;
};

NGRTL_EXPORT std::string formatDurationSec(double dsec);
NGRTL_EXPORT std::string formatNumberMetric(double d, int nDigitsNeeded = 4);

/* formatInt formats an integer number

   The formatString parameter can be the combination of the following characters:
   (in accordance with the printf format string)

    -:       left-justify
    +:       always print + or -
    <space>: always print ' ' or -
    0:       left pad with zeros
    x:       format as lower-case hex
    X:       format as upper-case hex

	If width = -1, there's no width enforcement.
	If width = 0, '0' will be returned as empty string, otherwise no effect
	If width > 0 and the result is shorter, it will be padded according to
	    the - and 0 flags
*/

template<typename INTTYPE>
std::string NGRTL_EXPORT formatInt(INTTYPE i, const char* formatString = 0, int width = -1);

/* sprintfDouble uses sprintf to a local buffer and
   returns it in std::string

   For the second and third signatures you need to specify the appropriate
   format strings, like:

   sprintfDouble("%*f", 10, 3.14)
   sprintfDouble("%.*f", 10, 3.14)
   sprintfDouble("%*.*f", 20, 10, 3.14)
*/
std::string NGRTL_EXPORT sprintfDouble(const char* formatString, double d);
std::string NGRTL_EXPORT sprintfDouble(const char* formatString, int widthOrPrecision, double d);
std::string NGRTL_EXPORT sprintfDouble(const char* formatString, int width, int precision, double d);

}

#define FORMATSTR(x, ...) (::ng::FormatStrHelper(x).run(__VA_ARGS__))
#define FORMATCSTR(x, ...) (::ng::FormatStrHelper(x).run(__VA_ARGS__).c_str())


#endif
