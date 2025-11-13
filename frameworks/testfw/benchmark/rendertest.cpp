/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "rendertest.h"
#include "graphics/graphicscontext.h"
#include "ng/log.h"

using namespace tfw;

RenderTest::RenderTest(bool offscreen)
: duration_(30)
, width_(512)
, height_(512)
, offscreen_(offscreen)
, frames_(0)
{}


bool RenderTest::init()
{
    const std::string &c = config();
    bool ok = descriptor_.fromJsonString(c);
    if (!ok) return false;

    width_ = descriptor_.env().width();
    height_ = descriptor_.env().height();
    duration_ = descriptor_.rawConfign("duration", duration_);

    const Config &b = descriptor_.env().graphics().config();
    int32_t colorBpp = b.red() + b.green() + b.blue();

    offscreenmgr_.reset(OffscreenManager::Create());

    OffscreenManager::ManagerMode mode = offscreen_ ? OffscreenManager::OffscreenMode : OffscreenManager::OnscreenMode;
    int err = offscreenmgr_->Init(mode, width_, height_, 0, colorBpp, b.depth(), 0);
    ok = (0 == err);
    return ok;
}

bool RenderTest::terminate()
{
	bool ok =  result_.results().size() > 0;
	if(ok)
	{
		tfw::Result &res = result_.results()[0];
		NGLOG_INFO("device: %s", res.gfxResult().renderer());
		NGLOG_INFO("result: %s %s", res.score(), res.unit());
	}
    return ok;
}

std::string RenderTest::result()
{
    std::string result = result_.toJsonString();
    return result;
}

void RenderTest::preRender()
{
    offscreenmgr_->PreRender();
}

void RenderTest::postRender()
{
    bool swapNeeded = offscreenmgr_->PostRender(0, 0, width_, height_);
    if (swapNeeded)
        graphicsContext()->swapBuffers();
    ++frames_;
}

void RenderTest::flushRender()
{
    bool swapNeeded = offscreenmgr_->RenderLastFrames(width_, height_);
    if (swapNeeded)
        graphicsContext()->swapBuffers();
    offscreenmgr_->FinishRendering();
}


void RenderTest::cleanup()
{
    offscreenmgr_.release();
}

void RenderTest::setDuration(double duration)
{
    duration_ = duration;
}

bool RenderTest::durationExpired()
{
    return timer_.elapsed().wall >= duration_;
}

void RenderTest::startMeasurement()
{
    timer_.start();
}

void RenderTest::stopMeasurement()
{
    timer_.stop();
}

void RenderTest::updateResult()
{
    ResultGroup rg;
    Result r;
    r.setTestId(name());
    r.setResultId(name());
    if (isCancelled())
    {
        r.setStatus(Result::CANCELLED);
    }
    else
    {
        r.setStatus(Result::OK);
    }
    double dt = timer_.elapsed().wall;
    double score = duration_ / dt * frames_;
	
	 GfxResult &gfx = r.gfxResult();
	
#ifndef NO_RESULT
    r.setScore(score);
    r.setUnit("frames");
    r.setMeasuredTime(duration_*1000);
    r.setElapsedTime(dt*1000);
    gfx.setFps(score/duration_);
    gfx.setSurfaceWidth(renderWidth());
    gfx.setSurfaceHeight(renderHeight());
    gfx.setFrameCount(frames_);
#endif
	
    rg.addResult(r);
    result_ = rg;
}

int32_t RenderTest::renderWidth() const
{
    if (!offscreen_) return width_;
    return offscreenmgr_->width();
}

int32_t RenderTest::renderHeight() const
{
    if (!offscreen_) return height_;
    return offscreenmgr_->height();
}

uint32_t RenderTest::defaultFbo() const
{
    return offscreenmgr_->GetDefaultFBO();
}
