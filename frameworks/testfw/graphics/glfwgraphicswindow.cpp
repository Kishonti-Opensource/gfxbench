/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/glfwgraphicswindow.h"
#include "schemas/apidefinition.h"
#include "ng/log.h"
#include "ng/require.h"
#if defined __APPLE__
#include <TargetConditionals.h>
#endif

namespace {

void glfwerrorfunc(int errcode, const char *msg)
{
	NGLOG_ERROR("GLFW error: %s: %s\n", errcode, msg);
}


struct GLFWInitializer
{
    GLFWInitializer()
    {
        glfwInit();
		glfwSetErrorCallback(glfwerrorfunc);
        NGLOG_INFO("GLFW init");
    }
    ~GLFWInitializer()
    {

#ifndef __APPLE__
        glfwTerminate();
#endif
    }
};

}

GLFWGraphicsWindow::GLFWGraphicsWindow()
    : window_(0)
{
}

GLFWGraphicsWindow::~GLFWGraphicsWindow()
{
    destroy();
}

void GLFWGraphicsWindow::create(
        int width,
        int height,
        const std::string &title,
        bool fullscreen,
        const std::vector<tfw::ApiDefinition>& api,
        const std::string &target_api,
        bool isHidden)
{
    static GLFWInitializer GLFW_INIT;
	GLFWmonitor *monitor = NULL;
	if (width <= 0 || height <= 0) {
		fullscreen = true;
		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		width = mode->width;
		height = mode->height;
	}
	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}

	int major_version = 0;
	int minor_version = 0;

	bool desktop_gl = target_api.find("desktop") != std::string::npos;
	bool core_profile = target_api == "desktop_core";
	for (size_t i = 0; i<api.size(); i++)
	{
		if (api[i].type() == tfw::ApiDefinition::GL && desktop_gl)
		{
			major_version = api[i].major();
			minor_version = api[i].minor();
		}
		else if (api[i].type() == tfw::ApiDefinition::ES && !desktop_gl)
		{
			major_version = api[i].major();
			minor_version = api[i].minor();
		}
	}
#if defined __APPLE__

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
	if (desktop_gl && (major_version<4))
	{
		major_version = 2;
		minor_version = 1;
	}
#endif
#endif
	glfwDefaultWindowHints();

//NOTE: see platform.h on how statistics works
//NOTE: uncomment this when measuring statistics (even runtime stats)
    //major_version = 4;
    //minor_version = 5;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);

	if (format_.red >= 0) glfwWindowHint(GLFW_RED_BITS, format_.red);
	if (format_.green >= 0) glfwWindowHint(GLFW_GREEN_BITS, format_.green);
	if (format_.blue >= 0) glfwWindowHint(GLFW_BLUE_BITS, format_.blue);
	if (format_.alpha >= 0) glfwWindowHint(GLFW_ALPHA_BITS, format_.alpha);
	if (format_.depth >= 0) glfwWindowHint(GLFW_DEPTH_BITS, format_.depth);
	if (format_.stencil >= 0) glfwWindowHint(GLFW_STENCIL_BITS, format_.stencil);
	if (format_.fsaa >= 0) glfwWindowHint(GLFW_SAMPLES, format_.fsaa);

	// Create debug context in debug profile. _DEBUG macro is used on Windows
#if defined DEBUG || defined _DEBUG   
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    if (isHidden) {
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    }

	if (desktop_gl)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		if (major_version > 3 || (major_version == 3 && minor_version > 1))
		{
			if (core_profile)
			{
				// Create core profile
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				// Also remove all functionality marked deprecated
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			}
			else
			{
				// Create compatibility profile
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
			}
		}
	}
	else
	{
		// Use ES API
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	}
    glfwWindowHint(GLFW_RESIZABLE, false);
    window_ = glfwCreateWindow(width, height, title.c_str(), monitor, NULL);
	requireex(window_ != NULL);
}

void GLFWGraphicsWindow::destroy()
{
    if (window_ != 0) {
        glfwDestroyWindow(window_);
    }
    window_ = 0;
}

GLFWwindow* GLFWGraphicsWindow::handle()
{
    return window_;
}

void GLFWGraphicsWindow::pollEvents()
{
    glfwPollEvents();
}

bool GLFWGraphicsWindow::shouldClose()
{
    return glfwWindowShouldClose(window_) != 0;
}

void GLFWGraphicsWindow::requestClose()
{
    glfwSetWindowShouldClose(window_, 1);
}

int GLFWGraphicsWindow::width()
{
    int w, _;
    glfwGetFramebufferSize(window_, &w, &_);
    return w;
}

int GLFWGraphicsWindow::height()
{
    int _, h;
    glfwGetFramebufferSize(window_, &_, &h);
    return h;
}

void GLFWGraphicsWindow::setFormat(const tfw::GLFormat &format)
{
    format_ = format;
}

const tfw::GLFormat& GLFWGraphicsWindow::format() const
{
    return format_;
}
