/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testfw.h"
#include "ng/log.h"
#include "ng/timer.h"
#include "schemas/descriptors.h"
#include "schemas/result.h"
#include "deviceinfo/chartcollector.h"
#include "deviceinfo/platformruntimeinfo.h"
#include "messagequeue.h"
#include "graphics/graphicscontext.h"
#include "videostream.h"
#include <stdlib.h>
#include <fstream>

#include <cmath>
#if WIN32
#  ifdef HAVE_DX
#include "graphics/dxgraphicscontext.h"
#include <windows.h>
#  endif
#else
#include <unistd.h>
#endif

#include <epoxy/gl.h>
#include "graphics/videostreamrenderer.h"

#define DEBUG_SAVE_FRAMEBUFFER 0


#ifdef HAVE_VLD
#include "c:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#endif

#include "ng/macro_utils.h"
NG_TABLE_START(GL_ERROR_TABLE)
  NG_TABLE_ITEM0(GL_NO_ERROR)
  NG_TABLE_ITEM0(GL_INVALID_ENUM)
  NG_TABLE_ITEM0(GL_INVALID_VALUE)
  NG_TABLE_ITEM0(GL_INVALID_OPERATION)
  NG_TABLE_ITEM0(GL_STACK_OVERFLOW)
  NG_TABLE_ITEM0(GL_STACK_UNDERFLOW)
  NG_TABLE_ITEM0(GL_OUT_OF_MEMORY)
  NG_TABLE_ITEM0(GL_INVALID_FRAMEBUFFER_OPERATION)
NG_TABLE_END(GL_ERROR_TABLE)


#define CHECK_GL_ERROR { static int cnt = -1; ++cnt; GLenum err = glGetError(); if (err != GL_NO_ERROR) { NGLOG_ERROR("[%s] %s:%s glGetError() = %s\n", cnt, __FILE__, __LINE__, GL_ERROR_TABLE(err).c_str()); } }


namespace tfw
{

void sleep(double sec)
{
#ifdef WIN32
    Sleep(static_cast<DWORD>(sec*1e3));
#else
    usleep(sec*1e6);
#endif
}

class Basic : public TestBase
{
public:
    Basic()
    : initSteps_(30)
    , initStepTime_(0.1)
    , steps_(30)
    , stepTime_(0.1)
    , progress_(0)
    , width_(256)
    , height_(256)
    {
        runtimeInfo_ = &platformRuntimeInfo_;
    }
    ~Basic()
    {
        NGLOG_INFO("Basic test destructing");
    }
    bool init()
    {
        Descriptor d;
        std::string err;
        bool ok = Descriptor::fromJsonString(config(), &d, &err);
        if (ok)
        {
            initSteps_ = static_cast<int32_t>(d.rawConfign("init_steps", initSteps_));
            initStepTime_ = d.rawConfign("init_step_time", initStepTime_);
            steps_ = static_cast<int32_t>(d.rawConfign("steps", steps_));
            stepTime_ = d.rawConfign("step_time", stepTime_);
        }
        if (ctx_)
        {
            width_ = d.env().width();
            height_ = d.env().height();
            assert(ctx_->makeCurrent());
            if (ctx_->type() == GraphicsContext::GLES || ctx_->type() == GraphicsContext::OPENGL)
            {
                NGLOG_INFO("GL_VERSION: %s", (const char*)glGetString(GL_VERSION));
                NGLOG_INFO("GL_RENDERER: %s", (const char*)glGetString(GL_RENDERER));
                NGLOG_INFO("GL_VENDOR: %s", (const char*)glGetString(GL_VENDOR));
                NGLOG_INFO("GL_EXTENSIONS: %s", (const char*)glGetString(GL_EXTENSIONS));
                videoStreamRenderer_.setupGL();
            }
        }
        if (ok)
        {
            walk(initSteps_, initStepTime_);
        }
        return ok;
    }
    void run()
    {
        NGLOG_INFO("Running test: %s", name());
        if (ctx_) ctx_->makeCurrent();
        chartCollector_.start(*runtimeInfo_, 500);
        walk(steps_, stepTime_);
        resultGroup_ = chartCollector_.stop();
    }
    virtual float progress()
    {
        return progress_;
    }
    virtual std::string result()
    {
        Result result;
        result.setTestId(name());
        result.setResultId(name());
        result.setStatus(isCancelled() ? Result::CANCELLED : Result::OK);
        result.setScore(1.0);
        result.setUnit("ok");
        resultGroup_.results().push_back(result);
        return resultGroup_.toJsonString();
    }

    virtual bool terminate()
    {
        if (ctx_ != 0 && (ctx_->type() == GraphicsContext::GLES || ctx_->type() == GraphicsContext::OPENGL))
        {
            ctx_->makeCurrent();
            videoStreamRenderer_.releaseGL();
        }
        return true;
    }
private:
    void walk(int32_t s, double t)
    {
        int32_t i;
        for(i = 0; i < s && !isCancelled(); ++i)
        {
            while (msgQueue_ && msgQueue_->has_next())
            {
                Message msg = msgQueue_->pop_front();
                NGLOG_INFO("type: %s, arg1: %s, arg2: %s, flags: %s", msg.type, msg.arg1, msg.arg2, msg.flags);
            }
            sleep(t);
            progress_ = i/(float)s;
            draw();
        }
        progress_ = i/(float)s;
    }

    void draw()
    {
        if (ctx_)
        {
            if (ctx_->type() == GraphicsContext::DIRECTX)
            {
                drawDX();
            }
            else
            {
                drawGL();
            }
            ctx_->swapBuffers();
#if DEBUG_SAVE_FRAMEBUFFER
            char buf[256];
            static int i = 0;
            sprintf(buf, "/sdcard/kishonti/tfw/frame_%dx%d_%04d.rgba", width_, height_, i++ % 10);
            std::ofstream os(buf);
            std::vector<char> rgba;
            rgba.resize(width_ * height_ * 4);
            glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
            os.write(rgba.data(), rgba.size());
            os.close();
#endif
        }
    }

    void drawGL()
    {
        int32_t streamCount = videoStreamCount();
        if (streamCount > 0)
        {
            int32_t x = 0;
            int32_t dx = (int32_t)((width_/(float)streamCount) + 0.5f);
            for (int32_t i = 0; i < streamCount; ++i)
            {
                VideoStream *stream = videoStream(i);
                NGLOG_INFO("using: %s", stream->name());
                glViewport(x, 0, dx, height_);
                videoStreamRenderer_.drawGL(stream);
                x+=dx;
            }
        }
        else
        {
            float r = (rand()%32768)/32768.f;
            float g = (rand()%32768)/32768.f;
            float b = (rand()%32768)/32768.f;

            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    void drawDX()
    {
#ifdef HAVE_DX
        float r = (rand()%32768)/32768.f;
        float g = (rand()%32768)/32768.f;
        float b = (rand()%32768)/32768.f;

        DXGraphicsContext *dx = dynamic_cast<DXGraphicsContext*>(ctx_);
        ID3D11RenderTargetView *rtv = dx->getD3D11RenderTargetView();
        ID3D11DeviceContext *dxctx = dx->getD3D11DeviceContext();
        ID3D11Device *dxdev = dx->getD3D11Device();
        float color[4] = { r, g, b, 1.0f };
        dx->getD3D11DeviceContext()->ClearRenderTargetView(rtv, color);
#endif
    }

    int32_t initSteps_;
    double initStepTime_;
    int32_t steps_;
    double stepTime_;
    float progress_;
    ResultGroup resultGroup_;
    PlatformRuntimeInfo platformRuntimeInfo_;
    ChartCollector chartCollector_;
    int32_t width_;
    int32_t height_;
    VideoStreamRenderer videoStreamRenderer_;
};


}


template<class T>
class Communitizer : public T
{
public:
    virtual bool init()
    {
        NGLOG_INFO("pre-init - doing some community only stuff");
        return T::init();
    }
    virtual void run()
    {
        T::run();
        NGLOG_INFO("post-run: doing some community only stuff");
    }
};

#ifndef COMMUNITY
CREATE_FACTORY(tfw_basic, tfw::Basic)
#else
CREATE_FACTORY(tfw_basic, Communitizer<basic::Basic>)
#endif
