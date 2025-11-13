/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLFWMESSAGEQUEUE_H_
#define GLFWMESSAGEQUEUE_H_

#include <GLFW/glfw3.h>
#include <deque>
#include "messagequeue.h"
#include "ng/mutex.h"
#include "ng/require.h"

namespace tfw {

#define MSG_TYPE_CLOSE 100
#define MSG_TYPE_KEY 1
#define MSG_TYPE_CURSOR 2
#define MSG_TYPE_MOUSE 3
#define MSG_TYPE_RESIZE 4
#define MSG_TYPE_BATTERY 0xBA7
#define MSG_TYPE_CHECKBOX 0x1110
#define MSG_TYPE_RANGE 0x1100

class GLFWMessageQueue : public MessageQueue
{
public:
	GLFWMessageQueue(bool threading)
	: threading_(threading)
	{}

	bool attach(GLFWwindow *w)
	{
		bool ok = false;
		if (glfwGetWindowUserPointer(w) == NULL)
		{
			glfwSetWindowUserPointer(w, this);
			glfwSetKeyCallback(w, key_cb);
			glfwSetCursorPosCallback(w, cursor_cb);
			glfwSetMouseButtonCallback(w, mouse_cb);
			glfwSetWindowCloseCallback(w, close_cb);
			glfwSetWindowSizeCallback(w, resize_cb);
			ok = true;
		}
		return ok;
	}

	virtual bool has_next()
	{
		if (!threading_)
		{
			// glfwPollEvents may only be called from the main thread.
			// see: http://www.glfw.org/docs/3.0/group__window.html#ga37bd57223967b4211d60ca1a0bf3c832
			glfwPollEvents();
		}
		ScopedLock guard(mutex_);
		bool b = !messages_.empty();
		return b;
	}

	virtual Message pop_front()
	{
		ScopedLock guard(mutex_);
		Message msg = messages_.front();
		messages_.pop_front();
		return msg;
	}

	void push_back(const Message &msg)
	{
		ScopedLock guard(mutex_);
		messages_.push_back(msg);
	}

private:
	struct ScopedLock
	{
		ScopedLock(ng::mutex &m) : m_(m) { m.lock(); }
		~ScopedLock() { m_.unlock(); }
		ng::mutex &m_;
	};

	static Message makeKeyMessage(int key, int, int action, int mods)
	{
		Message msg;
		msg.type = MSG_TYPE_KEY;
		msg.arg1 = key;
		msg.arg2 = action;
		msg.flags = mods;
		return msg;
	}

	static Message makeCursorMessage(double x, double y)
	{
		Message msg;
		msg.type = MSG_TYPE_CURSOR;
		msg.arg1 = static_cast<int>(x);
		msg.arg2 = static_cast<int>(y);
		return msg;
	}

	static tfw::Message makeMouseMessage(int button, int action, int mods)
	{
		tfw::Message msg;
		msg.type = MSG_TYPE_MOUSE;
		msg.arg1 = button;
		msg.arg2 = action;
		msg.flags = mods;

		return msg;
	}

	static tfw::Message makeResizeMessage(int width, int height)
	{
		tfw::Message msg;
		msg.type = MSG_TYPE_RESIZE;
		msg.arg1 = width;
		msg.arg2 = height;

		return msg;
	}

	static tfw::Message makeCloseMessage()
	{
		tfw::Message msg;
		msg.type = MSG_TYPE_CLOSE;
		return msg;
	}

	static void key_cb(GLFWwindow *w, int key, int scancode, int action, int mods)
	{
		GLFWMessageQueue *thiz = (GLFWMessageQueue*) glfwGetWindowUserPointer(w);
		require(thiz != NULL);
		thiz->push_back(makeKeyMessage(key, scancode, action, mods));
	}

	static void cursor_cb(GLFWwindow *w, double x, double y)
	{
		GLFWMessageQueue *thiz = (GLFWMessageQueue*) glfwGetWindowUserPointer(w);
		require(thiz != NULL);
		thiz->push_back(makeCursorMessage(x, y));
	}

	static void mouse_cb(GLFWwindow *w, int button, int action, int mods)
	{
		GLFWMessageQueue *thiz = (GLFWMessageQueue*)glfwGetWindowUserPointer(w);
		require(thiz != NULL);
		thiz->push_back(makeMouseMessage(button, action, mods));
	}

	static void resize_cb(GLFWwindow *w, int width, int height)
	{
		GLFWMessageQueue *thiz = (GLFWMessageQueue*)glfwGetWindowUserPointer(w);
		require(thiz != NULL);
		thiz->push_back(makeResizeMessage(width, height));
	}

	static void close_cb(GLFWwindow *w)
	{
		GLFWMessageQueue *thiz = (GLFWMessageQueue*)glfwGetWindowUserPointer(w);
		require(thiz != NULL);
		thiz->push_back(makeCloseMessage());
	}

	std::deque<Message> messages_;
	ng::mutex mutex_;
	bool threading_;
};

}

#endif
