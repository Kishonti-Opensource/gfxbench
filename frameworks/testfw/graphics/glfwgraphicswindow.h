/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLFWGRAPHICSWINDOW_H_
#define GLFWGRAPHICSWINDOW_H_

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "graphics/glformat.h"
#include "graphics/graphicswindow.h"

namespace tfw {
    class ApiDefinition;
}

class GLFWGraphicsWindow : public GraphicsWindow
{
public:
    GLFWGraphicsWindow();
    virtual ~GLFWGraphicsWindow();
    void create(int width, int height, const std::string &title, bool fullscreen, const std::vector<tfw::ApiDefinition>& api, const std::string &target_api, bool isHidden = false);
    void destroy();
    GLFWwindow* handle();
    virtual void pollEvents();
    virtual bool shouldClose();
    virtual void requestClose();
    virtual int width();
    virtual int height();
    void setFormat(const tfw::GLFormat &format);
    const tfw::GLFormat &format() const;
private:
    GLFWwindow *window_;
    tfw::GLFormat format_;
};

#endif  // GLFWGRAPHICSWINDOW_H_
