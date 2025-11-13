/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "hwndmessagequeue.h"
#include <windowsx.h>


bool HWNDMessageQueue::has_next()
{
    ScopedLock guard(mutex_);
    bool b = !messages_.empty();
    return b;
}


tfw::Message HWNDMessageQueue::pop_front()
{
    ScopedLock guard(mutex_);
    tfw::Message msg = messages_.front();
    messages_.pop_front();
    return msg;
}


void HWNDMessageQueue::push_back(const tfw::Message &msg)
{
    ScopedLock guard(mutex_);
    messages_.push_back(msg);
}


tfw::Message HWNDMessageQueue::makeKeyMessage(int key, int, int action, int mods)
{
    tfw::Message msg;
    msg.type = MSG_TYPE_KEY;
    msg.arg1 = key;
    msg.arg2 = action;
    msg.flags = mods;
    return msg;
}


tfw::Message HWNDMessageQueue::makeCursorMessage(int x, int y)
{
    tfw::Message msg;
    msg.type = MSG_TYPE_CURSOR;
    msg.arg1 = x;
    msg.arg2 = y;
    return msg;
}


tfw::Message HWNDMessageQueue::makeMouseMessage(int button, int action, int mods)
{
    tfw::Message msg;
    msg.type = MSG_TYPE_MOUSE;
    msg.arg1 = button;
    msg.arg2 = action;
    msg.flags = mods;
    return msg;
}


tfw::Message HWNDMessageQueue::makeResizeMessage(int width, int height)
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


void HWNDMessageQueue::processEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_MOUSEMOVE:
        {
            int pos_x = GET_X_LPARAM(lParam);
            int pos_y = GET_Y_LPARAM(lParam);
            push_back(makeCursorMessage(pos_x, pos_y));
            break;
        }

        case WM_LBUTTONDOWN:
            push_back(makeMouseMessage(0, 1, 0));
            break;

        case WM_LBUTTONUP:
            push_back(makeMouseMessage(0, 1, 0));
            break;

        case WM_RBUTTONDOWN:
            push_back(makeMouseMessage(1, 1, 0));
            break;

        case WM_RBUTTONUP:
            push_back(makeMouseMessage(1, 0, 0));
            break;

        case WM_KEYDOWN:
        {
            int key = translateKeyCode(message, wParam, lParam);
            push_back(makeKeyMessage(key, 0, 1, 0));
            break;
        }

        case WM_KEYUP:
        {
            int key = translateKeyCode(message, wParam, lParam);
            push_back(makeKeyMessage(key, 0, 0, 0));
            break;
        }

		case WM_DISPLAYCHANGE:
		{
			push_back(makeResizeMessage(0, 0));
			break;
		}

        case WM_SYSKEYDOWN:
        case WM_SYSCHAR:
        case WM_SYSKEYUP:
        case WM_CHAR:
        default:
            break;
    }
}

int HWNDMessageQueue::translateKeyCode(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (wParam > 48 && wParam < 91)
    {
        return (int)wParam;
    }

    switch (wParam)
    {
    case VK_SPACE:
        return 32;
    case VK_RETURN:
        return 257;

    case VK_RSHIFT:
    case VK_LSHIFT:
    case VK_SHIFT:
        return 340;

    case VK_CONTROL:
        return 341;

    case VK_F1:
        return 290;
    case VK_F2:
        return 291;
    case VK_F3:
        return 292;
    case VK_F4:
        return 293;

    case VK_F5:
        return 294;
    case VK_F6:
        return 295;
    case VK_F7:
        return 296;
    case VK_F8:
        return 297;

    case VK_F9:
        return 298;
    case VK_F10:
        return 299;
    case VK_F11:
        return 300;
    case VK_F12:
        return 301;

    case VK_RIGHT:
        return 262;
    case VK_LEFT:
        return 263;
    case VK_DOWN:
        return 264;
    case VK_UP:
        return 265;

    case VK_TAB:
        return 258;

    default:
        return -1;
    }
}
