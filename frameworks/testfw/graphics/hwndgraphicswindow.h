/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef HWNDGRAPHICSWINDOW_H_
#define HWNDGRAPHICSWINDOW_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#include <string>

#include "graphics/graphicswindow.h"
#include "graphics/hwndmessagequeue.h"

class HWNDGraphicsWindow : public GraphicsWindow
{
public:
    HWNDGraphicsWindow();
    virtual ~HWNDGraphicsWindow();
    void create(int width, int height, const std::string &title, bool fullscreen);
    void destroy();
    HWND handle();
    virtual void pollEvents();
    virtual bool shouldClose();
    virtual void requestClose();
    virtual int width();
    virtual int height();
    virtual void setMessageQueue(HWNDMessageQueue *queue);
    virtual tfw::MessageQueue* getMessageQueue();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void translateAndEnqueMessage(UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE hInstance_;
    HWND hWnd_;
    HDC hDC_;
    bool closeRequested_;
    std::string className_;
    bool fullscreen_;
    int width_;
    int height_;
    HWNDMessageQueue *msg_queue_;
};


#endif  // HWNDGRAPHICSWINDOW_H_
