/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <QGLContext>
#include <qapplication.h>
#include "graphics/qgraphicscontext.h"
#include "ng/require.h"



QGraphicsContext::QGraphicsContext(QGLWidget *widget)
: widget_(widget)
{
	require(widget);
}

QGraphicsContext::~QGraphicsContext()
{
	widget_ = 0;
}

bool QGraphicsContext::isValid()
{
	return widget_ != 0 && widget_->isValid();
}


bool QGraphicsContext::makeCurrent()
{
	widget_->makeCurrent();
	return true;
}


bool QGraphicsContext::detachThread()
{
	widget_->doneCurrent();
	widget_->context()->moveToThread(QApplication::instance()->thread());
	return true;
}


bool QGraphicsContext::swapBuffers()
{
	widget_->swapBuffers();
	// makeCurrent squelches Qt warnings, see: http://doc.qt.io/qt-5/qglcontext.html#swapBuffers
	widget_->makeCurrent(); 
	return true;
}

GraphicsContext::GraphicsType QGraphicsContext::type()
{
	if (!isValid()) return GraphicsContext::NONE;
	return GraphicsContext::OPENGL;
}

int QGraphicsContext::versionMajor()
{
	return isValid() ? widget_->format().majorVersion() : -1;
}

int QGraphicsContext::versionMinor()
{
	return isValid() ? widget_->format().minorVersion() : -1;
}

bool QGraphicsContext::hasFlag(int flag)
{
	if (!isValid()) return false;

	QGLFormat fmt = widget_->format();
	if (flag == FLAG_GL_COMPATIBILITY_PROFILE)
	{
		return fmt.profile() == QGLFormat::CompatibilityProfile;
	}
	if (flag == FLAG_GL_CORE_PROFILE)
	{
		return fmt.profile() == QGLFormat::CoreProfile;
	}
	return false;
}
