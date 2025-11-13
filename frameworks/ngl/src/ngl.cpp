/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"

void empty_logf(const char *format, ...)
{
}

PFN_BeginCommandBuffer nglBeginCommandBuffer = 0;
PFN_EndCommandBuffer nglEndCommandBuffer = 0;
PFN_SubmitCommandBuffer nglSubmitCommandBuffer = 0;
PFN_GenTexture nglGenTexture = 0;
PFN_GenVertexBuffer nglGenVertexBuffer = 0;
PFN_GenIndexBuffer nglGenIndexBuffer = 0;
PFN_GenJob nglGenJob = 0;
PFN_Begin nglBegin = 0;
PFN_NextSubpass nglNextSubpass = 0;
PFN_End nglEnd = 0;
PFN_Draw nglDraw = 0;
PFN_DrawInstanced nglDrawInstanced = 0;
PFN_DrawIndirect nglDrawIndirect = 0;
PFN_Dispatch nglDispatch = 0;
PFN_DispatchIndirect nglDispatchIndirect = 0;
PFN_ViewportScissor nglViewportScissor = 0;
PFN_GetString nglGetString = 0;
PFN_GetInteger nglGetInteger = 0;
PFN_DeletePipelines nglDeletePipelines = 0;
PFN_CustomAction nglCustomAction = 0;
PFN_LineWidth nglLineWidth = 0;
PFN_GetTextureContent nglGetTextureContent = 0;
PFN_GetVertexBufferContent nglGetVertexBufferContent = 0;
PFN_ResizeTextures nglResizeTextures = 0;
PFN_BlendState nglBlendState = 0;
PFN_DepthState nglDepthState = 0;
PFN_Flush nglFlush = 0;
PFN_Finish nglFinish = 0;
PFNLOGF _logf = empty_logf;
PFN_BeginStatistic nglBeginStatistic = 0;
PFN_EndStatistic nglEndStatistic = 0;
PFN_GetStatistic nglGetStatistic = 0;
PFN_DestroyContext nglDestroyContext = 0;
PFN_Barrier nglBarrier = 0;


void nglCreateContextD3D11(NGL_context_descriptor &descriptor);
void nglCreateContextD3D12(NGL_context_descriptor &descriptor);
void nglCreateContextMetal(NGL_context_descriptor &descriptor);
void nglCreateContextOGL(NGL_context_descriptor &descriptor);
void nglCreateContextVulkan(NGL_context_descriptor &descriptor);
void nglCreateContextNull(NGL_context_descriptor &descriptor);


bool nglCreateContext(NGL_context_descriptor& descriptor)
{
	if (descriptor.__logf)
	{
		_logf = descriptor.__logf;
	}

	switch (descriptor.m_api)
	{

#ifdef ENABLE_D3D11_IMPLEMENTATION
	case NGL_DIRECT3D_11:
	{
		nglCreateContextD3D11(descriptor);
		return true;
	}
#endif

#ifdef ENABLE_D3D12_IMPLEMENTATION
	case NGL_DIRECT3D_12:
	{
		nglCreateContextD3D12(descriptor);
		return true;
	}
#endif

#ifdef ENABLE_METAL_IMPLEMENTATION
    case NGL_METAL_IOS:
    case NGL_METAL_MACOS:
    {
		nglCreateContextMetal(descriptor);
        return true;
    }
#endif

#ifdef ENABLE_GL_IMPLEMENTATION
	case NGL_OPENGL:
	{
		nglCreateContextOGL(descriptor);
		return true;
	}
#endif

#if defined(ENABLE_ES31_IMPLEMENTATION) || defined(ENABLE_GL_IMPLEMENTATION)
	case NGL_OPENGL_ES:
	{
		nglCreateContextOGL(descriptor);
		return true;
	}
#endif

#ifdef ENABLE_VULKAN_IMPLEMENTATION
	case NGL_VULKAN:
	{
		nglCreateContextVulkan(descriptor);
		return true;
	}
#endif

#ifdef ENABLE_NULL_IMPLEMENTATION
	case NGL_NULL_DRIVER:
	{
		nglCreateContextNull(descriptor);
		return true;
	}
#endif

    default:
        return false;
	}

	return false;
}
