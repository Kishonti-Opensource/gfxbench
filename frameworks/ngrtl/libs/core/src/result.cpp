/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/result.h"

#include <string.h>

#ifdef UNDER_CE
#include "string.h"
#endif

#include "ng/format.h"

namespace ng {

	Result Result::
		s_throwingResult;

	std::string ErrorCode::
		message() const
	{
		return FORMATSTR("%s (errno = %s)", strerror(_ec), _ec);
	}

	const char* SystemError::
		what() const throw()
	{
		if(_whatString.empty())
		{
			_whatString = _ec.message();
			if(!_whatArg.empty())
			{
				_whatString += ", ";
				_whatString += _whatArg;
			}
		}
		return _whatString.c_str();
	}

	void Result::
		clear()
	{
        if (throws()) return;
		_bError = false;
		_message.clear();
	}
		
	void Result::
		setMessage(cstring s)
	{
        if (throws()) return;
        _bError = true;
        _message.assign(s.c_str(), s.size());
	}
		
	void Result::
		setErrorCode(const ng::ErrorCode& ec)
	{
        if (throws()) return;
        _bError = true;
        _errorCode = ec;
        _message = ec.message();
	}

	void Result::
		setMsgErrorCode(cstring msg, const ng::ErrorCode& ec)
	{
        if (throws()) return;
        _bError = true;
        _errorCode = ec;
        _message = FORMATSTR("%s, %s", msg, ec.message());
	}
}
