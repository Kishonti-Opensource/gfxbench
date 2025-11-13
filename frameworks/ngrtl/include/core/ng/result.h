/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_RESULT_INCLUDED
#define NG_RESULT_INCLUDED

#include <string>
#include "ng/ngrtl_core_export.h"
#include "ng/macros.h"
#include "ng/require.h"
#include "ng/cstring.h"

/* -------- Result class --------

  Supports optional error report or exception throwing in functions

  Example:

  Define your function accepting a reference to a Result object.
  At the point of the error use the NG_SET_RESULT_MSG macro

	  void myopenfile(const char* filename, ng::Result& res)
	  {
          res.clear();
		  FILE* f = fopen(filename, "rb");
		  if ( f == NULL )
		  {
			NG_SET_RESULT_MSG(res, "Can't open file");
			return;
		  }
		  //...
	  }

  You can call the function in two ways. When you're interested
  in the result:

	  ng::Result res;
	  myopenfile("1.txt", res);
	  if ( res.failed() )
		printf("Error: %s", res.what());

  When you're not:

	  myopenfile("1.txt", ng::throws());

  Having to specify ng::throws() warns you that the function may throw exceptions.

  ng::throws() is a reference to a special ng::Result object whose address is nullptr.
  So it's the reference version of a nullptr.

  Please note that Result have some nonstandard behaviour to save typing.
  ng::throws(), that is (ng::Result&)0 is considered a valid cleared (= nonerror) Result object.
  That's why the following operations are valid on ng::throws():

  clear(), what(), error(), ok(), throws()
 

*/


namespace ng
{
	class Result;

	//similar to std::error_code
	class NGRTL_EXPORT ErrorCode
	{
	public:
		ErrorCode()	: _ec(0) {}
		ErrorCode(int ec) : _ec(ec) {} //for now it's generic error category always
		int value() const { return _ec; }
		std::string message() const;
	private:
		int _ec;
	};

    namespace result_detail {
        inline Result* throws();
    }

    inline Result& throws() { return *result_detail::throws(); }

    class NGRTL_EXPORT Result
	{
	public:
		Result()
			: _bError(false)
		{}
		
		void clear();
		
		void setMessage(cstring s);
		void setErrorCode(const ng::ErrorCode& ec);
		void setMsgErrorCode(cstring msg, const ng::ErrorCode& ec);

		const char* what() const
		{ return _message.c_str(); }
		
		bool error() const
		{ return _bError; }
		
		bool ok() const
		{ return !_bError; }

		bool throws() const
		{ return this == &::ng::throws(); }

		const ErrorCode& code() const
		{ return _errorCode; }
	private:
		bool _bError;
		std::string _message;
		ng::ErrorCode _errorCode;

        static Result s_throwingResult;

        friend Result* result_detail::throws();
	};

    namespace result_detail {
        inline Result* throws() { return &Result::s_throwingResult; }
    }


    //similar to std::system_error
	class NGRTL_EXPORT SystemError
		: public std::exception
	{
	public:
		SystemError(const ErrorCode& ec) : _ec(ec) {}
		SystemError(const ErrorCode& ec, cstring msg) : _ec(ec), _whatArg(msg.c_str(), msg.size()) {}
		virtual ~SystemError() throw() {}
		const ErrorCode& code() const { return _ec; }
		const char* what() const throw();
	private:
		ErrorCode _ec;
		std::string _whatArg;
		mutable std::string _whatString;
	};
}

#define NG_SET_RESULT_MSG(LHS,S) ((&(LHS) == &ng::throws()) ? throw std::runtime_error(S) : (LHS).setMessage(S))
#define NG_SET_RESULT_ERRNO(LHS,S) ((&(LHS) == &ng::throws()) ? throw ::ng::SystemError(ng::ErrorCode(S)) : (LHS).setErrorCode(ng::ErrorCode(S)))
#define NG_SET_RESULT_MSG_ERRNO(LHS,MSG,EN) ((&(LHS) == &ng::throws()) ? throw ::ng::SystemError(ng::ErrorCode(EN),(MSG)) : (LHS).setMsgErrorCode((MSG),ng::ErrorCode(EN)))

#endif

