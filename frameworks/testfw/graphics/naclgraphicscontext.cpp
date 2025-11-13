/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "naclgraphicscontext.h"
#include <ppapi/cpp/instance.h>
#include <ppapi/gles2/gl2ext_ppapi.h>
#include <stdio.h>

NaClGraphicsContext::NaClGraphicsContext(pp::Instance* instance, int width, int height)
{
	const int32_t attrib_list[] = {
		PP_GRAPHICS3DATTRIB_RED_SIZE, 8,
		PP_GRAPHICS3DATTRIB_GREEN_SIZE, 8,
		PP_GRAPHICS3DATTRIB_BLUE_SIZE, 8,
		PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 0,
		PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 16,
		PP_GRAPHICS3DATTRIB_WIDTH, width,
		PP_GRAPHICS3DATTRIB_HEIGHT, height,
		PP_GRAPHICS3DATTRIB_NONE
	};

	cbFactory.Initialize(this);
	graphics=pp::Graphics3D(instance, attrib_list);
	if (isValid())
	{
		if (!instance->BindGraphics(graphics)) graphics=pp::Graphics3D();
	}
	surfaceWidth_=width;
	surfaceHeight_=height;
	pthread_mutex_init(&swapBuffersInProgressMutex,NULL);
	pthread_cond_init(&swapBuffersFinishedEvent,NULL);
	swapBuffersInProgress = false;
}

NaClGraphicsContext::~NaClGraphicsContext()
{
	pthread_cond_destroy(&swapBuffersFinishedEvent);
	pthread_mutex_destroy(&swapBuffersInProgressMutex);
	detachThread();
}

const pp::Graphics3D& NaClGraphicsContext::getContext()
{
	return graphics;
}

bool NaClGraphicsContext::makeCurrent()
{
	if (!isValid()) return false;
	glSetCurrentContextPPAPI(graphics.pp_resource());
	return true;
}

bool NaClGraphicsContext::swapBuffers()
{
	if (!isValid()) return false;
	pthread_mutex_lock(&swapBuffersInProgressMutex);
	detachThread();
	pp::Module::Get()->core()->CallOnMainThread(0,cbFactory.NewCallback(&NaClGraphicsContext::actualSwapBuffers),0);
	pthread_cond_wait(&swapBuffersFinishedEvent,&swapBuffersInProgressMutex);
	makeCurrent();
	pthread_mutex_unlock(&swapBuffersInProgressMutex);
	return true;
}

void NaClGraphicsContext::actualSwapBuffers(int32_t result)
{
	graphics.SwapBuffers(cbFactory.NewCallback(&NaClGraphicsContext::swapBuffersFinished));
}

void NaClGraphicsContext::swapBuffersFinished(int32_t result)
{
	pthread_mutex_lock(&swapBuffersInProgressMutex);
	makeCurrent();
	pthread_cond_signal(&swapBuffersFinishedEvent);
	detachThread();
	pthread_mutex_unlock(&swapBuffersInProgressMutex);
}

bool NaClGraphicsContext::ResizeBuffers(int width, int height)
{
	if (!isValid()) return false;
	if (graphics.ResizeBuffers(width,height)) {
		surfaceWidth_=width;
		surfaceHeight_=height;
		return true;
	}
	return false;
}

bool NaClGraphicsContext::isValid()
{
	return !graphics.is_null();
}

bool NaClGraphicsContext::detachThread()
{
	glSetCurrentContextPPAPI(0);
}
