/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_REQUIRE_INCLUDED
#define NG_REQUIRE_INCLUDED

#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"
#include <stdexcept>
#include <string>
#include <stdio.h>

#ifdef PLATFORM_ANDROID
	#ifdef NDEBUG
		#define ngassert(b) ((void)0)
	#else
		namespace ng { bool ngassert_android(const char* e, const char* fun, const char* file, int line); }
		#define ngassert(b) (void)( (!!(b)) || (::ng::ngassert_android(#b, __FUNCTION__, __FILE__, __LINE__)) )
	#endif
	
	#undef assert
	#define assert(x) ngassert(x)
#else
	#include <assert.h>
	#define ngassert(x) assert(x)
#endif

//require() can be used like assert()
//- in debug builds it is replaced by assert()
//- in release builds it throws a std::runtime_error exception when the condition is false

//requireex() same as require() except always throws (both in debug and release)

//Use require just like asserts, the condition must not have side effects
//This is WRONG:
//	require(fread(...) == 1); //WRONG!!!
//This will work for now but later we may introduce a
//macro to experimantally disable all requires. The programs must
//work correctly without require's.

//require(cond, message) can be used to throw std::runtime_error exceptions
//which contains the message and other data (expr, func, file, line) about the error
//you can use STRFORMAT in the message

//#define NG_REQUIRE_IS_ASSERT
//#define NG_REQUIRE_IS_EXCEPTION

#if !defined(NG_REQUIRE_IS_ASSERT) && !defined(NG_REQUIRE_IS_EXCEPTION)
#ifndef _DEBUG
#define NG_REQUIRE_IS_EXCEPTION
#else
#define NG_REQUIRE_IS_ASSERT
#endif
#endif

#if defined(NG_REQUIRE_IS_ASSERT) && defined(NG_REQUIRE_IS_EXCEPTION)
#error both NG_REQUIRE_IS_ASSERT and NG_REQUIRE_IS_EXCEPTION are defined
#endif

#if !defined(NG_REQUIRE_IS_ASSERT) && !defined(NG_REQUIRE_IS_EXCEPTION)
#error neither NG_REQUIRE_IS_ASSERT nor NG_REQUIRE_IS_EXCEPTION is defined
#endif

namespace ng
{
	class NGRTL_EXPORT require_failed
		: public ::std::exception
	{
	public:
		explicit require_failed(const char* message, const char* details);
		~require_failed() throw() {}

        const char* what() const throw() { return _what.c_str(); }
		const char* details() const { return _details.c_str(); }
		const char* message() const { return _message.c_str(); }
	private:
		std::string _what, _message, _details;
	};
}

#if defined(NG_REQUIRE_IS_ASSERT)
#define require(b,...) (ngassert(b))
#else
#define require(b,...) ((b)?(void)0:(::ng::require_exception_core(#b, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)))
#endif

#define requireex(b,...) ((b)?(void)0:(::ng::require_exception_core(#b, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)))

namespace ng {
NGRTL_EXPORT void require_exception_core(const char* expression, const char* file, int line, const char* function, const char* message);
NGRTL_EXPORT inline void require_exception_core(const char* expression, const char* file, int line, const char* function, const std::string& message)
	{ require_exception_core(expression, file, line, function, message.c_str()); }
NGRTL_EXPORT inline void require_exception_core(const char* expression, const char* file, int line, const char* function)
	{ require_exception_core(expression, file, line, function, 0); }
}

#endif


