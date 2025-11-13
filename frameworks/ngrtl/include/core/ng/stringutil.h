/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_stringutil_1359050238
#define INCLUDE_GUARD_stringutil_1359050238

#include "ng/ngrtl_core_export.h"
#include <stdint.h>
#include <string>
#include <vector>

#include "ng/substring.h"
#include "ng/noncopyable.h"

#include "ng/detail/strutil_detail.h"

namespace ng {

	//Variation of strtok
	//Changes:
	// - doesn't alter input string
	// - instead return vector of substring which refer into original string
	NGRTL_EXPORT std::vector<substring> strtok(substring str, substring delim);

	class itoabuf_t;

	//Itoa variations
	//First group: works into supplied itoabuf, returns const char* (use that one, buf.begin() won't work)
	inline const char* itoa(int32_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa(value, buf, base); }
	inline const char* itoa(uint32_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa(value, buf, base); }
	inline const char* itoa(int64_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa(value, buf, base); }
	inline const char* itoa(uint64_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa(value, buf, base); }

	//Second group: works into supplied itoabuf, returns substring
	inline substring itoa_ss(int32_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa_ss(value, buf, base); } 
	inline substring itoa_ss(uint32_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa_ss(value, buf, base); } 
	inline substring itoa_ss(int64_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa_ss(value, buf, base); } 
	inline substring itoa_ss(uint64_t value, itoabuf_t& buf, int base) { return strutil_detail::itoa_ss(value, buf, base); } 

	//Third group: returns std::string, needs no itoabuf
	NGRTL_EXPORT std::string itoa(int32_t value, int base);
	NGRTL_EXPORT std::string itoa(uint32_t value, int base);
	NGRTL_EXPORT std::string itoa(int64_t value, int base);
	NGRTL_EXPORT std::string itoa(uint64_t value, int base);

	//About itoabuf:
	//Use this types (declared at the end of the file):
	//
	//itoabuf_X_B holds enough number of chars stored in intX_t or uintX_t
	//and converting to base B
	//
	//typedef itoabuf_n<34> itoabuf_32_2;
	//typedef itoabuf_n<12> itoabuf_32_10;
	//typedef itoabuf_n<10> itoabuf_32_16;
	//typedef itoabuf_n<66> itoabuf_64_2;
	//typedef itoabuf_n<21> itoabuf_64_10;
	//typedef itoabuf_n<18> itoabuf_64_16;

	//Interpret whole string as one integer, exception or !*bOk on failure
	NGRTL_EXPORT int32_t atoi(substring ss, OUT bool* bOk = 0);
	NGRTL_EXPORT uint32_t atoui(substring ss, OUT bool* bOk = 0);
	NGRTL_EXPORT int64_t atoi64(substring ss, OUT bool* bOk = 0);
	NGRTL_EXPORT uint64_t atoui64(substring ss, OUT bool* bOk = 0);

	//Interpret whole string as one double, exception or !*bOk on failure
	NGRTL_EXPORT double atof(substring ss, OUT bool* bOk = 0);

	//Begin parsing at ss.begin(), report end of parsing (endptr)
	//Skips leading whitespaces.
	//bOk or exception on failure
	//Examples:
	//"12", "-123", "1234a": ok (negative sign is only for signed versions)
	//"", "xy": error, no number found
	//"4294967296": error for int32_t, overflow
	NGRTL_EXPORT int32_t strtoi(substring ss, bool* bOk = 0);
	NGRTL_EXPORT int32_t strtoi(substring ss, const char** endptr, bool* bOk = 0);
	NGRTL_EXPORT uint32_t strtoui(substring ss, bool* bOk = 0);
	NGRTL_EXPORT uint32_t strtoui(substring ss, const char** endptr, bool* bOk = 0);
	NGRTL_EXPORT int64_t strtoi64(substring ss, bool* bOk = 0);
	NGRTL_EXPORT int64_t strtoi64(substring ss, const char** endptr, bool* bOk = 0);
	NGRTL_EXPORT uint64_t strtoui64(substring ss, bool* bOk = 0);
	NGRTL_EXPORT uint64_t strtoui64(substring ss, const char** endptr, bool* bOk = 0);

	//instantiated for int/uint 32/64
	template<typename T>
	class NGRTL_EXPORT strtoi_t
	{
	public:
        /*
		strtoi_t(); //base = 10
		explicit strtoi_t(int base);
         */
        
        strtoi_t()
        : _base(10)
        , _bOk(false)
        , _end(0)
        , _result(0)
        {}
        
		explicit strtoi_t(int base)
        : _base(base)
        , _bOk(false)
        , _end(0)
        , _result(0)
        {}

		bool convert(const substring& ss);

		T result() const { return _result; }
		bool ok() const { return _bOk; }
		const char* end() const { return _end; }
	private:
		const int _base;
		bool _bOk;
		const char* _end;
		T _result;
	};

	class NGRTL_EXPORT itoabuf_t
		: public ng::noncopyable
	{
	public:
		char* begin() { return _b; }
		char* end() { return _e; }
		size_t size() { return _e - _b; }
	protected:
		itoabuf_t(char* buf, size_t s)
			: _b(buf)
			, _e(buf + s)
		{}
		char* _b;
		char* _e;
	};

	template<int N>
	class NGRTL_EXPORT itoabuf_n
		: public itoabuf_t
	{
	public:
		itoabuf_n()
			: itoabuf_t(buf, N)
		{}
	private:
		char buf[N];
	};

	typedef itoabuf_n<34> itoabuf_32_2;
	typedef itoabuf_n<12> itoabuf_32_10;
	typedef itoabuf_n<10> itoabuf_32_16;
	typedef itoabuf_n<66> itoabuf_64_2;
	typedef itoabuf_n<21> itoabuf_64_10;
	typedef itoabuf_n<18> itoabuf_64_16;

	/*
	int64_t atoi64(const char *v);

	substring trim_nongraph(const substring& s);
	
	*/

	NGRTL_EXPORT substring trim_nongraph(const substring& s);

	NGRTL_EXPORT std::string join_strings(const std::vector<std::string>& sl, const substring& delimiter);
	NGRTL_EXPORT std::string join_strings(const std::vector<substring>& sl, const substring& delimiter);

	NGRTL_EXPORT void tolower(INOUT std::string& s);
	NGRTL_EXPORT void toupper(INOUT std::string& s);
	NGRTL_EXPORT std::string tolower_copy(substring s);
	NGRTL_EXPORT std::string toupper_copy(substring s);
}



#endif

