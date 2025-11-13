/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <Poco/Thread.h>
#include <Poco/RunnableAdapter.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/IntValidator.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/SharedLibrary.h>
#include <Poco/RegularExpression.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/FileStream.h>
#include <Poco/StringTokenizer.h>
#include <Poco/NotificationQueue.h>
#include <Poco/StreamCopier.h>
#include "ng/log.h"
#include "ng/scoped_ptr.h"
#include "ng/ptr_vector.h"
#include "ng/stringutil.h"
#include "schemas/descriptors.h"
#include "schemas/compositedescriptor.h"
#include "schemas/result.h"

#include "graphics/windowfactory.h"
#include "graphics/graphicscontext.h"
#include "graphics/graphicswindow.h"

#include "deviceinfo/platformruntimeinfo.h"

#include "testfw.h"
#include "messagequeue.h"

#include <iostream>
#include <ctime>
#include <cmath>


#ifdef HAVE_GSTREAMER
#include "videostream_gstreamer.h"
#endif

using namespace tfw;

std::string testPrefix(const tfw::Descriptor &desc)
{
    if (!desc.dataPrefix().empty())
    {
        return desc.dataPrefix();
    }
    else
    {
        const std::string &test_id = desc.testId();
        size_t pos = test_id.find_first_of("_");
        return test_id.substr(0,pos);
    }
}


class LibraryLoader {
public:
    LibraryLoader(const Poco::Path &path)
        : path_(path)
        , debugPostfix_("_d")
    {}

    void loadLibraries(const std::vector<std::string> &libs)
    {
        std::vector<std::string>::const_iterator it(libs.begin());
        std::vector<std::string>::const_iterator end(libs.end());
        for (; it != end; ++it)
        {
            loadLibrary(*it);
        }
    }

    void setDebugPostfix(const std::string &d)
    {
        debugPostfix_ = d;
    }

    void loadLibrary(const std::string &lib)
    {
        Poco::SharedLibrary plugin;
        Poco::Path path(lib);
        if (path.isAbsolute())
        {
            plugin.load(path.toString());
        }
        else
        {
#ifdef _WIN32
            Poco::Path absPath   = Poco::Path(path_).append(lib + ".dll");
            Poco::Path absPath_d = Poco::Path(path_).append(lib + debugPostfix_ + ".dll");
#elif defined __APPLE__
            Poco::Path absPath   = Poco::Path(path_).append("lib" + lib + ".dylib");
            Poco::Path absPath_d = Poco::Path(path_).append("lib" + lib + debugPostfix_ + ".dylib");
#else
            Poco::Path absPath   = Poco::Path(path_).append("lib" + lib + ".so");
            Poco::Path absPath_d = Poco::Path(path_).append("lib" + lib + debugPostfix_ + ".so");
#endif
#if !NDEBUG
            path = absPath_d;
#else
            path = absPath;
#endif
        }
#if !SILENCE_PLUGIN_LOAD_WARNINGS
        NGLOG_DEBUG("Loading library: %s (%s)", lib, path.toString());
#endif
        try {
            plugin.load(path.toString());
        } catch(const std::exception &e) {
#if !SILENCE_PLUGIN_LOAD_WARNINGS
            NGLOG_WARN("%s", e.what());
            NGLOG_WARN("Failed to load library: %s", path.toString());
#else
			(void)e;
#endif
        }
    }

private:
    Poco::Path path_;
    std::string debugPostfix_;
};


class InitializedNotification : public Poco::Notification
{
public:
    InitializedNotification(const std::string &n) : name(n) {}
    std::string name;
};


class Runner : public Poco::Runnable
{
public:
#if !defined(__QNX__)
    Runner(const tfw::Descriptor &d, Poco::Event &runlock)
#else
    Runner(const tfw::Descriptor &d, Poco::Event &runlock, const std::string &desiredDisplay_)
#endif
    : desc_(d)
    , wnd_(0)
    , runlock_(runlock)
#if defined(__QNX__)
    , desiredDisplay_(desiredDisplay_)
#endif
    {}
    ~Runner()
    {
#ifdef HAVE_GSTREAMER
        if (wnd_ && wnd_->graphics()) wnd_->graphics()->makeCurrent();
        videoStreams_.clear();
#endif
        delete wnd_;
    }
    void prepare(const std::string &gfx, LibraryLoader &loader, const std::string &api)
    {
        loader.loadLibraries(desc_.preloadLibs());
        NGLOG_INFO("Creating test factory for: %s", desc_.testId());
        tfw::TestFactory factory = tfw::TestFactory::test_factory(desc_.factoryMethod().c_str());
        if (factory.valid())
        {
            test_.reset(factory.create_test());
            test_->setName(desc_.testId().c_str());
            prepareGraphics(gfx, api);
            if (desc_.rawConfigHasKey("video_streams_gst"))
            {
                wnd()->graphics()->makeCurrent();
#ifdef HAVE_GSTREAMER
                bool isPipeline = desc_.rawConfigb("video_streams_gst_pipeline", false);
                const std::vector<std::string> &videoStreams = desc_.rawConfigsv("video_streams_gst");
                for(size_t i = 0; i < videoStreams.size(); ++i)
                {
                    GStreamerVideoStream *stream = new GStreamerVideoStream();
                    videoStreams_.push_back(stream);
                    try
                    {
                        if (isPipeline)
                        {
                            stream->openPipeline(videoStreams[i]);
                        }
                        else
                        {
                            Poco::Path path(desc_.env().readPath());
                            path.append("/media/adas2/ui-tests/" + videoStreams[i]);
                            stream->openFile(path.toString());
                        }
                        stream->setLoopEnabled(true);
                        test_->setVideoStream(i, stream);
                    }
                    catch(std::exception &e)
                    {
                        NGLOG_WARN("Failed to open stream: %s: %s", videoStreams[i], e.what());
                    }
                }
#else
                NGLOG_WARN("GStreamer was not found at build time.");
#endif
            }
        }
        else
        {
            std::string err = desc_.testId() + ": no factory for: " + desc_.factoryMethod();
            throw std::runtime_error(err.c_str());
        }
        NGLOG_INFO("Preparing done.");
    }

    virtual void run()
    {
        if (wnd_->graphics()) wnd_->graphics()->makeCurrent();
        desc_.env().setWidth(wnd_->wnd()->width());
        desc_.env().setHeight(wnd_->wnd()->height());
        test_->setConfig(desc_.toJsonString());
        NGLOG_INFO("Initializing %s", desc_.testId());
        bool ok = test_->init();

        // notify app that init ran
        Poco::NotificationQueue::defaultQueue().enqueueNotification(new InitializedNotification(desc_.testId()));

        // wait until we can proceed with run. for composite tests, all run method should run parallel
        runlock_.wait();

        if (ok && !test_->isCancelled())
        {
            NGLOG_INFO("Initialization successful", desc_.testId());
            NGLOG_INFO("Running %s", desc_.testId());
            test_->run();
        }
        test_->terminate();
        NGLOG_INFO("Test ended (%s)", desc_.testId());
    }

    WindowFactory::Wnd *wnd() { return wnd_; }
    tfw::TestBase *test() { return test_.get(); }
    const tfw::TestBase *test() const { return test_.get(); }

private:
    void prepareGraphics(const std::string &gfx, const std::string &api)
    {
        std::string title = test_->name();
        WindowFactory f;
        f.setApi(desc_.env().graphics().versions(), api);
        f.setSize(desc_.env().width(), desc_.env().height());
#if defined(__QNX__)
        f.setDesiredDisplay(desiredDisplay_);

        if(desc_.env().width() == 0 && desc_.env().height() == 0)
        {
            f.setFullscreen(true);
        }
        else
        {
#endif
        f.setFullscreen(desc_.env().graphics().isFullScreen());
#if defined(__QNX__)
        }

        f.setPixelFormat(desc_.env().graphics().getPixelFormat());
        f.setNumBuffers(desc_.env().graphics().getNumBuffers());
        f.setInterval(desc_.env().graphics().getInterval());
#endif
        tfw::GLFormat format(desc_.env().graphics().config().red(),
            desc_.env().graphics().config().green(),
            desc_.env().graphics().config().blue(),
            desc_.env().graphics().config().alpha(),
            desc_.env().graphics().config().depth(),
            desc_.env().graphics().config().stencil(),
            desc_.env().graphics().config().samples(),
            desc_.env().graphics().config().isExact());
        f.setFormat(format);
		f.setVsync( desc_.env().graphics().config().isVsync() );
        wnd_ = f.create(gfx);
        require(wnd_ != NULL);
        test_->setRuntimeInfo(&runtimeInfo_);
        test_->setGraphicsContext(wnd_->graphics());
        test_->setMessageQueue(wnd_->queue());
    }

    tfw::Descriptor desc_;
    ng::scoped_ptr<tfw::TestBase> test_;
    WindowFactory::Wnd *wnd_;
    tfw::PlatformRuntimeInfo runtimeInfo_;
    Poco::Event &runlock_;
#if defined(__QNX__)
    std::string desiredDisplay_;
#endif
#ifdef HAVE_GSTREAMER
    ng::ptr_vector<GStreamerVideoStream> videoStreams_;
#endif
};

struct HandleEvents
{
    HandleEvents() : shouldClose(false) {}
    void operator() (Runner &r)
    {
        WindowFactory::Wnd *w = r.wnd();
        w->wnd()->pollEvents();
        if (w->wnd()->shouldClose())
            shouldClose = true;
    }
    bool shouldClose;
};
void cancelRunner(Runner &r) { r.test()->cancel(); }


class HostApp : public Poco::Util::Application
{
public:
    typedef std::vector<tfw::Descriptor> Descriptors;

    HostApp()
    : autotest_(false)
    , helpRequested_(false)
    , verbosity_(1)
    {
        setUnixOptions(true);
        appLog_.addSink(&printfSink_);
        ng::Logger::setTheGlobal(&appLog_);
    }
    virtual ~HostApp()
    {
        ng::Logger::setTheGlobal(0);
    }

    virtual int main(const std::vector<std::string> &/*args*/)
    {
        if (helpRequested_) return 0;

#if !defined(__QNX__)
        std::string gl_api = config().getString("gl_api", "desktop_core");
#else
        std::string gl_api = config().getString("gl_api", "gles");
#endif /* __QNX__ */

        Poco::Path basePath = config().getString("application.dir");
        if (basePath.depth() > 0) {
            std::string directory = basePath.directory(basePath.depth() - 1);
            if (directory == "bin") {
                basePath.popDirectory();
            } else if (directory == "MacOS") {
                basePath.popDirectory().pushDirectory("Resources");
            }
        }
        basePath_ = config().getString("base_path", basePath.toString());
        NGLOG_INFO("Base path: %s", basePath_.toString());

        configDir_ = Poco::Path(basePath_).append("config");
        autotest_ = config().hasOption("autotest");
        std::string gfx = config().getString("gfx", "egl");
        int cldevice = config().getInt("cl_device", 0);
        //bool ignoreError = config().hasOption("force");
        std::string testFilterRegex = ".+";

        resultPath_ = basePath_;
        if (config().hasOption("resultdir"))
        {
            Poco::Path rdir = config().getString("resultdir");
            if (rdir.isAbsolute())
            {
                resultPath_ = rdir;
            }
            else
            {
                resultPath_.append(rdir);
            }
        }
        else
        {
            resultPath_.append("results");

            char datedir[20];
            time_t timestamp;
            time(&timestamp);
            strftime(datedir,20,"%Y_%m_%d_%H_%M_%S",localtime(&timestamp));
            resultPath_.append(datedir);
        }

        if (config().hasOption("filter"))
        {
            testFilterRegex = config().getString("filter");
        }
        Poco::Path pluginPath = Poco::Path(basePath_).append("plugins");
        if (config().hasOption("plugin_path"))
        {
            pluginPath = Poco::Path(config().getString("plugin_path"));
        }
        LibraryLoader loader(pluginPath.makeDirectory());
        if (config().hasOption("debug_postfix"))
        {
            loader.setDebugPostfix(config().getString("debug_postfix"));
        }
        if (autotest_)
        {
            loadTestsRegex(testFilterRegex);
        }

        if (testIds_.empty())
        {
            throw Poco::Exception("No test selected. Please specify some tests with '-t test_id' or use '--autotest'");
        }
        for (std::vector<std::string>::const_iterator testId = testIds_.begin(); testId != testIds_.end(); ++testId)
        {
#if defined(__QNX__)
            try
            {
#endif
            Poco::Path file(configDir_);
			if (testId->find(".json") != std::string::npos )
			{
				file.append(Poco::Path(*testId));
			}
			else
			{
				file.append(Poco::Path(*testId + ".json"));
			}
			FILE* f = fopen(file.toString().c_str(), "rb");
			if (f == nullptr)
			{
				NGLOG_WARN("File not found: %s", file.toString());
				continue;
			}
            tfw::CompositeDescriptor compositeDescriptor;
            compositeDescriptor.fromJsonFile(file.toString());

            std::vector<tfw::Descriptor> descriptors = compositeDescriptor.descriptors();
            require(!descriptors.empty());

            ng::ptr_vector<Runner> runners;
            Poco::Event syncRunnerEvent(false);
            Poco::ThreadPool pool((int)descriptors.size());

            for (std::vector<tfw::Descriptor>::const_iterator di = descriptors.begin(); di != descriptors.end(); ++di)
            {
                tfw::Descriptor d = *di;
                if (config().has("width") && config().has("height"))
                {
                    d.env().setWidth(config().getInt("width"));
                    d.env().setHeight(config().getInt("height"));
                }
                if (config().has("fullscreen"))
                {
                    if (descriptors.size() == 1)
                    {
                        d.env().graphics().setFullScreen(config().getInt("fullscreen") != 0);
                    }
                    else
                    {
                        NGLOG_WARN("ignore fullscreen mode for composite test: %s[%s]", *testId, d.testId());
                    }
                }

#if defined(__QNX__)
		if(config().has("format")) {
			d.env().graphics().setPixelFormat(config().getString("format"));
		} else {
			d.env().graphics().setPixelFormat("auto");
		}

		if(config().has("nbuffers")) {
			d.env().graphics().setNumBuffers(config().getInt("nbuffers"));
		} else {
			d.env().graphics().setNumBuffers(4);
		}

		if(config().has("interval")) {
			d.env().graphics().setInterval(config().getInt("interval"));
		} else {
			d.env().graphics().setInterval(0);
		}
#endif /* __QNX__ */

                std::string prefix = testPrefix(d);
                Poco::Path readPath = Poco::Path(basePath_).append(Poco::Path(std::string("data/") + prefix));
                readPath.makeDirectory();

                d.env().setReadPath(readPath.toString());
                d.env().setWritePath(readPath.toString());
                d.env().compute().setConfigIndex(cldevice);
                if(config().has("mtl_device"))
                {
                    std::vector<tfw::ApiDefinition> api = d.env().graphics().versions();
                    int index = config().getInt("mtl_device");
                    for(size_t i = 0; i < api.size(); i++)
                    {
                        api[i].setDeviceIndex(index);
                    }
                    d.env().graphics().setVersions(api);
                    d.env().compute().setConfigIndex(index);
                }

                for (std::map<std::string, double>::const_iterator h = rawn.begin(); h != rawn.end(); ++h) { d.setRawConfig(h->first, h->second); }
                for (std::map<std::string, std::string>::const_iterator h = raws.begin(); h != raws.end(); ++h) { d.setRawConfig(h->first, h->second); }
                for (std::map<std::string, bool>::const_iterator h = rawz.begin(); h != rawz.end(); ++h) { d.setRawConfig(h->first, h->second); }
#if defined(__QNX__)
                Runner *r = new Runner(d, syncRunnerEvent, desiredDisplay_);
#else
                Runner *r = new Runner(d, syncRunnerEvent);
#endif
                // hold a reference
                runners.push_back(r);

                r->prepare(gfx, loader, gl_api);
                if (r->wnd() && r->wnd()->graphics())
                {
                    // detach context from the main thread
                    r->wnd()->graphics()->detachThread();
                }
                // start test initialization
                pool.start(*r);
            }

            size_t numOfInitialized = 0;

            HandleEvents handleEvents;
            while(pool.available() < pool.capacity())
            {
                Poco::Notification::Ptr n = Poco::NotificationQueue::defaultQueue().dequeueNotification();
                if (n.get() && dynamic_cast<InitializedNotification*>(n.get()))
                {
                    ++numOfInitialized;
                    if (numOfInitialized == runners.size())
                    {
                        syncRunnerEvent.set();
                    }
                }

                // handle window events
                handleEvents = std::for_each(runners.begin(), runners.end(), handleEvents);

                // one of the windows was requested to close
                if (handleEvents.shouldClose)
                {
                    // cancel all runners
                    std::for_each(runners.begin(), runners.end(), cancelRunner);
                }
                else
                {
                    Poco::Thread::sleep(16);
                }
            }
            pool.joinAll();

            saveResult(*testId, runners);
        }
#if defined(__QNX__)
            catch(std::exception &e)
            {
                // Need to catch exceptions so that if there are any tests that need to be run after the current one,
                // they are still executed (have to prevent entire program from being aborted due to uncaught exception)

                printf("Exception occurred when running the %s benchmark: %s \n", testId->c_str(), e.what());
            }
        }
#endif
        return 0;
    }

protected:
    virtual void defineOptions(Poco::Util::OptionSet &options)
    {
#if defined(__QNX__)
        Poco::Util::Option format("format", "format", "Set pixel format for corresponding EGL and Vulkan configurations: auto, rgb565, rgba8888, bgra8888, rgba1010102, bgra1010102");
        format
            .required(false)
            .argument("pixel format", true)
            .binding("format");
        options.addOption(format);

        Poco::Util::Option interval("interval", "interval", "Set interval between frame swap, default is 1, which is at vsync");
        interval
            .required(false)
            .argument("swap interval", true)
            .binding("interval");
        options.addOption(interval);

        Poco::Util::Option nbuffers("nbuffers", "nbuffers", "Set number of buffers to use when rendering, default is 4");
        nbuffers
            .required(false)
            .argument("number of buffers", true)
            .binding("nbuffers");
        options.addOption(nbuffers);

        Poco::Util::Option display_option("display","d", "Choose what display to use");
        display_option
            .required(false)
            .argument("value", true)
            .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleExtraArgs));
        options.addOption(display_option);

#endif /* __QNX__ */

        Poco::Util::Option gl_api("gl_api", "", "specify OpenGL (ES) context type. Supported values: desktop_core, desktop_compat, gles");
        gl_api
            .required(false)
            .argument("API", true)
            .binding("gl_api");
        options.addOption(gl_api);

        Poco::Util::Option basePath("base_path", "b", "specify the base directory");
        basePath
        .required(false)
        .argument("dir", true)
        .binding("base_path");
        options.addOption(basePath);

        Poco::Util::Option pluginPath("plugin_path", "", "specify the plugins directory");
        pluginPath
        .required(false)
        .argument("dir", true)
        .binding("plugin_path");
        options.addOption(pluginPath);

        Poco::Util::Option width("width", "w", "specify the window width");
        width
        .required(false)
        .argument("value", true)
        .binding("width").validator(new Poco::Util::IntValidator(1,8192));
        options.addOption(width);

        Poco::Util::Option height("height", "h", "specify the window height");
        height
        .required(false)
        .argument("value", true)
        .binding("height").validator(new Poco::Util::IntValidator(1,8192));
        options.addOption(height);

        Poco::Util::Option fullscreen("fullscreen", "", "run the application in full screen mode");
        fullscreen
        .required(false)
        .argument("value", true)
        .binding("fullscreen").validator(new Poco::Util::IntValidator(0,1));
        options.addOption(fullscreen);

        Poco::Util::Option autotest("autotest", "", "run all tests matching filter");
        autotest
        .required(false)
        .binding("autotest");
        options.addOption(autotest);

        Poco::Util::Option filter("filter", "", "set test_id filter regular expression");
        filter
        .required(false)
        .argument("regex")
        .binding("filter");
        options.addOption(filter);

        Poco::Util::Option gfx("gfx", "", "set graphics type");
        gfx
        .required(true)
        .argument("GFX")
        .binding("gfx");
        options.addOption(gfx);

        Poco::Util::Option debug_postfix("debug_postfix", "", "debug postfix for the plugin file name");
        debug_postfix
        .required(false)
        .argument("_d", true)
        .binding("debug_postfix");
        options.addOption(debug_postfix);


        Poco::Util::Option force("force", "", "forces the application to continue if some tests fail");
        force
        .required(false)
        .repeatable(false)
        .binding("force");
        options.addOption(force);

        Poco::Util::Option test_id("test_id", "t", "comma separated list of test ids");
        test_id
        .required(false)
        .repeatable(true)
        .argument("value", true)
        .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::addTest));
        options.addOption(test_id);

        Poco::Util::Option help("help", "?", "show help");
        help.callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleHelp));
        options.addOption(help);

        Poco::Util::Option verbose("verbose", "v", "set verbose level");
        verbose
        .required(false)
        .repeatable(true)
        .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleVerbose));
        options.addOption(verbose);

        Poco::Util::Option resultdir("resultdir", "", "set directory where to save the results");
        resultdir
        .required(false)
        .argument("result_path", true)
        .binding("resultdir");
        options.addOption(resultdir);

        Poco::Util::Option cldevice("cl_device", "c", "select OpenCL device index");
        cldevice
        .required(false)
        .argument("index", true)
        .binding("cl_device").validator(new Poco::Util::IntValidator(0,8));
        options.addOption(cldevice);
        
        Poco::Util::Option mtldevice("mtl_device", "", "select Metal device index");
        mtldevice
        .required(false)
        .argument("index", true)
        .binding("mtl_device").validator(new Poco::Util::IntValidator(0,8));
        options.addOption(mtldevice);

        Poco::Util::Option extra_args_ei("ei", "", "integer arg");
        extra_args_ei
            .required(false)
            .repeatable(true)
            .argument("value", true)
            .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleExtraArgs));
        options.addOption(extra_args_ei);

        Poco::Util::Option extra_args_ef("ef", "", "float arg");
        extra_args_ef
            .required(false)
            .repeatable(true)
            .argument("value", true)
            .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleExtraArgs));
        options.addOption(extra_args_ef);

        Poco::Util::Option extra_args_es("es", "", "string arg");
        extra_args_es
            .required(false)
            .repeatable(true)
            .argument("value", true)
            .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleExtraArgs));
        options.addOption(extra_args_es);

        Poco::Util::Option extra_args_ez("ez", "", "boolean arg");
        extra_args_ez
            .required(false)
            .repeatable(true)
            .argument("value", true)
            .callback(Poco::Util::OptionCallback<HostApp>(this, &HostApp::handleExtraArgs));
        options.addOption(extra_args_ez);

        Application::defineOptions(options);
    }

    virtual void handleHelp(const std::string &, const std::string &)
    {
        Poco::Util::HelpFormatter help(options());
        help.setUnixStyle(true);
        help.format(std::cerr);
        stopOptionsProcessing();
        helpRequested_ = true;
    }

    virtual void initialize(Application &self)
    {
        Application::initialize(self);
    }

    virtual void reinitialize(Application &self)
    {
        Application::reinitialize(self);
    }

private:
    void saveResult(const std::string &testId, ng::ptr_vector<Runner> &runners)
    {
        tfw::ResultGroup rg;
        for (size_t i = 0; i < runners.size(); ++i)
        {
            tfw::ResultGroup g0;
            requireex(g0.fromJsonString(runners[i].test()->result()));
            rg.results().insert(rg.results().end(), g0.results().begin(), g0.results().end());
        }
        Poco::FileOutputStream fs(createResultFile(testId).path());
        fs << rg.toString();
        fs.close();

		//composite result
		if (runners.size() > 1)
		{
			double composite_score = 1.0f;
			bool all_ok = true;

			for (size_t i = 0; i < runners.size(); ++i)
			{
				tfw::ResultGroup g0;
				requireex(g0.fromJsonString(runners[i].test()->result()));
				composite_score *= g0.results()[0].score();

				if (g0.results()[0].status() != Result::OK)
					all_ok = false;
			}

			composite_score = pow(composite_score, (1.0 / (double)runners.size()));

			if (all_ok)
				NGLOG_INFO("%s result: %s", testId, composite_score);
			else
				NGLOG_INFO("Some of test components are FAILED ");
		}
    }
    void handleResult(tfw::TestBase &test)
    {
        Poco::FileOutputStream fs(createResultFile(test.name()).path());
        fs << test.result();
        fs.close();
    }

    Poco::File createResultFile(const std::string &testId)
    {
        Poco::File resultDir(resultPath_.makeDirectory());
        if (!resultDir.exists())
        {
            resultDir.createDirectories();
        }
        Poco::Path filePath = resultPath_;
        filePath.append(testId);
        filePath.setExtension("json");
        return Poco::File(filePath);
    }

    void handleVerbose(const std::string&, const std::string&)
    {
        ++verbosity_;
    }

    void handleExtraArgs(const std::string& name, const std::string& value)
    {
#if defined(__QNX__)
        // Separate if statement here as display arg does not match format of
        // other extra args, leading to a thrown exception
        if(name == "display")
        {
            desiredDisplay_ = value;
            return;
        }
#endif
        Poco::StringTokenizer tokenizer(value, "=");

        if (tokenizer.count() != 2)
        {
            throw Poco::Exception("Parameter parsing error: \"" + value + "\" is wrong please use valid key=value pair");
        }
        else
        {
            std::string sub_key = tokenizer[0];
            std::string sub_value = tokenizer[1];

            //gfx specific "-" syntax;
            if (sub_key[0] == '-')
                sub_key = sub_key.substr(1, sub_key.length());

            if (name == "ei")
            {
                rawn[sub_key] = atol(sub_value.c_str());
            }
            else if (name == "ef")
            {
                rawn[sub_key] = ng::atof(sub_value.c_str());
            }
            else if (name == "es")
            {
                raws[sub_key] = sub_value;
            }
            else if (name == "ez")
            {
                if (sub_value == "true" || sub_value == "1") {
                    rawz[sub_key] = true;
                }
                else if (sub_value == "false" || sub_value == "0") {
                    rawz[sub_key] = false;
                }
            }
        }
    }

    void loadTestsRegex(const std::string &regex)
    {
        Poco::Path configPath(basePath_);
        Poco::RegularExpression re(regex);
        configPath.append("config/");
        Poco::DirectoryIterator it(configPath);
        Poco::DirectoryIterator end;
        for(; it != end; ++it)
        {
            Poco::Path path = it.path();
            std::string basename = path.getBaseName();
            if (path.getExtension() == "json" && re.match(basename))
            {
                testIds_.push_back(basename);
            }
        }
    }
    void addTest(const std::string &, const std::string &value)
    {
        Poco::StringTokenizer tokens(value, ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY|Poco::StringTokenizer::TOK_TRIM);
        testIds_.insert(testIds_.end(), tokens.begin(), tokens.end());
    }

    Poco::Path configDir_;
    Poco::Path basePath_;
    Poco::Path resultPath_;
    std::vector<std::string> testIds_;
    bool autotest_;
    bool helpRequested_;
    int32_t verbosity_;
    ng::Logger appLog_;
    ng::LogSinkPrintf printfSink_;
#if defined(__QNX__)
    std::string desiredDisplay_;
#endif
    std::map<std::string, double> rawn;
    std::map<std::string, bool> rawz;
    std::map<std::string, std::string> raws;
};

POCO_APP_MAIN(HostApp)
