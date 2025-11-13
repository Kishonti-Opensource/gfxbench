/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"
#include "kcl_io.h"
#include "../gfxbench/global_test_environment.h"

#if defined USE_ANY_GL
#include "opengl/fbo.h"
#elif defined HAVE_DX
#include "d3d11/fbo3.h"
#elif defined USE_METAL
#include "metal/fbo.h"
#endif

#ifdef USE_ANY_GL
#include "opengl/ext.h"
#if defined OPT_TEST_GFX30 || defined OPT_TEST_GFX31
#include "opengl/gui_test.h"
#endif
#include "opengl/glb_opengl_state_manager.h"
#elif (defined WIN32) && (defined HAVE_DX)
#include "d3d11/dxb_base.h"
#endif

#ifdef USE_METAL
#include "mtl_types.h"
#endif

namespace GLB
{

//testfw functions
bool CreateGlobals(const char* data, const char* datarw,const GlobalTestEnvironment * const gte)
{
   	KCL::g_os = KCL::OS::CreateI("");

	KCL::AssetFile::ClearScenePath();
	KCL::AssetFile::SetDataPath(data);
	KCL::AssetFile::SetRWDataPath(datarw);
#ifdef USE_ANY_GL
    if (gte->IsGraphicsContextGLorES())
    {
        g_extension = new Extension(gte->GetGraphicsContext());
        g_extension->init ();
    }
#endif

#if defined HAVE_DX
	DXB::Initialize(); //in HAVE_DX, this is called from GFXBenchDxInterop::Initialize()
#elif defined USE_METAL
    if (gte->IsGraphicsContextMetal())
    {
        MetalRender::Initialize(gte) ;
        FBO::CreateGlobalFBO(gte) ;
    }
#elif defined USE_ANY_GL
    if (gte->IsGraphicsContextGLorES())
    {
		KCL::Initialize(false);
        FBO::CreateGlobalFBO();
        OpenGLStateManager::Reset();
    }
#endif
	return true;
}

void DestroyGlobals(const GlobalTestEnvironment * const gte)
{
#if defined HAVE_DX
	DX::Release();
#elif defined USE_METAL
    if (gte->IsGraphicsContextMetal())
    {
        FBO::DeleteGlobalFBO() ;
        MetalRender::Release() ;
    }
#else
    if (gte->IsGraphicsContextGLorES())
    {
        FBO::DeleteGlobalFBO();
        KCL::Release();
        OpenGLStateManager::Reset();
        delete g_extension;
        g_extension = 0;
    }
#endif
	KCL::OS::DestroyI(&KCL::g_os);
	KCL::g_os = 0;
}

//testfw functions end

}//namespace GLB
