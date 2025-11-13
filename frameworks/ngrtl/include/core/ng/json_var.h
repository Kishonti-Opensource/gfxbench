/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef INCLUDE_GUARD_json_var_1382438067
#define INCLUDE_GUARD_json_var_1382438067

#include "ng/result.h"
#include "ng/substring.h"
#include "ng/macros.h"

namespace ng {

class var;

//JSON read/write methods for ng::var
//inspired by the javascript JSON object
class JSON_var
{
public:
	enum Format
	{
		FORMAT_PRETTY, //spaces, tabs and newlines
		FORMAT_COMPACT //no whitespaces
	};
	static var parse(substring ss);
	static var parse(substring ss, OUT ng::Result& result);
	static void parse(substring ss, OUT var& v);
	static void parse(substring ss, OUT var& v, OUT ng::Result& result);
	static std::string stringify(const var& v, Format f = FORMAT_COMPACT);
	static void stringify(const var& v, OUT std::string& s, Format m = FORMAT_COMPACT);
};

}

#endif

