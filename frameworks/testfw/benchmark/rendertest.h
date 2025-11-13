/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RENDERTEST_H_
#define RENDERTEST_H_

#include "testfw.h"
#include "schemas/descriptors.h"
#include "schemas/result.h"
#include "ng/scoped_ptr.h"
#include "ng/timer.h"
#include "benchmark/offscreenmanager.h"

namespace tfw {

class RenderTest : public TestBase
{
public:
    RenderTest(bool offscreen = false);
    virtual bool init();
    virtual bool terminate();
    virtual std::string result();

    virtual void preRender();
    virtual void postRender();
    virtual void flushRender();
    virtual void cleanup();
    virtual void updateResult();
    void startMeasurement();
    void stopMeasurement();

    int32_t renderWidth() const;
    int32_t renderHeight() const;
    uint32_t defaultFbo() const;

    void setDuration(double duration);
    bool durationExpired();
protected:
    Descriptor descriptor_;
    ResultGroup result_;
    double duration_;
    ng::scoped_ptr<OffscreenManager> offscreenmgr_;
    ng::cpu_timer timer_;

private:
    int32_t width_;
    int32_t height_;
    bool offscreen_;
    uint32_t frames_;
};

}

#endif  // RENDERTEST_H_
