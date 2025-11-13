/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef HWNDMESSAGEQUEUE_H_
#define HWNDMESSAGEQUEUE_H_

#include <deque>
#include <windows.h>
#include "messagequeue.h"
#include "ng/mutex.h"
#include "ng/require.h"

#define MSG_TYPE_CLOSE 100
#define MSG_TYPE_KEY 1
#define MSG_TYPE_CURSOR 2
#define MSG_TYPE_MOUSE 3
#define MSG_TYPE_RESIZE 4
#define MSG_TYPE_BATTERY 0xBA7
#define MSG_TYPE_CHECKBOX 0x1110
#define MSG_TYPE_RANGE 0x1100

class HWNDMessageQueue : public tfw::MessageQueue
{
public:
    HWNDMessageQueue() {}

    virtual bool has_next();
    virtual tfw::Message pop_front();
    void push_back(const tfw::Message &msg);

    void processEvent(UINT message, WPARAM wParam, LPARAM lParam);

    static tfw::Message makeKeyMessage(int key, int, int action, int mods);
    static tfw::Message makeCursorMessage(int x, int y);
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

    std::deque<tfw::Message> messages_;
    ng::mutex mutex_;

    int translateKeyCode(UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
