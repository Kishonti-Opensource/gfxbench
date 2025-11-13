/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_MESSAGEQUEUE_H_
#define TFW_MESSAGEQUEUE_H_

#include <deque>
#include "messagequeue.h"
#include "ng/mutex.h"
#include "ng/log.h"


namespace tfw {

class TfwMessageQueue : public MessageQueue
{
public:
	virtual bool has_next()
	{
		ng::unique_lock<ng::mutex> mutex_;
		bool b = !messages_.empty();
		return b;
	}

	virtual Message pop_front()
	{
		ng::unique_lock<ng::mutex> mutex_;
		Message msg = messages_.front();
		messages_.pop_front();
		return msg;
	}

	void push_back(const Message &msg)
	{
		ng::unique_lock<ng::mutex> mutex_;
		messages_.push_back(msg);
	}

private:
	std::deque<Message> messages_;
	ng::mutex mutex_;
};

}

#endif
