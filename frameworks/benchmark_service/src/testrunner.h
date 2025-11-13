/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include "benchmarkexception.h"

#include <memory>



class GraphicsContext;
namespace tfw {
    class Descriptor;
    class ResultGroup;
};



class TestRunner
{
public:
    TestRunner();
    virtual ~TestRunner();
    virtual void loadTest(
            const std::string& pluginPath,
            const tfw::Descriptor& descriptor,
            GraphicsContext& context);
    virtual void runTest();
    virtual void cancelTest();
    virtual tfw::ResultGroup getResult() const;
private:
    class Private;
    std::unique_ptr<Private> d;
};



#endif // TESTRUNNER_H
