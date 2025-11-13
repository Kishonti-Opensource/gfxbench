/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MESSAGEQUEUE_H_
#define MESSAGEQUEUE_H_

#include <stdint.h> // int32_t

namespace tfw
{

struct Message
{
	Message()
	: type(0)
	, arg1(0)
	, arg2(0)
	, flags(0)
	{}
	Message(int32_t t, int32_t a1, int32_t a2, int32_t f)
	: type(t)
	, arg1(a1)
	, arg2(a2)
	, flags(f)
	{}
	int32_t type;
	int32_t arg1;
	int32_t arg2;
	int32_t flags;
};

class MessageQueue
{
public:
    virtual ~MessageQueue() {}
	virtual Message pop_front() = 0;
	virtual bool has_next() = 0;
};

}

#endif  // MESSAGEQUEUE_H_
