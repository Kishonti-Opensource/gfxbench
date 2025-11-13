/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testrunner.h"

#include "testfw.h"
#include "schemas/descriptors.h"
#include "schemas/result.h"

#include "ng/log.h"

#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Thread.h>

#if __APPLE__
#include <TargetConditionals.h>
#endif



class TestRunner::Private
{
public:
    std::unique_ptr<tfw::TestBase> test;
    tfw::ResultGroup result;
    Poco::Mutex mutex;
    std::vector<std::unique_ptr<Poco::SharedLibrary> > libraries;

    void reset() {
        /* synchronized */ {
            Poco::ScopedLock<Poco::Mutex> lock(mutex);
            if(test.get()) test->terminate();
            test.reset();
        }
        libraries.clear();
    }

    static std::string formatPath(const std::string& directory, const std::string& name) {
        std::string result = directory + "/";
#ifndef _WIN32
        result += "lib";
#endif
        result += name;
#ifndef NDEBUG
        result += "_d";
#endif
#ifdef _WIN32
        result += ".dll";
#elif __APPLE__
        result += ".dylib";
#else
        result += ".so";
#endif
        return result;
    }
};



TestRunner::TestRunner():
    d(new Private)
{}



TestRunner::~TestRunner()
{
    d->reset();
}



void TestRunner::loadTest(
        const std::string& pluginPath,
        const tfw::Descriptor& descriptor,
        GraphicsContext& context)
{
    d->reset();

    tfw::Result result;
    result.setTestId(descriptor.testId());
    result.setResultId(descriptor.testId());
    result.setStatus(tfw::Result::OK);
    d->result = tfw::ResultGroup();
    d->result.addResult(result);

#if !TARGET_OS_IPHONE
    tfw::TestFactory::TestFactoryFn factory = nullptr;
    for (auto i = descriptor.preloadLibs().begin(); i != descriptor.preloadLibs().end(); ++i) {
        std::string path = Private::formatPath(pluginPath, *i);
        try {
            Poco::SharedLibrary* rawPointer = new Poco::SharedLibrary(path);
            d->libraries.push_back(std::unique_ptr<Poco::SharedLibrary>(rawPointer));
            if (rawPointer->hasSymbol("create_test_" + descriptor.factoryMethod())) {
                factory = (tfw::TestFactory::TestFactoryFn)rawPointer->getSymbol(
                        "create_test_" + descriptor.factoryMethod());
            }
        } catch (const Poco::LibraryLoadException& e) {
            NGLOG_WARN("Cannot load library: %s (%s)", path, e.what());
            d->result.results().front().setStatus(tfw::Result::FAILED);
            d->result.results().front().setErrorString(BenchmarkException::CANNOT_CREATE_TEST);
            return;
        }
    }
    if (!factory) {
#else
    tfw::TestFactory factory = tfw::TestFactory::test_factory(descriptor.factoryMethod().c_str());
    if (!factory.valid()) {
#endif
        NGLOG_WARN("Cannot create test factory.");
        d->result.results().front().setStatus(tfw::Result::FAILED);
        d->result.results().front().setErrorString(BenchmarkException::CANNOT_CREATE_TEST);
        return;
    }


#if !TARGET_OS_IPHONE
    tfw::TestBase *testPointer = factory().test;
#else
    tfw::TestBase *testPointer = factory.create_test();
#endif
    /* synchronized */ {
        Poco::ScopedLock<Poco::Mutex> lock(d->mutex);
        d->test.reset(testPointer);
    }
    if (!d->test.get()) {
        d->result.results().front().setStatus(tfw::Result::FAILED);
        d->result.results().front().setErrorString(BenchmarkException::CANNOT_CREATE_TEST);
        return;
    }
    d->test->setGraphicsContext(&context);
    d->test->setConfig(descriptor.toJsonString());
    bool success = d->test->init();
    if (!success && !d->test->isCancelled()) {
        d->result.fromJsonString(d->test->result());
    }
}



void TestRunner::runTest()
{
    if (!d->test.get()) {
        throw BenchmarkException(BenchmarkException::CANNOT_CREATE_TEST, "No test is loaded.");
    }
    try {
        Poco::Thread::current()->setPriority(Poco::Thread::PRIO_HIGHEST);
        d->test->run();
        Poco::Thread::current()->setPriority(Poco::Thread::PRIO_NORMAL);
        d->result.fromJsonString(d->test->result());
    } catch (const std::exception& e) {
        NGLOG_ERROR("Test has thrown exception: %s", e.what());
        Poco::Thread::current()->setPriority(Poco::Thread::PRIO_NORMAL);
        d->result.results().front().setStatus(tfw::Result::FAILED);
        d->result.results().front().setErrorString("Test has thrown exception.");
    } catch (...) {
        Poco::Thread::current()->setPriority(Poco::Thread::PRIO_NORMAL);
        throw;
    }
    d->reset();
}



void TestRunner::cancelTest()
{
    Poco::ScopedLock<Poco::Mutex> lock(d->mutex);
    if (d->test.get()) {
        d->test->cancel();
    }
}



tfw::ResultGroup TestRunner::getResult() const
{
    return d->result;
}
