# autoconfigure([output-config_h-filename])
# creates autoconf-like variables and optionally configures a config.h file
# if no file given, only sets variables
macro(autoconfigure)

	include(CheckIncludeFileCXX)
	include(CheckIncludeFile)
	include(CheckSymbolExists)
	include(CheckCXXSourceCompiles)
	include(TestBigEndian)

	check_include_file_cxx(chrono HAVE_CHRONO)
	check_include_file_cxx(mutex HAVE_MUTEX)
	check_include_file_cxx(thread HAVE_THREAD)

	check_include_file(Windows.h HAVE_WINDOWS_H)
	if(HAVE_WINDOWS_H)
		check_symbol_exists(QueryPerformanceCounter Windows.h HAVE_QUERYPERFORMANCECOUNTER)
	endif()
	check_include_file(mach/mach_time.h HAVE_MACH_MACH_TIME_H)
	check_include_file(sys/time.h HAVE_SYS_TIME_H)
	check_include_file(sys/times.h HAVE_SYS_TIMES_H)
	check_include_file(pthread.h HAVE_PTHREAD_H)
	check_symbol_exists(MONOTONIC_CLOCK time.h HAVE_MONOTONIC_CLOCK)

	check_cxx_source_compiles("int main() { auto x = \"\"; return 0; }" HAVE_CXX_AUTO)
	check_cxx_source_compiles("void f(int&&) {} int main() { return 0; }" HAVE_LVALUE_REF)
	check_cxx_source_compiles("int main() { static_assert(true, \"\"); return 0; }" HAVE_STATIC_ASSERT)
	check_cxx_source_compiles("int main() { char* p = nullptr; return 0; }" HAVE_NULLPTR)

	check_cxx_source_compiles(
		"template <int I> struct A {}; char xxx(int); char xxx(float); template <class T> A<sizeof(xxx((T)0))> f(T){} int main() { f(1); return 0; }"
		HAVE_EXPRESSION_SFINAE)
	check_cxx_source_compiles(
		"struct P { P() {}; }; union U { P p; int i; }; int main() { return 0;}"
		HAVE_UNRESTRICTED_UNIONS)
	check_cxx_source_compiles(
		"struct S { constexpr int f() { return 0; }}; int main() { return 0; }"
		HAVE_CONSTEXPR)

	test_big_endian(WORDS_BIGENDIAN)

	if(NOT "${ARGV0}" STREQUAL "")
		set(autoconfigure_config_h_source
"#ifndef INCLUDE_GUARD_\${autoconfigure_filename}_\${autoconfigure_random_value}
#define INCLUDE_GUARD_\${autoconfigure_filename}_\${autoconfigure_random_value}

/* pthread */
#cmakedefine HAVE_PTHREAD_H

/* STL */
#cmakedefine HAVE_CHRONO
#cmakedefine HAVE_MUTEX
#cmakedefine HAVE_THREAD

/* windows.h */
#cmakedefine HAVE_WINDOWS_H
#cmakedefine HAVE_QUERYPERFORMANCECOUNTER

/* mach */
#cmakedefine HAVE_MACH_MACH_TIME_H

/* time.h */
#cmakedefine HAVE_MONOTONIC_CLOCK

/* sys/ */
#cmakedefine HAVE_SYS_TIME_H
#cmakedefine HAVE_SYS_TIMES_H

/* C++ */
#cmakedefine HAVE_CXX_AUTO
#cmakedefine HAVE_LVALUE_REF
#cmakedefine HAVE_STATIC_ASSERT
#cmakedefine HAVE_NULLPTR
#cmakedefine HAVE_EXPRESSION_SFINAE
#cmakedefine HAVE_UNRESTRICTED_UNIONS
#cmakedefine HAVE_CONSTEXPR

#cmakedefine WORDS_BIGENDIAN

#endif
		")

		get_filename_component(autoconfigure_filename "${ARGV0}" NAME)
		string(MAKE_C_IDENTIFIER "${autoconfigure_filename}" autoconfigure_filename)
		string(RANDOM autoconfigure_random_value)
		message("autoconfigure_filename ${autoconfigure_filename}")
		string(CONFIGURE "${autoconfigure_config_h_source}"
			autoconfigure_output
			)

		file(WRITE "${ARGV0}" "${autoconfigure_output}")
	endif()

endmacro()
