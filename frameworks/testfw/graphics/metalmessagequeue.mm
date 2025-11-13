/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "metalmessagequeue.h"

	
bool MTLMessageQueue::has_next()
{
	ScopedLock guard(mutex_);
	bool b = !messages_.empty();
	return b;
}


tfw::Message MTLMessageQueue::pop_front()
{
	ScopedLock guard(mutex_);
	tfw::Message msg = messages_.front();
	messages_.pop_front();
	return msg;
}


void MTLMessageQueue::push_back(const tfw::Message &msg)
{
	ScopedLock guard(mutex_);
	messages_.push_back(msg);
}


tfw::Message MTLMessageQueue::makeKeyMessage(int key, int, int action, int mods)
{
	tfw::Message msg;
	msg.type = MSG_TYPE_KEY;
	msg.arg1 = key;
	msg.arg2 = action;
	msg.flags = mods;
	return msg;
}


tfw::Message MTLMessageQueue::makeCursorMessage(double x, double y)
{
	tfw::Message msg;
	msg.type = MSG_TYPE_CURSOR;
	msg.arg1 = static_cast<int>(x);
	msg.arg2 = static_cast<int>(y);
	return msg;
}


tfw::Message MTLMessageQueue::makeMouseMessage(int button, int action, int mods)
{
	tfw::Message msg;
	msg.type = MSG_TYPE_MOUSE;
	msg.arg1 = button;
	msg.arg2 = action;
	msg.flags = mods;
	
	return msg;
}


tfw::Message MTLMessageQueue::makeResizeMessage(int width, int height)
{
	tfw::Message msg;
	msg.type = MSG_TYPE_RESIZE;
	msg.arg1 = width;
	msg.arg2 = height;
	
	return msg;
}


tfw::Message makeCloseMessage()
{
	tfw::Message msg;
	msg.type = MSG_TYPE_CLOSE;
	return msg;
}


void MTLMessageQueue::processEvent(NSEvent* event)
{
	switch (event.type) {
			
		case NSFlagsChanged:
		{
			break;
		}
			
		case NSKeyDown:
		{
			int key = translateKeyCode(event);
			push_back(makeKeyMessage(key, 0, 1, 0));
			break;
		}
			
		case NSKeyUp:
		{
			int key = translateKeyCode(event);
			push_back(makeKeyMessage(key, 0, 0, 0));
			break;
		}
			
		case NSLeftMouseDown:
		{
			push_back(makeMouseMessage(0, 1, 0));
			break;
		}
			
		case NSLeftMouseUp:
		{
			push_back(makeMouseMessage(0, 0, 0));
			break;
		}
			
			
		case NSRightMouseDown:
		{
			push_back(makeMouseMessage(1, 1, 0));
			break;
		}
			
		case NSRightMouseUp:
		{
			push_back(makeMouseMessage(1, 0, 0));
			break;
		}
			
		default:
			break;
	}
}


int MTLMessageQueue::translateKeyCode(NSEvent* event)
{
	
	switch (event.keyCode) {
		case 0:  return 65; // a
		case 2:  return 68; // d
		case 14: return 69; // e
		case 12: return 81; // q
		case 15: return 82; // r
		case 1:  return 83; // s
		case 13: return 87; // w
			
		case 48: return 258; // tab
			
		default:
			break;
	}
	return -1;
}

