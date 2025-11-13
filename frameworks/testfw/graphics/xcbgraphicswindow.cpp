/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/xcbgraphicswindow.h"
#include "ng/log.h"


XCBGraphicsWindow::XCBGraphicsWindow()
    : connection_(0)
    , screen_(0)
    , window_(0)
    , width_(0)
    , height_(0)
    , closeRequested_(false)
{
}


XCBGraphicsWindow::~XCBGraphicsWindow()
{
    destroy();
}

xcb_intern_atom_reply_t *atom_wm_delete_window;
bool XCBGraphicsWindow::create(int width, int height, const std::string &title)
{
    if (width <= 0 || height <= 0)
        {
            NGLOG_ERROR("XCBGraphicsWindow: width, and height must be greater than zero (%s, %s)", width, height);
            return false;
        }

    uint32_t value_mask, value_list[32];
    int scr;
    connection_ = xcb_connect(NULL, &scr);
    if (connection_ == NULL)
        {
            printf("Cannot find a compatible Vulkan installable client driver "
                   "(ICD).\nExiting ...\n");
            fflush(stdout);
            exit(1);
        }

    window_ = xcb_generate_id(connection_);

    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;

    setup = xcb_get_setup(connection_);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);

    screen_ = iter.data;


    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = screen_->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_EXPOSURE |
                    XCB_EVENT_MASK_KEY_PRESS |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    xcb_create_window(connection_, XCB_COPY_FROM_PARENT, window_,
                      screen_->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen_->root_visual,
                      value_mask, value_list);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(connection_, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply =
        xcb_intern_atom_reply(connection_, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 =
        xcb_intern_atom(connection_, 0, 16, "WM_DELETE_WINDOW");
    atom_wm_delete_window =
        xcb_intern_atom_reply(connection_, cookie2, 0);

    xcb_change_property(connection_, XCB_PROP_MODE_REPLACE, window_,
                        (*reply).atom, 4, 32, 1,
                        &(*atom_wm_delete_window).atom);
    free(reply);

    xcb_map_window(connection_, window_);
    xcb_flush(connection_);

    width_ = width;
    height_ = height;
    return true;
}



void XCBGraphicsWindow::destroy()
{
    xcb_disconnect (connection_);
    /*
        if (window_ != 0) {
            XDestroyWindow(display_, window_);
            XCloseDisplay(display_);
        }
    */
}


void XCBGraphicsWindow::schedule_xcb_repaint()
{
    xcb_client_message_event_t client_message;

    client_message.response_type = XCB_CLIENT_MESSAGE;
    client_message.format = 32;
    client_message.window = window_;
    client_message.type = XCB_ATOM_NOTICE;

    xcb_send_event(connection_, 0, window_, 0, (char *) &client_message);
}


void XCBGraphicsWindow::pollEvents()
{
    xcb_generic_event_t *event;
    xcb_key_press_event_t *key_press;
    xcb_client_message_event_t *client_message;

    while ( (event = xcb_poll_for_event(connection_)) != nullptr)
        {
            switch (event->response_type & 0x7f)
                {
                case XCB_CLIENT_MESSAGE:
                    client_message = (xcb_client_message_event_t *) event;
                    if (client_message->window != window_)
                        {
                            break;
                        }
                    /*
                          if (client_message->type == vc->xcb.atom_wm_protocols &&
                              client_message->data.data32[0] == vc->xcb.atom_wm_delete_window)
                              {
                    			exit(0);
                    		}
                    */
                    if (client_message->type == XCB_ATOM_NOTICE)
                        {
                            schedule_xcb_repaint();
                        }
                    break;

                case XCB_CONFIGURE_NOTIFY:

                    xcb_configure_notify_event_t *configure_event;
                    configure_event = (xcb_configure_notify_event_t*)event;
                    printf("resized %dx%d\n", configure_event->width, configure_event->height);

                    break;

                case XCB_EXPOSE:
                    schedule_xcb_repaint();
                    break;

                case XCB_KEY_PRESS:
                    key_press = (xcb_key_press_event_t *) event;

                    if (key_press->detail == 9)                        // escape
                        {
                            exit(0);
                        }
                    break;
                }

            free(event);
            xcb_flush(connection_);
        }
    /*
        XEvent event;
        while (XPending(display_)) {
            XNextEvent(display_, &event);
            NGLOG_INFO("in loop");
            switch (event.type)
            {
            case ClientMessage:
                closeRequested_ = true;
                break;
            }
        }
    */
}

bool XCBGraphicsWindow::shouldClose()
{
    return closeRequested_;
}

int XCBGraphicsWindow::width()
{
    return width_;
}

int XCBGraphicsWindow::height()
{
    return height_;
}

void XCBGraphicsWindow::requestClose()
{
    closeRequested_ = true;
}

