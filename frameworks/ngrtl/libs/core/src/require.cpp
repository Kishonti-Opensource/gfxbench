/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/require.h"

#include <stdio.h>
#include <string>

#ifdef PLATFORM_ANDROID
	#include <android/log.h>
	#include <signal.h>
#endif

namespace ng
{

#ifdef PLATFORM_ANDROID
bool ngassert_android(const char* e, const char* fun, const char* file, int line)
{
	__android_log_print(ANDROID_LOG_FATAL, "assert",
		"ngassert failed %s in: %s (%s:%d)", e, fun, file, line);
	kill(0, SIGINT);
	return false;
}
#endif

void require_exception_core(const char* expression, const char* file, int line, const char* function, const char* message)
{
	char buf[2048]; //must be enough
	sprintf(buf, "cond: (%s) in %s, file: %s:#%d", expression, function, file, line);
	throw require_failed(message, buf);
}

namespace {
	std::string create_require_what(const char* msg, const char* det)
	{
		bool bm = msg != 0 && *msg != 0;
		bool bd = det != 0 && *det != 0;
		if ( bm || bd )
		{
			char buf[2048];
			if ( !bd )
				sprintf(buf, "Required condition not met. Msg: \"%s\"", msg);
			else if ( !bm )
				sprintf(buf, "Required condition not met, %s", det);
			else
				sprintf(buf, "Required condition not met. Msg: \"%s\", %s", msg, det);
			return buf;
		} else
			return "Required condition not met.";
	}
}

require_failed::
	require_failed(const char* msg, const char* det)
	: _what(create_require_what(msg, det))
	, _message(msg ? msg : "")
	, _details(det ? det : "")
{
}


}


