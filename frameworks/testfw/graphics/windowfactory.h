/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef WINDOWFACTORY_H_
#define WINDOWFACTORY_H_

class GraphicsWindow;
class GraphicsContext;
namespace tfw {
    class MessageQueue;
    class ApiDefinition;
}

#include "glformat.h"
#include "ng/scoped_ptr.h"
#include <vector>

class WindowFactory
{
public:
    class Wnd
    {
    public:
        Wnd() {}
        // all member return weak pointers
        GraphicsWindow *wnd() { return wnd_.get(); }
        GraphicsContext *graphics() { return graphics_.get(); }
        tfw::MessageQueue *queue() { return queue_.get(); }

    private:
        friend class WindowFactory;
        Wnd(GraphicsWindow *w, tfw::MessageQueue *m, GraphicsContext *g)
        : wnd_(w)
        , queue_(m)
        , graphics_(g)
        {}
        ng::scoped_ptr<GraphicsWindow> wnd_;
        ng::scoped_ptr<tfw::MessageQueue> queue_;
        ng::scoped_ptr<GraphicsContext> graphics_;
    };

    WindowFactory();
    Wnd *create(const std::string &gfx);
    Wnd *createGLFW(const std::vector<tfw::ApiDefinition>& api, const std::string &target_api);
    Wnd *createEGLPB(const std::vector<tfw::ApiDefinition>& api_defs);
    Wnd *createEGLFBDEV(const std::vector<tfw::ApiDefinition>& api_defs);
    Wnd *createEGLCDS(const std::vector<tfw::ApiDefinition>& api_defs);
    Wnd *createEGL(const std::vector<tfw::ApiDefinition>& api_defs);
    WindowFactory::Wnd *createQNXScreenVulkan(const std::vector<tfw::ApiDefinition>& api_defs);
    WindowFactory::Wnd *createWaylandVulkan(const std::vector<tfw::ApiDefinition>& api_defs);
    WindowFactory::Wnd *createWaylandXCB(const std::vector<tfw::ApiDefinition>& api_defs);
    Wnd *createDX();
    Wnd *createWGL();
    Wnd *createHWND(const std::vector<tfw::ApiDefinition>& api_defs, const std::string &gfx);
    Wnd *createNULL();
    Wnd *createMTL(const std::vector<tfw::ApiDefinition>& api);
    void setSize(int width, int height);
    void setFullscreen(bool fullscreen);
    void setApi(const std::vector<tfw::ApiDefinition>& api, const std::string &target_api);
    void setFormat(const tfw::GLFormat& format);
	void setVsync( bool vsync );
#if defined(__QNX__)
    void setPixelFormat(const std::string& format);
    void setNumBuffers(int nbuffers);
    void setInterval(int interval);
    void setDesiredDisplay(const std::string &desiredDisplay);
    tfw::GLFormat getFormat();
#endif
private:
    std::string target_api_;
    std::vector<tfw::ApiDefinition> api_defs_;
    tfw::GLFormat format_;

    int width_;
    int height_;
    bool fullscreen_;
	bool vsync_;
#if defined(__QNX__)
    std::string pixelFormat_;
    std::string desiredDisplay_;
    int nbuffers_;
    int interval_;
#endif
};

#endif  // WINDOWFACTORY_H_
