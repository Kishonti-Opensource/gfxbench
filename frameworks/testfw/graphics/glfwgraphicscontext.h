/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLFWGRAPHICSCONTEXT_H_
#define GLFWGRAPHICSCONTEXT_H_

#include "graphics/graphicscontext.h"
#include <GLFW/glfw3.h>

class GLFWGraphicsContext : public GraphicsContext
{
public:
	GLFWGraphicsContext(GLFWwindow *window, bool owner);
	virtual ~GLFWGraphicsContext();
	virtual bool isValid();
	virtual bool makeCurrent();
	virtual bool detachThread();
	virtual bool swapBuffers();
	virtual GraphicsType type();
	virtual int versionMajor();
	virtual int versionMinor();
	virtual bool hasFlag(int flag);

    GLFWwindow* getWindow();
    
private:
	GLFWwindow *window_;
	GraphicsType type_;
	int majorVersion_;
	int minorVersion_;
	int flags_;
	bool owner_;
};

#endif // GLFWGRAPHICSCONTEXT_H_
