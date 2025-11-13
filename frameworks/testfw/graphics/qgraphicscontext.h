/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef QGRAPHICSCONTEXT_H_
#define QGRAPHICSCONTEXT_H_

#include "graphics/graphicscontext.h"

class QGLWidget;
class QGraphicsContext : public GraphicsContext
{
public:
	QGraphicsContext(QGLWidget *widget);
	virtual ~QGraphicsContext();
	virtual bool isValid();
	virtual bool makeCurrent();
	virtual bool detachThread();
	virtual bool swapBuffers();
	virtual GraphicsType type();
	virtual int versionMajor();
	virtual int versionMinor();
	virtual bool hasFlag(int flag);
	
	QGLWidget *widget() { return widget_; }

private:
	QGLWidget *widget_;
};

#endif // GLFWGRAPHICSCONTEXT_H_
