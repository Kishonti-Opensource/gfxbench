/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics/graphicscontext.h"
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/utility/completion_callback_factory.h>
#include <pthread.h>

class NaClGraphicsContext : public GraphicsContext
{
public:
	NaClGraphicsContext(pp::Instance* instance, int width, int height);
	virtual ~NaClGraphicsContext();
	virtual bool makeCurrent();
	virtual bool swapBuffers();
	virtual bool isValid();
	virtual bool detachThread();
	const pp::Graphics3D& getContext();
	bool ResizeBuffers(int width, int height);
	int surfaceWidth() const { return surfaceWidth_; }
	int surfaceHeight() const { return surfaceHeight_; }
	virtual GraphicsType type() { return GLES; }
	virtual int versionMajor() { return 2; }
	virtual int versionMinor() { return 0; }
	virtual bool hasFlag(int flag) { return false; };
private:
	void actualSwapBuffers(int32_t result);
	void swapBuffersFinished(int32_t result);
	pp::CompletionCallbackFactory<NaClGraphicsContext> cbFactory;
	pp::Graphics3D graphics;
	int surfaceWidth_;
	int surfaceHeight_;
	pthread_mutex_t swapBuffersInProgressMutex;
	pthread_cond_t swapBuffersFinishedEvent;
	bool swapBuffersInProgress;
};