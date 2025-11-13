/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/json_var.h"
#include "ng/var.h"

namespace ng {

var JSON_var::
	parse(substring ss)
{
	var v;
	parse(ss, OUT v);
	return v;
}

var JSON_var::
	parse(substring ss, OUT ng::Result& result)
{
	var v;
	parse(ss, OUT v, OUT result);
	return v;
}

void JSON_var::
	parse(substring ss, OUT var& v)
{
	parse(ss, OUT v, ng::throws());
}

}

