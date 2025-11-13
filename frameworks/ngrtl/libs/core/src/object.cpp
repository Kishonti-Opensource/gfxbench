/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/object.h"

#include <typeinfo>

#include "ng/format.h"

namespace ng {

std::string Object::
	toString() const
{
	return FORMATSTR("%s@0x%s",typeid(*this).name(), this);
}

}
