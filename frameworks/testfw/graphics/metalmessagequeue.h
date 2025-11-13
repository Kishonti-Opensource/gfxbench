/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __METAL_MESSAGE_QUEUE_H__
#define __METAL_MESSAGE_QUEUE_H__


#include <deque>
#include "messagequeue.h"
#include "ng/mutex.h"
#include "ng/require.h"
#include <Cocoa/Cocoa.h>


#define MSG_TYPE_CLOSE 100
#define MSG_TYPE_KEY 1
#define MSG_TYPE_CURSOR 2
#define MSG_TYPE_MOUSE 3
#define MSG_TYPE_RESIZE 4
#define MSG_TYPE_BATTERY 0xBA7
#define MSG_TYPE_CHECKBOX 0x1110
#define MSG_TYPE_RANGE 0x1100


class MTLMessageQueue : public tfw::MessageQueue
{
public:
	MTLMessageQueue() {}
	
	virtual bool has_next();
	virtual tfw::Message pop_front();
	void push_back(const tfw::Message &msg);
	
	void processEvent(NSEvent* event);
	
	static tfw::Message makeKeyMessage(int key, int, int action, int mods);
	static tfw::Message makeCursorMessage(double x, double y);
	static tfw::Message makeMouseMessage(int button, int action, int mods);
	static tfw::Message makeResizeMessage(int width, int height);
	static tfw::Message makeCloseMessage();
	
protected:	
	struct ScopedLock
	{
		ScopedLock(ng::mutex &m) : m_(m) { m.lock(); }
		~ScopedLock() { m_.unlock(); }
		ng::mutex &m_;
	};

	int translateKeyCode(NSEvent* event);
	
	std::deque<tfw::Message> messages_;
	ng::mutex mutex_;
};



#endif  // __METAL_MESSAGE_QUEUE_H__

