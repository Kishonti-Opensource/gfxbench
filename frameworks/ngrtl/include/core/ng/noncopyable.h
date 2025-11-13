/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_NONCOPYABLE_H
#define NG_NONCOPYABLE_H

//Usage: class SomeClass : private noncopyable { ... };

#include "ng/ngrtl_core_export.h"

namespace ng
{

class NGRTL_EXPORT noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:
	noncopyable(const noncopyable&);
	noncopyable& operator = (const noncopyable&);
};

}

#endif //NON_COPYABLE_H
