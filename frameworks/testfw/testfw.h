/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TESTFW_H_
#define TESTFW_H_

// this version must be updated if TestBase class is modified
#define TFW_VERSION 6

class GraphicsContext;
class VideoStream;
#include <string>
#include <map>
#include <stdint.h>

namespace tfw {

class MessageQueue;
class RuntimeInfo;

class TestBase {
public:
    TestBase()
        : ctx_(NULL)
        , msgQueue_(NULL)
        //, runtimeInfo_(&nullRuntimeInfo_)
        , runtimeInfo_(0)
        , cancelled_(false)
        , armed_(true)
    {}
    virtual ~TestBase() {}
    virtual const std::string &config() const { return config_; }
    virtual void setConfig (const std::string &config) { config_ = config; }
    virtual bool init() { return true; }
    virtual GraphicsContext *graphicsContext() { return ctx_; }
    virtual void setGraphicsContext(GraphicsContext *ctx) { ctx_ = ctx; }
    virtual void run() = 0;
    virtual bool step() { return false; }
    virtual std::string result() = 0;
    virtual std::string version() { return ""; }
    virtual bool terminate() { return true; }
    virtual void cancel()
    {
        cancelled_ = true;
        onCancel();
    }
    virtual bool isCancelled() const { return cancelled_; }
    virtual void setArmed(bool b) { armed_ = b; }
    virtual bool isArmed() const { return armed_; }
    virtual void setName(const char *name) { name_ = name; }
    virtual const std::string &name() const { return name_; }
    virtual float progress() { return 0; }
    virtual void setMessageQueue(MessageQueue *msgQueue) { msgQueue_ = msgQueue; }

    virtual void setVideoStream(VideoStream *stream) { setVideoStream(0, stream); }
    virtual void setVideoStream(int32_t name, VideoStream *stream)
    {
        if (stream == 0) {
            videoStreams_.erase(name);
        } else {
            videoStreams_[name] = stream;
        }
    }
    virtual VideoStream *videoStream(int32_t name) { return videoStreams_.find(name)->second; }
    virtual int32_t videoStreamCount() const { return (int32_t) videoStreams_.size(); }
    virtual void setRuntimeInfo(RuntimeInfo *runtimeInfo) {
        runtimeInfo_ = runtimeInfo;
    }

protected:
    virtual void onCancel() {}
    GraphicsContext *ctx_;
    MessageQueue *msgQueue_;
    RuntimeInfo *runtimeInfo_;

private:
    bool cancelled_;
    bool armed_;
    std::string name_;
    std::string config_;
    std::map<int32_t, VideoStream *> videoStreams_;
};


class TestFactory
{
public:
#ifndef SWIG
    // implementation detail
    struct Holder
    {
        TestBase *test;
        int version;
    };
#endif
    typedef Holder (*TestFactoryFn)();
    static TestFactory test_factory(const char *name);
    TestBase *create_test();
    bool valid() const;

private:
    static void* gfx5_handle;
    static void* gfx4_handle;
    TestFactory();
    TestFactoryFn factory_method_;
};

}


#ifdef WIN32
#define APICALL __declspec(dllexport)
#elif __GNUC__ >= 4
#define APICALL __attribute__((visibility("default")))
#else
#define APICALL
#endif

#define CREATE_FACTORY(NAME,CONSTRUCTOR) \
extern "C" APICALL tfw::TestFactory::Holder create_test_##NAME() \
{\
    tfw::TestFactory::Holder holder = {0, 0};\
    holder.test = new CONSTRUCTOR;\
    holder.test->setName(#NAME);\
    holder.version = TFW_VERSION;\
    return holder;\
}\


#endif  // TESTFW_H_
