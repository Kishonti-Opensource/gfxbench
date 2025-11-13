/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_BASE_MACROS_INCLUDED
#define NG_BASE_MACROS_INCLUDED
// ---------------------
// IDENTIFY THE PLATFORM
//
// defines one of these: PLATFORM_WINDOWS, PLATFORM_MINGW, PLATFORM_ANDROID, PLATFORM_IPHONE, PLATFORM_MAC
// one of these: PLATFORM_32, PLATFORM_64
// PLATFORM_LINUX is defined on linux platforms
//
//
// ---------------------

#ifdef __MINGW32__
	#define PLATFORM_MINGW
	#ifdef __MINGW64__
		#define PLATFORM_64
	#else
		#define PLATFORM_32
	#endif
#elif defined(_WIN32)
	#ifdef _MSC_VER
		#pragma warning(disable: 4275) //C4275: non dll - interface class 'std::exception' used as base for dll - interface class 'ng::SystemError'
		#define PLATFORM_WINDOWS
	#else
		#error _WIN32 defined but nor _MSC_VER neither __MINGW32__
	#endif
	#ifndef _WIN64
		#define PLATFORM_32
	#else
		#define PLATFORM_64
	#endif
#elif defined(__ANDROID__)
	#define PLATFORM_ANDROID
#elif defined (__APPLE_CPP__) || defined(__APPLE_CC__)
	//need another way to detect iphone without including corefoundation
#include <TargetConditionals.h>
	//#include <CoreFoundation/CoreFoundation.h>
	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		#define PLATFORM_IPHONE
	#else
		#define PLATFORM_APPLE
	#endif
#elif defined(__CYGWIN__)
	#define PLATFORM_CYGWIN
#elif defined(NACL)
	#define PLATFORM_NACL
#elif defined(EMSCRIPTEN)
	#define PLATFORM_EMSCRIPTEN
#elif defined (LINUX) || defined (__linux__)
	#define PLATFORM_LINUX
#elif defined __QNX__
	#define PLATFORM_QNX
#else
	#error Please complete the macros.h with the detection of your platform
#endif

#if !defined(PLATFORM_32) && !defined(PLATFORM_64)
 #if defined (__LP64__) || defined (_LP64)
  #define PLATFORM_64
 #else
  #define PLATFORM_32
 #endif
#endif

#if !defined(PLATFORM_32) && !defined(PLATFORM_64)
#error Please complete macros.h with the detection of 32/64bit for your platform
#endif

///////////////////////////
// DEBUG/RELEASE definitions
///////////////////////////

//The default is release build.
//Please use the macro _DEBUG to indicate debug builds.
//NDEBUG is checked to make sure everything's ok.

#if defined(NDEBUG) && defined(_DEBUG)
#error Please make sure either NDEBUG or _DEBUG is defined
#endif

//////////////
// Misc macros
//////////////

// Should be elaborated
#define NOEXCEPT

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef OUT
#define OUT //marks output parameter
#endif

#ifndef INOUT
#define INOUT //marks inout parameter (must be initialized and returns result)
#endif

#ifndef SWAPIN
#define SWAPIN //marks a swapped-in input parameter, output is undefined (usually empty)
#endif

#ifndef OWNED
#define OWNED //marks pointer with ownership
#endif

#define GETTER(T,N) const T& N() const { return _##N; } //const-ref getter
#define GETTERV(T,N) T N() const { return _##N; } //value-getter
#define SETTER(T,N) void set##N(const N& n) { _##N = n; }

#define JOIN_ARGS2(x, y) x##y
#define JOIN_ARGS(x, y) JOIN_ARGS2(x, y)

//////////////
// SCOPED_ENUM
//////////////
//  Sample usage:
//
//     ENUM_CLASS(algae) { green, red, cyan }; ENUM_CLASS_END
//     ...
//     ENUM(algae) sample( algae::red );
//     void foo( ENUM(algae) color );
//     ...
//     sample = algae::green;
//     foo( algae::cyan );

#define ENUM_CLASS(name) struct name { enum enum_type
#define ENUM_CLASS_END };
#define ENUM(name) name::enum_type

// Better SCOPED_ENUM
//  Sample usage:
//
//     struct algaecolors { enum color { green, red, cyan }; };
//     typedef ScopedEnum<algaecolors,algaecolors::color> algae;
//     ...
//     algae sample( algae::red );
//     void foo( algae color ) {}
//	   ...
//     sample = algae::green;
//     foo( algae::cyan );

//TODO: add similar macros as above, to make it possible to write similar simplified enum definitions

template <typename Scope, typename Type>
struct ScopedEnum
	: public Scope
{
	ScopedEnum(Type value)
		: value(value)
	{}
	Type value;
};

////////////////////////////
// COMPILER-DEPENDENT MACROS
////////////////////////////
//this  list, if you need to use them, write an #ifdef for your platform
//and define it there
//#define NG_NO_STATIC_ASSERT //define this if compiler doesn't support the static_assert keyword
//#define NG_NO_OVERRIDE //define this if compiler doesn't support the override keyword
//#define NG_NO_NULLPTR //define this if compiler doesn't support the nullptr keyword


#ifdef _MSC_VER
#if _MSC_VER < 1600
#define NG_NO_STATIC_ASSERT
#define NG_NO_OVERRIDE //define this if compiler doesn't support the override keyword
#define NG_NO_NULLPTR
#else
#define NG_HAS_MOVE
#endif

#if _MSC_VER >= 1600
#define NG_HAS_CPP11
#endif

#endif

#if defined(__GNUC__)
	#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#if GCC_VERSION < 40201 && (!defined(__GXX_EXPERIMENTAL_CXX0X__) || GCC_VERSION < 40600)
		#define NG_NO_NULLPTR
	#endif
	#if __cplusplus < 201103L && !defined(__GXX_EXPERIMENTAL_CXX0X__) 
		#define NG_NO_STATIC_ASSERT
		#define NG_NO_NULLPTR
	#endif
#endif

#ifdef static_assert
#undef NG_NO_STATIC_ASSERT
#endif

#ifdef nullptr
#undef NG_NO_NULLPTR
#endif

#ifdef NG_NO_STATIC_ASSERT
namespace ng_static_assert
{
	template <bool x> struct STATIC_ASSERTION_FAILURE;
	template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
}
#define static_assert( B, C ) \
   enum { JOIN_ARGS(ngbase_static_assert_enum_, __LINE__) \
      = sizeof(::ng_static_assert::STATIC_ASSERTION_FAILURE< (bool)( B ) >) }
#endif

#ifdef NG_NO_OVERRIDE
#define override
#endif

#ifdef NG_NO_NULLPTR
#define nullptr (0)
#endif

#ifdef NG_HAS_MOVE
	#define NG_MOVE_IF_SUPPORTED(x) (std::move(x))
#else
	#define NG_MOVE_IF_SUPPORTED(x) (x)
#endif


//one of these must be defined
//currently all known architecture are little-endian
//endianness is checked at startup-time in a global object in macros.cpp
//#define NG_BIG_ENDIAN
#define NG_LITTLE_ENDIAN

#if defined(NG_LITTLE_ENDIAN) && defined(NG_BIG_ENDIAN)
#error both NG_LITTLE_ENDIAN and NG_BIG_ENDIAN are defined
#endif
#if !defined(NG_LITTLE_ENDIAN) && !defined(NG_BIG_ENDIAN)
#error neither NG_LITTLE_ENDIAN nor NG_BIG_ENDIAN are defined
#endif

#define NG_STRINGIZE(x) #x
#define NG_EXPAND_STRINGIZE(x) NG_STRINGIZE(x)
#define NG_CURRENT_LINE_INFO "function: [" __FUNCTION__ "], file: [" __FILE__ "]#" NG_EXPAND_STRINGIZE(__LINE__)

#define NG_FOR_EACH(IT, CONT) for(auto IT = (CONT).begin(); IT != (CONT).end(); ++IT)

#endif



