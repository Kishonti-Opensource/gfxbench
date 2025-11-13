/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ogl_gles31_functions.h"

#if !defined(WIN32) && !defined(IPHONE)
#include <dlfcn.h>
#endif

#ifdef __QNX__
// Included so that NULL is defined
#include <cstddef>
#endif

#ifndef GL_EXT_discard_framebuffer

#ifndef GL_COLOR_EXT
#define GL_COLOR_EXT 0x1800
#endif

#ifndef GL_DEPTH_EXT
#define GL_DEPTH_EXT 0x1801
#endif

#ifndef GL_STENCIL_EXT
#define GL_STENCIL_EXT 0x1802
#endif

#endif

PFNGLDISPATCHCOMPUTEPROC glDispatchComputeProc = 0 ;
PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirectProc = 0;
PFNGLBINDIMAGETEXTUREPROC glBindImageTextureProc = 0 ;
PFNGLMEMORYBARRIERPROC glMemoryBarrierProc = 0 ;
PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirectProc = 0;
PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirectProc = 0;
PFNGLBLENDFUNCIPROC glBlendFunciProc = 0 ;
PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceivProc = 0;
PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceivProc = 0;
PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceNameProc = 0;
PFNGLPATCHPARAMETERIPROC glPatchParameteriProc = 0;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64vProc = 0;

// KHR_debug
PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControlProc = 0;
PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsertProc = 0;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallbackProc = 0;
PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLogProc = 0;
PFNGLGETPOINTERVPROC glGetPointervProc = 0;
PFNGLPUSHDEBUGGROUPPROC glPushDebugGroupProc = 0;
PFNGLPOPDEBUGGROUPPROC glPopDebugGroupProc = 0;
PFNGLOBJECTLABELPROC glObjectLabelProc = 0;
PFNGLGETOBJECTLABELPROC glGetObjectLabelProc = 0;
PFNGLOBJECTPTRLABELPROC glObjectPtrLabelProc = 0;
PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabelProc = 0;


#ifdef HAVE_GLES3

void* GfxbGetProcAddress(const char* proc_name) {
	void* procAddress = 0;
#if GFXB_EMULATOR
	HMODULE handle = GetModuleHandle( "libGLESv2.dll" );
	procAddress = GetProcAddress( handle, proc_name );
#else
#ifndef IPHONE
	procAddress = (void*)eglGetProcAddress( proc_name );
#if !defined(WIN32) && !defined(IPHONE)
	if (procAddress==0)
	{
		void *handle = dlopen( "libGLESv2.so", RTLD_NOW );
		if (handle != NULL)
		{
			procAddress = dlsym( handle, proc_name );
			dlclose(handle);
		}
	}
#endif
#endif
#endif

	return procAddress;
}


void getES31ProcAddresses()
{
		glDispatchComputeProc = (PFNGLDISPATCHCOMPUTEPROC)GfxbGetProcAddress("glDispatchCompute" );
		glDispatchComputeIndirectProc = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)GfxbGetProcAddress("glDispatchComputeIndirect");
		glBindImageTextureProc = (PFNGLBINDIMAGETEXTUREPROC)GfxbGetProcAddress("glBindImageTexture" );
		glMemoryBarrierProc = (PFNGLMEMORYBARRIERPROC)GfxbGetProcAddress("glMemoryBarrier" );
		glDrawArraysIndirectProc = (PFNGLDRAWARRAYSINDIRECTPROC)GfxbGetProcAddress("glDrawArraysIndirect" );
		glDrawElementsIndirectProc = (PFNGLDRAWELEMENTSINDIRECTPROC)GfxbGetProcAddress("glDrawElementsIndirect");
		glGetProgramResourceivProc = (PFNGLGETPROGRAMRESOURCEIVPROC)GfxbGetProcAddress("glGetProgramResourceiv");
		glGetProgramInterfaceivProc = (PFNGLGETPROGRAMINTERFACEIVPROC)GfxbGetProcAddress("glGetProgramInterfaceiv");
		glGetProgramResourceNameProc = (PFNGLGETPROGRAMRESOURCENAMEPROC)GfxbGetProcAddress("glGetProgramResourceName");

		// GL_EXT_draw_buffers_indexed
		glBlendFunciProc = (PFNGLBLENDFUNCIPROC)GfxbGetProcAddress("glBlendFunciEXT") ;

		// EXT_tessellation_shader
		glPatchParameteriProc = (PFNGLPATCHPARAMETERIPROC)GfxbGetProcAddress("glPatchParameteriEXT");

		// ARB_timer_query
		glGetQueryObjectui64vProc = (PFNGLGETQUERYOBJECTUI64VPROC) GfxbGetProcAddress("glGetQueryObjectui64vEXT");

		// KHR_debug
		glDebugMessageControlProc = (PFNGLDEBUGMESSAGECONTROLPROC)GfxbGetProcAddress("glDebugMessageControl");
		glDebugMessageInsertProc = (PFNGLDEBUGMESSAGEINSERTPROC)GfxbGetProcAddress("glDebugMessageInsert");
		glDebugMessageCallbackProc = (PFNGLDEBUGMESSAGECALLBACKPROC)GfxbGetProcAddress("glDebugMessageCallback");
		glGetDebugMessageLogProc = (PFNGLGETDEBUGMESSAGELOGPROC)GfxbGetProcAddress("glGetDebugMessageLog");
		glGetPointervProc = (PFNGLGETPOINTERVPROC)GfxbGetProcAddress("glGetPointerv");
		glPushDebugGroupProc = (PFNGLPUSHDEBUGGROUPPROC)GfxbGetProcAddress("glPushDebugGroup");
		glPopDebugGroupProc = (PFNGLPOPDEBUGGROUPPROC)GfxbGetProcAddress("glPopDebugGroup");
		glObjectLabelProc = (PFNGLOBJECTLABELPROC)GfxbGetProcAddress("glObjectLabel");
		glGetObjectLabelProc = (PFNGLGETOBJECTLABELPROC)GfxbGetProcAddress("glGetObjectLabel");
		glObjectPtrLabelProc = (PFNGLOBJECTPTRLABELPROC)GfxbGetProcAddress("glObjectPtrLabel");
		glGetObjectPtrLabelProc = (PFNGLGETOBJECTPTRLABELPROC)GfxbGetProcAddress("glGetObjectPtrLabel");

#if 0
		if (!glDispatchComputeProc) glDispatchComputeProc = glDispatchCompute;
		if (!glBindImageTextureProc) glBindImageTextureProc = glBindImageTexture;
		if (!glMemoryBarrierProc) glMemoryBarrierProc = glMemoryBarrier;
		if (!glDrawArraysIndirectProc) glDrawArraysIndirectProc = glDrawArraysIndirect;
		if (!glGetProgramResourceivProc) glGetProgramResourceivProc = glGetProgramResourceiv;
		if (!glGetProgramInterfaceivProc) glGetProgramInterfaceivProc = glGetProgramInterfaceiv;
		if (!glGetProgramResourceNameProc) glGetProgramResourceNameProc = glGetProgramResourceName;

		// GL_EXT_draw_buffers_indexed
		if (!glBlendFunciProc) glBlendFunciProc = glBlendFunciEXT;
		// EXT_tessellation_shader
		if (!glPatchParameteriProc) glPatchParameteriProc = glPatchParameteriEXT;
#endif
}

#endif

#ifdef HAVE_GLEW
void getES31ProcAddresses()
{
	glDispatchComputeProc = glDispatchCompute;
	glDispatchComputeIndirectProc = glDispatchComputeIndirect;
	glBindImageTextureProc = glBindImageTexture;
	glMemoryBarrierProc = glMemoryBarrier;
	glDrawArraysIndirectProc = glDrawArraysIndirect;
	glDrawElementsIndirectProc = glDrawElementsIndirect;
	glGetProgramResourceivProc = glGetProgramResourceiv;
	glGetProgramInterfaceivProc = glGetProgramInterfaceiv;
	glGetProgramResourceNameProc = glGetProgramResourceName;

	// GL_EXT_draw_buffers_indexed
	glBlendFunciProc = glBlendFunci;

	// EXT_tessellation_shader
	glPatchParameteriProc = glPatchParameteri;

	glGetQueryObjectui64vProc = glGetQueryObjectui64v;

	// KHR_debug
	glDebugMessageControlProc = glDebugMessageControl;
	glDebugMessageInsertProc = glDebugMessageInsert;
	glDebugMessageCallbackProc = glDebugMessageCallback;
	glGetDebugMessageLogProc = glGetDebugMessageLog;
	glGetPointervProc = glGetPointerv;
	glPushDebugGroupProc = glPushDebugGroup;
	glPopDebugGroupProc = glPopDebugGroup;
	glObjectLabelProc = glObjectLabel;
	glGetObjectLabelProc = glGetObjectLabel;
	glObjectPtrLabelProc = glObjectPtrLabel;
	glGetObjectPtrLabelProc = glGetObjectPtrLabel;
}
#endif
