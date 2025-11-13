/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef ANDROID
#include <android/native_window.h>
#endif
#include "ng/log.h"
#include "ovrgraphicscontext.h"
#include <vector>
#include "ng/macro_utils.h"
#ifdef HAVE_EPOXY
#include <epoxy/gl.h>
#else
#include <GLES2/gl2.h>
#ifndef EGL_CONTEXT_MAJOR_VERSION_KHR
#define EGL_CONTEXT_MAJOR_VERSION_KHR 0x3098
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30FB
#if !defined( EGL_OPENGL_ES3_BIT_KHR )
#define EGL_OPENGL_ES3_BIT_KHR      0x0040
#endif
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1800)
#define snprintf sprintf_s
#endif


JavaVM *JNU_GetJVM(); // defined in the swig interface file
JNIEnv *JNU_GetEnv(); // defined in the swig interface file


NG_TABLE_START(GL_ERROR_TABLE)
  NG_TABLE_ITEM0(GL_NO_ERROR)
  NG_TABLE_ITEM0(GL_INVALID_ENUM)
  NG_TABLE_ITEM0(GL_INVALID_VALUE)
  NG_TABLE_ITEM0(GL_INVALID_OPERATION)
#ifdef HAVE_EPOXY
  NG_TABLE_ITEM0(GL_STACK_OVERFLOW)
  NG_TABLE_ITEM0(GL_STACK_UNDERFLOW)
#endif
  NG_TABLE_ITEM0(GL_OUT_OF_MEMORY)
  NG_TABLE_ITEM0(GL_INVALID_FRAMEBUFFER_OPERATION)
NG_TABLE_END(GL_ERROR_TABLE)

NG_TABLE_START(EGL_ERROR_TABLE)
  NG_TABLE_ITEM0(EGL_SUCCESS)
  NG_TABLE_ITEM0(EGL_NOT_INITIALIZED)
  NG_TABLE_ITEM0(EGL_BAD_ACCESS)
  NG_TABLE_ITEM0(EGL_BAD_ALLOC)
  NG_TABLE_ITEM0(EGL_BAD_ATTRIBUTE)
  NG_TABLE_ITEM0(EGL_BAD_CONTEXT)
  NG_TABLE_ITEM0(EGL_BAD_CONFIG)
  NG_TABLE_ITEM0(EGL_BAD_CURRENT_SURFACE)
  NG_TABLE_ITEM0(EGL_BAD_DISPLAY)
  NG_TABLE_ITEM0(EGL_BAD_SURFACE)
  NG_TABLE_ITEM0(EGL_BAD_MATCH)
  NG_TABLE_ITEM0(EGL_BAD_PARAMETER)
  NG_TABLE_ITEM0(EGL_BAD_NATIVE_PIXMAP)
  NG_TABLE_ITEM0(EGL_BAD_NATIVE_WINDOW)
  NG_TABLE_ITEM0(EGL_CONTEXT_LOST)
NG_TABLE_END(EGL_ERROR_TABLE)

namespace {
    void logAllEglConfigs(EGLDisplay display);
    void logEglConfig(EGLDisplay display, EGLConfig config, bool printHeader = false);
}

OVRGraphicsContext::OVRGraphicsContext()
    : display_(EGL_NO_DISPLAY)
    , config_(0)
    , surface_(EGL_NO_SURFACE)
    , context_(EGL_NO_CONTEXT)
    , versionMajor_(1)
    , versionMinor_(0)
    , surfaceWidth_(-1)
    , surfaceHeight_(-1)
    , selectedConfig_(0)
    , useDefaultChooseConfig_(false)
    , verboseLogging_(true)
    , eglVersionMajor_(1)
    , eglVersionMinor_(0)
{
}


OVRGraphicsContext::OVRGraphicsContext(const tfw::GLFormat &format)
{
    setFormat(format);
}


OVRGraphicsContext::~OVRGraphicsContext()
{
    if (isValid())
    {
        destroy();
    }
}


bool OVRGraphicsContext::initWindowSurface(EGLNativeWindowType window) {
    NGLOG_INFO("OVRGraphicsContext needs an activityObject");
    return false;
}

bool OVRGraphicsContext::initWindowSurface(EGLNativeWindowType window, jobject activityObject)
{
#ifdef WITH_OVR_SDK
    NGLOG_INFO("initOVRWindowSurface started");
#else
	NGLOG_INFO("initOVRWindowSurface DUMMY started");
#endif
    EGLContext ovrContext_ = EGL_NO_CONTEXT;

#ifdef WITH_OVR_SDK
    java_.Vm = JNU_GetJVM();
    java_.Env = JNU_GetEnv();
    java_.ActivityObject = java_.Env->NewGlobalRef( activityObject );

    const ovrInitParms initParms = vrapi_DefaultInitParms( &java_ );

    int32_t initResult = vrapi_Initialize( &initParms );
    if ( initResult != VRAPI_INITIALIZE_SUCCESS )
    {
        char const * msg = initResult == VRAPI_INITIALIZE_PERMISSIONS_ERROR ? 
                                        "Thread priority security exception. Make sure the APK is signed." :
                                        "VrApi initialization error.";
        NGLOG_ERROR("vrapi_Initialize failed with: %s", msg);

        return false;
    }
#endif

    if ((display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        NGLOG_ERROR("eglGetDisplay() returned EGL_NO_DISPLAY");
        return false;
    }

    if (!eglInitialize(display_, &eglVersionMajor_, &eglVersionMinor_)) {
        NGLOG_ERROR("eglInitialize() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        return false;
    }

    initContextAttribs();
    NGLOG_INFO("Display is %s, EGL version: %s.%s", display_, eglVersionMajor_, eglVersionMinor_);
    NGLOG_DEBUG("Using default eglChooseConfig: %s", useDefaultChooseConfig_);

    //OVR context needs these parameters
    format_.red = 8;
    format_.green = 8;
    format_.blue = 8;
    format_.alpha = 8;
    format_.depth = 0;
    format_.stencil = 0;
    format_.fsaa = 0;

    NGLOG_DEBUG("format: rgba: %s %s %s %s depth: %s stencil: %s fsaa: %s",
        format_.red, format_.green, format_.blue, format_.alpha, format_.depth, format_.stencil, format_.fsaa);
    NGLOG_DEBUG("chooseConfig()");
    config_ = chooseConfig();
    if (config_ == 0) {
        NGLOG_ERROR("No suitable EGL configuration found!");
        destroy();
        return false;
    }

    if (verboseLogging_)
    {
        logAllEglConfigs(display_);
        NGLOG_INFO("Selected EGL cofiguration");
        logEglConfig(display_, config_);
    }



#ifdef ANDROID
    EGLint formatAttrib = 0;
    if (!eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &formatAttrib)) {
        NGLOG_ERROR("eglGetConfigAttrib() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    if (window == 0) {
        NGLOG_ERROR("EGL window is null!");
        destroy();
        return false;
    }
    ANativeWindow_setBuffersGeometry(window, 0, 0, formatAttrib);
#endif


    EGLint surfaceAttribs[] = { EGL_NONE };

#ifdef WITH_OVR_SDK
    if (!(ovrContext_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttribs_.data()))) {
        NGLOG_ERROR("eglCreateContext() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    if (!(context_ = eglCreateContext(display_, config_, ovrContext_, contextAttribs_.data()))) {
        NGLOG_ERROR("eglCreateContext() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    if (!(surface_ = eglCreatePbufferSurface(display_, config_, surfaceAttribs))) {
        NGLOG_ERROR("eglCreateWindowSurface() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        return false;
    }

    if (!(ovr_surface_ = eglCreateWindowSurface(display_, config_, window, surfaceAttribs))) {
        NGLOG_ERROR("eglCreateWindowSurface() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        return false;
    }
        
    if (!eglMakeCurrent(display_, ovr_surface_, ovr_surface_, ovrContext_)) {
        NGLOG_ERROR("eglMakeCurrent() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }
#else
	if (!(surface_ = eglCreateWindowSurface(display_, config_, window, surfaceAttribs))) {
        NGLOG_ERROR("eglCreateWindowSurface() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    if (!(context_ = eglCreateContext(display_, config_, ovrContext_, contextAttribs_.data()))) {
        NGLOG_ERROR("eglCreateContext() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        NGLOG_ERROR("eglMakeCurrent() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        destroy();
        return false;
    }

    eglQuerySurface(display_, surface_, EGL_WIDTH, &surfaceWidth_);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &surfaceHeight_);
    eglGetConfigAttrib (display_, config_, EGL_CONFIG_ID, &selectedConfig_);

    if (updateVersionFromContext()) {
        window_ = window;
        return true;
    } else {
        NGLOG_ERROR("Incompatible context created: %s for request: %s.%s", glGetString(GL_VERSION), versionMajor_, versionMinor_);
        destroy();
        return false;
    }
#endif

#ifdef WITH_OVR_SDK
    ovrModeParams_ = vrapi_DefaultModeParms( &java_ );

    NGLOG_INFO( "        eglGetCurrentSurface( EGL_DRAW ) = %s", eglGetCurrentSurface( EGL_DRAW ) );
    ovrMobile_ = vrapi_EnterVrMode( &ovrModeParams_ );  
    NGLOG_INFO( "        eglGetCurrentSurface( EGL_DRAW ) = %s", eglGetCurrentSurface( EGL_DRAW ) );

    return true;

#else
    return false;

#endif
}

void OVRGraphicsContext::setFormat(const tfw::GLFormat &format)
{
    format_ = format;
}


const tfw::GLFormat &OVRGraphicsContext::format() const
{
    return format_;
}

/* This is left here to preserve behaviour of older tests */
EGLConfig OVRGraphicsContext::chooseConfig() const
{
    EGLint num = 0;
    EGLint id = 0;
    EGLConfig config = 0;
    int idxNoAlpha = -1;
    int idxAlpha = -1;
    EGLBoolean success = eglGetConfigs (display_, 0, 0, &num);

    if(!success)
    {
        NGLOG_ERROR("eglGetConfigs() returned error %s", EGL_ERROR_TABLE(eglGetError()));
        return config;
    }

    std::vector<EGLConfig> configs(num);
    eglGetConfigs(display_, configs.data(), num, &num);
    for(int i = 0; i < num; ++i)
    {
        EGLint value;

        eglGetConfigAttrib(display_, configs[i], EGL_SURFACE_TYPE, &value);
        if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT))
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_RENDERABLE_TYPE, &value);
        if((value & EGL_OPENGL_ES2_BIT) != EGL_OPENGL_ES2_BIT)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_CONFIG_CAVEAT, &value);
        if(value == EGL_SLOW_CONFIG)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_RED_SIZE, &value);
        if(value < format_.red)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_GREEN_SIZE, &value);
        if(value < format_.green)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_BLUE_SIZE, &value);
        if(value < format_.blue)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_ALPHA_SIZE, &value);
        if(value < 8)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_DEPTH_SIZE, &value);
        if(value != 0)
            continue;

        eglGetConfigAttrib(display_, configs[i], EGL_STENCIL_SIZE, &value);
        if(value < format_.stencil)
            continue;

        if(format_.fsaa > 0 )
        {
            eglGetConfigAttrib(display_, configs[i], EGL_SAMPLE_BUFFERS, &value);
            if(value != (format_.fsaa != 0) )
                continue;
        }
        if(format_.fsaa > 0 )
        {
            eglGetConfigAttrib(display_, configs[i], EGL_SAMPLES, &value);
            if(value != format_.fsaa)
                continue;
        }
        eglGetConfigAttrib(display_, configs[i], EGL_CONFIG_ID, &id);

        if(value == 0)
        {
            if(idxNoAlpha == -1)
            {
                idxNoAlpha = i;
            }
        }
        else
        {
            if( idxAlpha == -1)
            {
                idxAlpha = i;
            }
        }
        // if we found config with and without alpha skip other configs
        if (idxAlpha != -1 && idxNoAlpha != -1)
        {
            break;
        }
    }
    if (idxNoAlpha != -1)
    {
        config = configs[idxNoAlpha];
    }
    else if (idxAlpha != -1)
    {
        config = configs[idxAlpha];
    }
    else
    {
        NGLOG_ERROR("OVRGraphicsContext::chooseConfig failed to select matching config");
    }
    return config;
}

void OVRGraphicsContext::destroy()
{

#ifdef WITH_OVR_SDK
    NGLOG_DEBUG("Destroying context");

    vrapi_LeaveVrMode(ovrMobile_);
    java_.Env->DeleteGlobalRef( java_.ActivityObject );
    vrapi_Shutdown();

    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if(context_)
    {
      eglDestroyContext(display_, context_);
    }
    if(surface_)
    {
      eglDestroySurface(display_, surface_);
    }
    if(display_)
    {
       eglTerminate(display_);
    }

    config_ = 0;
    display_ = EGL_NO_DISPLAY;
    surface_ = EGL_NO_SURFACE;
    context_ = EGL_NO_CONTEXT;
#endif
}

bool OVRGraphicsContext::isValid()
{
    return context_ != EGL_NO_CONTEXT;
}

bool OVRGraphicsContext::makeCurrent()
{
    if (isValid())
        return eglMakeCurrent(display_, surface_, surface_, context_) != 0;
    else
        return false;
}

bool OVRGraphicsContext::detachThread()
{
    return eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != 0;
}

#include <fstream>
bool OVRGraphicsContext::swapBuffers()
{
#ifdef WITH_OVR_SDK
	return true;
#else
	//TODO is this correct??!?!?!
    /*if (ispbuffer_)
    {
#if 0
        std::vector<char> buf(4*surfaceWidth_*surfaceHeight_);
        glReadPixels(0, 0, surfaceWidth_, surfaceHeight_, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());
        std::ofstream f("/data/local/tmp/eglpb.rgba", std::ofstream::out|std::ofstream::binary);
        f.write(buf.data(), buf.size());
#endif
        glFlush();
        return true;
    }
    if(vulkan_)
    {
      return true;
    }*/
    return eglSwapBuffers(display_, surface_) != 0;
#endif
}


bool OVRGraphicsContext::setContextVersion(int32_t versionMajor, int32_t versionMinor)
{
    versionMajor_ = versionMajor;
    versionMinor_ = versionMinor;
    return true;
}

namespace {
    void logEglConfig(EGLDisplay display, EGLConfig config, bool printHeader)
    {
        int attribs[] =
        {
            EGL_CONFIG_ID,
            EGL_SURFACE_TYPE,
            EGL_NATIVE_RENDERABLE,
            EGL_BUFFER_SIZE,
            EGL_RED_SIZE,
            EGL_GREEN_SIZE,
            EGL_BLUE_SIZE,
            EGL_ALPHA_SIZE,
            EGL_DEPTH_SIZE,
            EGL_STENCIL_SIZE,
            EGL_SAMPLE_BUFFERS,
            EGL_SAMPLES,
            EGL_NONE,
        };
        static const char * header = "ID Surface Native Buffer  R  G  B  A  Depth Stencil SampleBuf Samples";
        static const char * format = "%2d %7d %6d %6d %2d %2d %2d %2d %6d %7d %9d %7d";

        if (printHeader)
        {
            NGLOG_INFO(header);
        }
        std::vector<int> values;
        const int *attrib = attribs;
        while (*attrib != EGL_NONE)
        {
            EGLint value = -1;
            if (eglGetConfigAttrib (display, config, *attrib, &value))
            {
                values.push_back(value);
            }
            else
            {
                values.push_back(-1);
            }
            ++attrib;
        }
        char line[4096];
        snprintf(line, sizeof(line), format, values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10], values[11]);
        NGLOG_INFO(line);
    }

    std::vector<EGLConfig> getEglConfigs(EGLDisplay display)
    {
        std::vector<EGLConfig> configs;
        int32_t numConfigs = 0;
        if (eglGetConfigs(display, 0, 0, &numConfigs))
        {
            configs.resize(numConfigs);
            eglGetConfigs(display, configs.data(), numConfigs, &numConfigs);
        }
        else
        {
            NGLOG_ERROR("eglGetConfigs failed");
        }
        return configs;
    }

    void logAllEglConfigs(EGLDisplay display)
    {
        std::vector<EGLConfig> configs = getEglConfigs(display);
        for (size_t i = 0; i < configs.size(); ++i)
        {
            logEglConfig(display, configs[i], i == 0);
        }
    }
}


GraphicsContext::GraphicsType OVRGraphicsContext::type()
{
    // TODO: add support for desktop GL
    return GraphicsContext::OVR_EGL;
}

bool OVRGraphicsContext::hasFlag(int)
{
    /* TODO: should this really always return false? */
    return false;
}

int OVRGraphicsContext::versionMajor()
{
    return isValid() ? versionMajor_ : -1;
}

int OVRGraphicsContext::versionMinor()
{
    return isValid() ? versionMinor_ : -1;
}

const char *OVRGraphicsContext::clientApis() const
{
    return eglQueryString(display_, EGL_CLIENT_APIS);
}

const char *OVRGraphicsContext::vendor() const
{
    return eglQueryString(display_, EGL_VENDOR);
}

const char *OVRGraphicsContext::extensions() const
{
    return eglQueryString(display_, EGL_EXTENSIONS);
}

const char *OVRGraphicsContext::version() const
{
    return eglQueryString(display_, EGL_VERSION);
}

void OVRGraphicsContext::setUseDefaultChooseConfig(bool useDefaultChooseConfig)
{
    useDefaultChooseConfig_ = useDefaultChooseConfig;
}


bool OVRGraphicsContext::updateVersionFromContext()
{
    std::string version = (const char*) glGetString(GL_VERSION);

    int major, minor;
    std::istringstream is(version);
    std::string ogl;
    std::string es;
    is >> ogl >> es >> major;
    is.get(); // read '.'
    is >> minor;
    require(is.good() || is.eof());
    bool ok = isGLESCompatible(major, minor);
    if (ok) {
        versionMajor_ = major;
        versionMinor_ = minor;
    }
    return ok;
}

bool OVRGraphicsContext::isGLESCompatible(int32_t major, int32_t minor) const
{
    if (versionMajor_ == 1 && major != 1) return false;
    if (versionMajor_ < major) return true;
    if (versionMajor_ > major) return false;
    if (versionMinor_ <= minor) return true;
    return false;
}

void OVRGraphicsContext::initContextAttribs()
{
    contextAttribs_.clear();
    std::string ext = eglQueryString(display_, EGL_EXTENSIONS);
    if ((eglVersionMajor_ == 1 && eglVersionMinor_ >= 5) || std::string::npos != ext.find("EGL_KHR_create_context"))
    {
        contextAttribs_.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttribs_.push_back(versionMajor_);
        contextAttribs_.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttribs_.push_back(versionMinor_);
    }
    else
    {
        contextAttribs_.push_back(EGL_CONTEXT_CLIENT_VERSION);
        contextAttribs_.push_back(versionMajor_);
    }
    contextAttribs_.push_back(EGL_NONE);
}
