/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "schemas/apidefinition.h"
#include "graphics/windowfactory.h"
#include "graphics/eglgraphicscontext.h"
#include "graphics/graphicswindow.h"
#include "graphics/nullgraphicswindow.h"
#include "graphics/nullgraphicscontext.h"
#include "graphics/fbdevgraphicswindow.h"
#include "graphics/metalgraphicswindow.h"
#include "tfwmessagequeue.h"

#ifdef _WIN32
#include "graphics/hwndmessagequeue.h"
#include "graphics/hwndgraphicswindow.h"
#include "graphics/wglgraphicscontext.h"
#include "windowsvulkangraphicscontext.h"
#endif
#ifdef HAVE_D3D11
#include "graphics/dxwin32graphicscontext.h"
#endif
#ifdef HAVE_GLFW3
#include "graphics/glfwgraphicscontext.h"
#include "graphics/glfwgraphicswindow.h"
#include "graphics/glfwmessagequeue.h"
#endif
#ifdef HAVE_X11
#include "graphics/xgraphicswindow.h"
#endif

#ifdef DISPLAY_PROTOCOL_XCB
#include "graphics/xcbgraphicswindow.h"
#include "graphics/xcbgraphicscontext.h"
#endif

#ifdef DISPLAY_PROTOCOL_WAYLAND
#include "graphics/wlgraphicswindow.h"
#include "graphics/wlgraphicscontext.h"
#endif

#ifdef DISPLAY_PROTOCOL_SCREEN
#include "graphics/qnxgraphicswindow.h"
#include "graphics/qnxgraphicscontext.h"
#endif /* DISPLAY_PROTOCOL_SCREEN */

#if defined ANDROID

#if ANDROID_ENABLE_ANW
#include "graphics/anwgraphicswindow.h"
#endif

// needed for android_createDisplaySurface
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
typedef EGLNativeWindowType (*ANDROID_CREATEDISPLAYSURFACE_FUNC)();
#endif
#include "ng/require.h"

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#endif

#ifdef HAVE_EPOXY
  #include <epoxy/egl.h>
  #include <epoxy/gl.h>
  #ifdef _WIN32
    #include <epoxy/wgl.h>
  #elif defined HAVE_X11
    #include <epoxy/glx.h>
  #endif
#else
  #ifdef _WIN32
    #include <Windows.h>
    #include <gl/GL.h>
    #pragma comment(lib, "opengl32.lib")
  #else
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
  #endif
#endif

// fix compile error on linux where major/minor might be defined. conflicts with ApiDefinitions c++ API
#undef major
#undef minor


void setEGLContextVersionIfFoundInApidef(const std::vector<tfw::ApiDefinition> &apis, EGLGraphicsContext *egl)
{
    for (size_t i = 0; i < apis.size(); i++)
    {
        const tfw::ApiDefinition &api = apis[i];
        if (api.type() == tfw::ApiDefinition::ES)
        {
            egl->setContextVersion(api.major(), api.minor());
            break;
        }
    }
}

WindowFactory::WindowFactory()
    : width_(0)
    , height_(0)
    , fullscreen_(0)
#if defined(__QNX__)
    , pixelFormat_("auto")
    , nbuffers_(4)
    , interval_(0)
#endif
{}

WindowFactory::Wnd *WindowFactory::createGLFW(const std::vector<tfw::ApiDefinition>& api, const std::string &target_api)
{
#ifdef HAVE_GLFW3
    ng::scoped_ptr<GLFWGraphicsWindow> wnd(new GLFWGraphicsWindow());
    wnd->create(width_, height_, "GLFW", fullscreen_, api, target_api);
    wnd->setFormat(format_);
    ng::scoped_ptr<GLFWGraphicsContext> glfw(new GLFWGraphicsContext(wnd->handle(), false));
    ng::scoped_ptr<tfw::GLFWMessageQueue> msg(new tfw::GLFWMessageQueue(true));
    msg->attach(wnd->handle());
    requireex(glfw->makeCurrent());
    return new Wnd(wnd.release(), msg.release(), glfw.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createEGLPB(const std::vector<tfw::ApiDefinition>& api_defs)
{
    requireex(width_ > 0 && height_ > 0, FORMATCSTR("eglpb: width and height must be greater than 0"));
    ng::scoped_ptr<NullGraphicsWindow> wnd(new NullGraphicsWindow(width_, height_, false));
    ng::scoped_ptr<EGLGraphicsContext> egl(new EGLGraphicsContext());
    egl->setUseDefaultChooseConfig(true);
    setEGLContextVersionIfFoundInApidef(api_defs, egl.get());
    egl->setFormat(format_);
    egl->initPBufferSurface(width_, height_);
    return new Wnd(wnd.release(), 0, egl.release());
}

WindowFactory::Wnd *WindowFactory::createNULL()
{
    ng::scoped_ptr<NullGraphicsWindow> wnd(new NullGraphicsWindow(width_, height_, false));
    ng::scoped_ptr<NullGraphicsContext> null(new NullGraphicsContext());
    return new Wnd(wnd.release(), 0, null.release());
}

WindowFactory::Wnd *WindowFactory::createEGLCDS(const std::vector<tfw::ApiDefinition>& api_defs)
{
#if defined ANDROID
    EGLNativeWindowType nativeHandle = 0;
    std::ostringstream os;
    ng::scoped_ptr<NullGraphicsWindow> wnd(new NullGraphicsWindow(width_, height_, false));
    void *module = dlopen("/system/lib/libui.so", RTLD_GLOBAL);
    if (!module) {
        os << "dlopen failed: " << strerror(errno);
        throw std::runtime_error(os.str());
    }
    ANDROID_CREATEDISPLAYSURFACE_FUNC android_createDisplaySurface =
        (ANDROID_CREATEDISPLAYSURFACE_FUNC) dlsym(RTLD_DEFAULT, "android_createDisplaySurface");
    fprintf(stderr, "android_createDisplaySurface: %p\n", (void*)android_createDisplaySurface);
    if (0 == android_createDisplaySurface) {
        os << "dlsym failed: " << strerror(errno);
        throw std::runtime_error(os.str());
    }
    nativeHandle = android_createDisplaySurface();
    fprintf(stderr, "EGLNativeWindowType: nativeHandle: %p\n", (void*)nativeHandle);
    dlclose(module);
    ng::scoped_ptr<EGLGraphicsContext> egl(new EGLGraphicsContext());

    setEGLContextVersionIfFoundInApidef(api_defs, egl.get());
    egl->setFormat(format_);
    requireex(egl->initWindowSurface(nativeHandle));
    return new Wnd(wnd.release(), 0, egl.release());
#else
    (void)api_defs;
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createEGL(const std::vector<tfw::ApiDefinition>& api_defs)
{
#ifdef __APPLE__
    return 0;
#else
#ifdef _WIN32
    ng::scoped_ptr<HWNDGraphicsWindow> wnd(new HWNDGraphicsWindow());
    wnd->create(width_, height_, "HWND + EGL", fullscreen_);
#elif defined ANDROID && ANDROID_ENABLE_ANW
    ng::scoped_ptr<ANWGraphicsWindow> wnd(new ANWGraphicsWindow());
    wnd->create(width_, height_, "ANW + EGL", 3);//3==RGB888
#elif defined DISPLAY_PROTOCOL_WAYLAND
    ng::scoped_ptr<WLGraphicsWindow> wnd(new WLGraphicsWindow(true));
    requireex(wnd->create(width_, height_, "Wayland + EGL"));
#elif defined HAVE_X11
    ng::scoped_ptr<XGraphicsWindow> wnd(new XGraphicsWindow());
    requireex(wnd->create(width_, height_, "X11 + EGL"));
#elif defined __QNX__
    ng::scoped_ptr<QNXGraphicsWindow> wnd(new QNXGraphicsWindow());
    wnd->setInterval(interval_);
    wnd->setNumBuffers(nbuffers_);
    wnd->create(width_, height_, "QNX Screen + EGL", fullscreen_, pixelFormat_, desiredDisplay_);
    /* We can adjust width and height, update them */
    width_ = wnd->width();
    height_ = wnd->height();
    pixelFormat_ = wnd->pixelFormatStr();
    if (pixelFormat_.compare("rgb565") == 0) {
        format_.visual_id = SCREEN_FORMAT_RGB565;
        format_.red = 5;
        format_.green = 6;
        format_.blue = 5;
        format_.alpha = 0;
        format_.isExact = true;
    }
    if (pixelFormat_.compare("rgba8888") == 0) {
        format_.visual_id = SCREEN_FORMAT_RGBA8888;
        format_.red = 8;
        format_.green = 8;
        format_.blue = 8;
        format_.alpha = 8;
        format_.isExact = true;
    }
#if defined(SCREEN_FORMAT_BGRA8888)
    if (pixelFormat_.compare("bgra8888") == 0) {
        format_.visual_id = SCREEN_FORMAT_BGRA8888;
        format_.red = 8;
        format_.green = 8;
        format_.blue = 8;
        format_.alpha = 8;
        format_.isExact = true;
    }
#endif /* SCREEN_FORMAT_BGRA8888 */
#if defined(SCREEN_FORMAT_RGBA1010102)
    if (pixelFormat_.compare("rgba1010102") == 0) {
        format_.visual_id = SCREEN_FORMAT_RGBA1010102;
        format_.red = 10;
        format_.green = 10;
        format_.blue = 10;
        format_.alpha = 2;
        format_.isExact = true;
    }
#endif /* SCREEN_FORMAT_RGBA1010102 */
#if defined(SCREEN_FORMAT_BGRA1010102)
    if (pixelFormat_.compare("bgra1010102") == 0) {
        format_.visual_id = SCREEN_FORMAT_BGRA1010102;
        format_.red = 10;
        format_.green = 10;
        format_.blue = 10;
        format_.alpha = 2;
        format_.isExact = true;
    }
#endif /* SCREEN_FORMAT_BGRA1010102 */

#else
    NGLOG_WARN("EGLNativeWindowType could not be detected at build time. Fallback to NULL as EGLNativeWindowType. See also: EGLPB\n"
        "No one defined: _WIN32, ANDROID, HAVE_X11,USE_WAYLAND, __QNX__, __APPLE__ \n");
    ng::scoped_ptr<NullGraphicsWindow> wnd(new NullGraphicsWindow(width_, height_, false));
#endif
    // create a EGLGraphicsContext with the native window handle
    ng::scoped_ptr<EGLGraphicsContext> egl(new EGLGraphicsContext());
    setEGLContextVersionIfFoundInApidef(api_defs, egl.get());
    egl->setFormat(format_);
#ifdef DISPLAY_PROTOCOL_WAYLAND
    NGLOG_INFO("initalize wayland surface %s %s", wnd->display(), wnd->handle() );
    requireex(egl->initWaylandWindowSurface((EGLNativeDisplayType)wnd->display(), (EGLNativeWindowType)wnd->handle()));
#else
    requireex(egl->initWindowSurface((EGLNativeWindowType)wnd->handle()));
#endif
    return new Wnd(wnd.release(), 0, egl.release());
#endif//__APPLE__
}

WindowFactory::Wnd *WindowFactory::createEGLFBDEV(const std::vector<tfw::ApiDefinition>& api_defs)
{
    ng::scoped_ptr<FBDEVGraphicsWindow> wnd(new FBDEVGraphicsWindow(width_, height_));
    ng::scoped_ptr<EGLGraphicsContext> egl(new EGLGraphicsContext());
    setEGLContextVersionIfFoundInApidef(api_defs, egl.get());
    egl->setFormat(format_);
    requireex(egl->initWindowSurface((EGLNativeWindowType)wnd->handle()));
    return new Wnd(wnd.release(), 0, egl.release());
}

WindowFactory::Wnd *WindowFactory::createDX()
{
#ifdef HAVE_D3D11
    ng::scoped_ptr<HWNDGraphicsWindow> wnd(new HWNDGraphicsWindow());
    wnd->create(width_, height_, "HWND + DX", fullscreen_);
    ng::scoped_ptr<DxWin32GraphicsContext> dx(new DxWin32GraphicsContext(wnd->handle()));
	dx->setFormat(format_.red, format_.green, format_.blue, format_.alpha, format_.depth, format_.stencil, format_.fsaa);
	dx->init("", vsync_);
	ng::scoped_ptr<HWNDMessageQueue> queue(new HWNDMessageQueue());
	wnd->setMessageQueue(queue.get());
    return new Wnd(wnd.release(), queue.release(), dx.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createWGL()
{
#ifdef _WIN32
    ng::scoped_ptr<HWNDGraphicsWindow> wnd(new HWNDGraphicsWindow());
    wnd->create(width_, height_, "HWND + WGL", false);
    ng::scoped_ptr<WGLGraphicsContext> wgl(new WGLGraphicsContext());
    wgl->create(wnd->handle(), 32);
    return new Wnd(wnd.release(), 0, wgl.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createMTL(const std::vector<tfw::ApiDefinition>& api)
{
#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_11)
    ng::scoped_ptr<MTLGraphicsWindow> wnd(CreateMTLGraphicsWindow(width_, height_, api));
    requireex(wnd->create() );
    ng::scoped_ptr<GraphicsContext> mtlgc( wnd->getGraphicsContext() );
    ng::scoped_ptr<tfw::MessageQueue> msg( wnd->getMessageQueue() );
    return new Wnd(wnd.release(),msg.release(),mtlgc.release()) ;
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createHWND(const std::vector<tfw::ApiDefinition>& api_defs, const std::string &gfx)
{
#ifdef _WIN32
    ng::scoped_ptr<HWNDGraphicsWindow> wnd(new HWNDGraphicsWindow());

    GraphicsContext* gfx_ctx = nullptr;
    if (gfx == "vulkan")
    {
		wnd->create(width_, height_, "HWND + null", fullscreen_);
		gfx_ctx = new WindowsVulkanGraphicsContext(wnd->handle());
    }
    else if(gfx == "dx12")
    {
//TODO check the size of api_defs
#ifdef HAVE_D3D11//<--workaround for testfw
		wnd->create(width_, height_, "DX12", fullscreen_);
		gfx_ctx = new DxBareGraphicsContext(wnd->handle(), api_defs[0].major(), api_defs[0].minor(), GraphicsContext::DIRECTX12);
#else
		return nullptr;
#endif
    }
    else if(gfx == "dx11")
    {
#ifdef HAVE_D3D11
		wnd->create(width_, height_, "DX11", fullscreen_);
		gfx_ctx = new DxBareGraphicsContext(wnd->handle(), api_defs[0].major(), api_defs[0].minor());
#else
		return nullptr;
#endif
    }
    else
    {
        return nullptr;
    }

    ng::scoped_ptr<HWNDMessageQueue> queue(new HWNDMessageQueue());
    wnd->setMessageQueue(queue.get());
    return new Wnd(wnd.release(), queue.release(), gfx_ctx);
#elif defined __linux__
#ifdef DISPLAY_PROTOCOL_WAYLAND
    ng::scoped_ptr<WLGraphicsWindow> wnd(new WLGraphicsWindow(false));
    requireex(wnd->create(width_, height_, "Wayland"));
    ng::scoped_ptr<WLGraphicsContext> wl_ctx(new WLGraphicsContext());
    wl_ctx->init(wnd->display(), wnd->surface() );
    return new Wnd(wnd.release(), 0, wl_ctx.release());
#elif defined DISPLAY_PROTOCOL_XCB
    ng::scoped_ptr<XCBGraphicsWindow> wnd(new XCBGraphicsWindow());
    wnd->create(width_, height_, "XCB");

    ng::scoped_ptr<XCBGraphicsContext> xcb_ctx(new XCBGraphicsContext());
    xcb_ctx->init(wnd->connection(), wnd->window(), wnd->screen() );

    return new Wnd(wnd.release(), 0, xcb_ctx.release());
#endif
#elif defined ANDROID && ANDROID_ENABLE_ANW
    ng::scoped_ptr<ANWGraphicsWindow> wnd(new ANWGraphicsWindow());
    wnd->create(width_, height_, "ANW + EGL", 5);//5==BGRA888

    // create a EGLGraphicsContext with the native window handle
    ng::scoped_ptr<EGLGraphicsContext> egl(new EGLGraphicsContext());
    setEGLContextVersionIfFoundInApidef(api_defs, egl.get());
    egl->setFormat(format_);
    requireex(egl->initVulkanWindowSurface((EGLNativeWindowType)wnd->handle()));
    return new Wnd(wnd.release(), 0, egl.release());
#endif
    return NULL;
}

void WindowFactory::setApi(const std::vector<tfw::ApiDefinition>& api_defs, const std::string &target_api)
{
    api_defs_ = api_defs;
    target_api_ = target_api;
}

void WindowFactory::setSize(int width, int height)
{
    width_ = width;
    height_ = height;
}

void WindowFactory::setFullscreen(bool fullscreen)
{
    fullscreen_ = fullscreen;
}

void WindowFactory::setVsync( bool vsync )
{
	vsync_ = vsync;
}

void WindowFactory::setFormat(const tfw::GLFormat& format)
{
    format_ = format;
}

#if defined(__QNX__)
void WindowFactory::setPixelFormat(const std::string& format)
{
    pixelFormat_ = format;
}
void WindowFactory::setNumBuffers(int nbuffers)
{
    nbuffers_ = nbuffers;
}
void WindowFactory::setInterval(int interval)
{
    interval_ = interval;
}
void WindowFactory::setDesiredDisplay(const std::string &desiredDisplay)
{
    desiredDisplay_ = desiredDisplay;
}
tfw::GLFormat WindowFactory::getFormat()
{
    return format_;
}
#endif

WindowFactory::Wnd *WindowFactory::createWaylandVulkan(const std::vector<tfw::ApiDefinition>& api_defs)
{
#ifdef DISPLAY_PROTOCOL_WAYLAND
   ng::scoped_ptr<WLGraphicsWindow> wnd(new WLGraphicsWindow(false));
    ng::scoped_ptr<WLGraphicsContext> wl_ctx(new WLGraphicsContext());
    requireex(wnd->create(width_, height_, "Wayland + EGL"));
    wl_ctx->init(wnd->display(), wnd->surface() );
    return new Wnd(wnd.release(), 0, wl_ctx.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createWaylandXCB(const std::vector<tfw::ApiDefinition>& api_defs)
{
#ifdef DISPLAY_PROTOCOL_XCB
    ng::scoped_ptr<XCBGraphicsWindow> wnd(new XCBGraphicsWindow());
    wnd->create(width_, height_, "XCB");

    ng::scoped_ptr<XCBGraphicsContext> xcb_ctx(new XCBGraphicsContext());
    xcb_ctx->init(wnd->connection(), wnd->window(), wnd->screen() );

    return new Wnd(wnd.release(), 0, xcb_ctx.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::createQNXScreenVulkan(const std::vector<tfw::ApiDefinition>& api_defs)
{
#ifdef DISPLAY_PROTOCOL_SCREEN
    ng::scoped_ptr<QNXGraphicsWindow> wnd(new QNXGraphicsWindow());
    ng::scoped_ptr<QNXGraphicsContext> qnx_ctx(new QNXGraphicsContext());
    wnd->create(width_, height_, "QNX Vulkan", fullscreen_, pixelFormat_, desiredDisplay_);
    width_ = wnd->width();
    height_ = wnd->height();
    pixelFormat_ = wnd->pixelFormatStr();
    qnx_ctx->init(wnd->context(), wnd->window(), pixelFormat_.c_str(), nbuffers_);
    return new Wnd(wnd.release(), nullptr, qnx_ctx.release());
#else
    return 0;
#endif
}

WindowFactory::Wnd *WindowFactory::create(const std::string &gfx)
{
    Wnd* window = nullptr;

    if (gfx == "glfw") {
        window = createGLFW(api_defs_, target_api_);
    } else if (gfx == "wayland_vulkan") {
	    window = createWaylandVulkan(api_defs_);
    } else if (gfx == "xcb_vulkan") {
	    window = createWaylandXCB(api_defs_);
    } else if (gfx == "screen_vulkan") {
        window = createQNXScreenVulkan(api_defs_);
    } else if (gfx == "egl") {
        window = createEGL(api_defs_);
    } else if (gfx == "eglfbdev") {
        window = createEGLFBDEV(api_defs_);
    } else if (gfx == "eglpb") {
        window = createEGLPB(api_defs_);
    } else if (gfx == "eglcds") {
        window = createEGLCDS(api_defs_);
    } else if (gfx == "wgl") {
        window = createWGL();
    } else if (gfx == "vulkan" || gfx == "dx12") {
        window = createHWND(api_defs_, gfx);
    } else if (gfx == "dx" || gfx == "dx11") {
        window = createDX();
    } else if (gfx == "null") {
        window = createNULL();
    } else if (gfx == "mtl"){
        window = createMTL(api_defs_);
    } else {
#ifdef WIN32
        NGLOG_FATAL("Unknown gfx type: %s ! Valid types: glfw, egl, eglpb, wgl, dx, vulkan, dx12", gfx);
#elif defined __linux__
        NGLOG_FATAL("Unknown gfx type: %s ! Valid types: glfw, egl, eglpb, eglfbdev, wayland_vulkan, xcb_vulkan", gfx);
#elif defined __QNX__
        NGLOG_FATAL("Unknown gfx type: %s ! Valid types: egl, screen_vulkan, wayland_vulkan", gfx);
#else
        NGLOG_FATAL("Unknown gfx type: %s ! Valid types: glfw, egl, eglpb, eglfbdev, eglcds, wgl, dx", gfx);
#endif
    }
    if(!window) {
        NGLOG_FATAL("Selected gfx type not supported: %s !", gfx);
    }
    return window;
}
