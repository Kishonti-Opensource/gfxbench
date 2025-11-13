/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/glfwgraphicscontext.h"
#include "ng/require.h"


GLFWGraphicsContext::GLFWGraphicsContext(GLFWwindow *window, bool owner)
	: window_(window)
	, owner_(owner)
{
	require(window);

	type_ = GraphicsContext::OPENGL;
	if (glfwGetWindowAttrib(window, GLFW_CLIENT_API) == GLFW_OPENGL_ES_API)
	{
		type_ = GraphicsContext::GLES;
	}
	
	// Query the OpenGL version
	majorVersion_ = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minorVersion_ = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);

	flags_ = 0;
	if (type_ == GraphicsContext::OPENGL)
	{
		// Core or compatibility profile is only valid for desktop GL
		if (glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE) == GLFW_OPENGL_CORE_PROFILE)
		{
			flags_ |= FLAG_GL_CORE_PROFILE;
			if (glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT) == GL_TRUE)
			{
				flags_ |= FLAG_GL_FORWARD_COMPATIBLE;
			}
		}
		else
		{
			// Handle OpenGL versions below 3.3 as compatibility profiles
			flags_ |= FLAG_GL_COMPATIBILITY_PROFILE;
		}
	}

	// Note for GLFW issue: https://github.com/glfw/glfw/issues/46
	if (glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT) == GL_TRUE)
	{
		flags_ |= FLAG_GL_DEBUG_CONTEXT;
	}
	if (glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS) != GLFW_NO_ROBUSTNESS)
	{
		flags_ |= FLAG_GL_ROBUSTNESS;
	}
}

GLFWGraphicsContext::~GLFWGraphicsContext()
{
	if (window_ && owner_)
	{
		glfwDestroyWindow(window_);
	}
	window_ = 0;
}

bool GLFWGraphicsContext::isValid()
{
	return window_ != 0;
}


bool GLFWGraphicsContext::makeCurrent()
{
	glfwMakeContextCurrent(window_);
	return glfwGetCurrentContext() == window_;
}


bool GLFWGraphicsContext::detachThread()
{
	glfwMakeContextCurrent(NULL);
	return true;
}


bool GLFWGraphicsContext::swapBuffers()
{
	glfwSwapBuffers(window_);
	return true;
}

GraphicsContext::GraphicsType GLFWGraphicsContext::type()
{
	return type_;
}

int GLFWGraphicsContext::versionMajor()
{
	return majorVersion_;
}

int GLFWGraphicsContext::versionMinor()
{
	return minorVersion_;
}

bool GLFWGraphicsContext::hasFlag(int flag)
{
	return (flags_ & flag) != 0;
}

GLFWwindow* GLFWGraphicsContext::getWindow()
{
    return window_;
}

