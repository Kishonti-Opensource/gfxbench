/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testfw.h"
#include "ng/log.h"
#include "schemas/descriptors.h"
#include "schemas/result.h"

using namespace tfw;

class Fail : public TestBase
{
public:
    Fail()
    : initOk_(false)
    {
    }
    ~Fail()
    {
        NGLOG_INFO("Fail test destructing");
    }
    bool init()
    {
        Result r;
        r.setTestId(name());
        r.setResultId(name());
        r.setStatus(Result::Status::FAILED);
        result_.addResult(r);

        Descriptor d;
        std::string err;
        bool ok = Descriptor::fromJsonString(config(), &d, &err);
        if (!ok){
            NGLOG_ERROR("%s: init failed: %s", name(), err);
            return false;
        }
        initOk_ = d.rawConfigb("init_ok", initOk_);
        NGLOG_DEBUG("%s: conifgured init status: %s", name(), initOk_);
        return initOk_;
    }

    void run()
    {
    }

    std::string result()
    {
        return result_.toString();
    }

private:
    ResultGroup result_;
    bool initOk_;
};


CREATE_FACTORY(tfw_fail, Fail);
