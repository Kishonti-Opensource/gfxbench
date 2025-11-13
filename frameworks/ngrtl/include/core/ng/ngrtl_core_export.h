/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/*
 * Disables "needs to have dll-interface to be used by clients of class" warning.
 *
 * See: http://stackoverflow.com/questions/5661738/how-can-i-use-standard-library-stl-classes-in-my-dll-interface-or-abi
 */
#ifdef _MSC_VER
#pragma warning(disable: 4251)
#endif

#if defined NGRTL_STATIC
  // This macro should be set when _using_ the SDK, and the SDK is a static lib
  #define NGRTL_EXPORT
  #define NGRTL_NO_EXPORT
#elif defined _WIN32 || defined __CYGWIN__
  // ngrtl_${module}_EXPORTS should be set when _building_ the SDK as shared lib
  #ifdef ngrtl_core_EXPORTS
    #define NGRTL_EXPORT __declspec(dllexport)
  #else
    #define NGRTL_EXPORT __declspec(dllimport)
  #endif
  // not needed, symbols are hidden by default
  #define NGRTL_NO_EXPORT
#elif __GNUC__ >= 4
  #if defined ngrtl_core_EXPORTS
    #define NGRTL_EXPORT __attribute__ ((visibility ("default")))
  #else
    #define NGRTL_EXPORT
  #endif
  #define NGRTL_NO_EXPORT __attribute__ ((visibility ("hidden")))
#else
  #define NGRTL_EXPORT
  #define NGRTL_NO_EXPORT
#endif
 
#if defined _WIN32 || __CYGWIN__
  #define NGRTL_DEPRECATED __declspec(deprecated)
#elif __GNUC__ >= 4
  #define NGRTL_DEPRECATED __attribute__ ((__deprecated__))
#else
  #define NGRTL_DEPRECATED
#endif
 
#ifndef NGRTL_DEPRECATED_EXPORT
#  define NGRTL_DEPRECATED_EXPORT NGRTL_EXPORT NGRTL_DEPRECATED
#endif
 
#ifndef NGRTL_DEPRECATED_NO_EXPORT
#  define NGRTL_DEPRECATED_NO_EXPORT NGRTL_NO_EXPORT NGRTL_DEPRECATED
#endif
 
#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define NGRTL_NO_DEPRECATED
#endif
